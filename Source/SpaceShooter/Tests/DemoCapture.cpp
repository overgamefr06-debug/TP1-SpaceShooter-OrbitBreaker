#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "../SpaceGameMode.h"
#include "../SpaceAsteroid.h"
#include "../ShipPawn.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "UnrealClient.h"
#include "ImageUtils.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"

// Opt-in recording utility, excluded from the packaged game. It records the
// actual viewport while simulating inputs. Four pickups and one colliding pair
// are staged to demonstrate new features; the remaining spawns are normal.
class FRecordOrbitDemo : public IAutomationLatentCommand
{
public:
    explicit FRecordOrbitDemo(FAutomationTestBase* T) : Test(T) {}
    bool Update() override
    {
        UWorld* W=GEditor ? GEditor->PlayWorld : nullptr;
        APlayerController* PC=W ? W->GetFirstPlayerController() : nullptr;
        auto* Ship=PC ? Cast<AShipPawn>(PC->GetPawn()) : nullptr;
        auto* Mode=W ? W->GetAuthGameMode<ASpaceGameMode>() : nullptr;
        if (!Ship || !Mode) { Test->AddError(TEXT("Demo requires a PIE game world")); return true; }
        const double Now=FPlatformTime::Seconds();
        if (!bStarted)
        {
            Mode->bPersistProgress=false; Mode->BestScore=0;
            bStarted=true; Start=Now; LastFrame=Now;
            Folder=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("ArcadeDemoCapture"));
            IFileManager::Get().MakeDirectory(*Folder,true);
            UAudioMixerBlueprintLibrary::StartRecordingOutput(W,65.f);
        }
        const double T=Now-Start;
        auto Key=[PC](FKey K,bool Down) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(K,Down?IE_Pressed:IE_Released,Down?1.f:0.f)); };
        auto Hold=[&Key](FKey K,bool Wanted,bool& Held) { if(Wanted!=Held) { Key(K,Wanted); Held=Wanted; } };
        if (!bLaunched)
        {
            // Fresh progression: Aegis available, Spectre and Helios visibly locked.
            if(T>=5.) { Key(EKeys::Enter,true); Key(EKeys::Enter,false); bLaunched=true; }
        }
        if (Mode->State==ESpaceRunState::Playing)
        {
            const int32 BonusOrder[]={0,1,3,2};
            if(ShowcaseBonus<4 && T>=12.+ShowcaseBonus*7. && Mode->PickupClasses.Num()==4)
            {
                auto* P=W->SpawnActor<ASpacePickup>(Mode->PickupClasses[BonusOrder[ShowcaseBonus++]],Ship->GetActorLocation()+FVector(0,180,0),FRotator::ZeroRotator);
                if(P) P->SetActorLocation(Ship->GetActorLocation());
            }
            if(!bPairShown && T>18.)
            {
                bPairShown=true;
                for(int32 Side:{-1,1})
                {
                    const FVector P(220,Side*280,0);
                    auto* Rock=W->SpawnActorDeferred<ASpaceAsteroid>(Mode->AsteroidClass,FTransform(P));
                    Rock->bRandomSize=false; Rock->SizeClass=EAsteroidSize::Large;
                    Rock->FinishSpawning(FTransform(P)); Rock->Launch(FVector(0,-Side*120,0));
                }
            }
            // A slow rectangular patrol shows all four directional controls.
            const double Phase=FMath::Fmod(FMath::Max(0.,T-9.),8.);
            const bool Patrol=T<42.;
            Hold(EKeys::Right,Patrol && Phase<1.,Right);
            Hold(EKeys::Up,Patrol && Phase>=2. && Phase<2.55,Up);
            Hold(EKeys::Left,Patrol && Phase>=4. && Phase<5.,Left);
            Hold(EKeys::Down,Patrol && Phase>=6. && Phase<6.55,Down);
            Hold(EKeys::SpaceBar,T<42.,Fire);
        }
        else
        {
            Hold(EKeys::Right,false,Right); Hold(EKeys::Up,false,Up);
            Hold(EKeys::Left,false,Left); Hold(EKeys::Down,false,Down); Hold(EKeys::SpaceBar,false,Fire);
        }
        if (Mode->State==ESpaceRunState::GameOver)
        {
            if (GameOverAt<0.) GameOverAt=T;
            if (!bRestarted && T-GameOverAt>3.) { Key(EKeys::Enter,true); Key(EKeys::Enter,false); bRestarted=true; }
        }
        if (T>=59.)
        {
            UAudioMixerBlueprintLibrary::StopRecordingOutput(W,EAudioRecordingExportType::WavFile,TEXT("DemoAudio"),Folder);
            FFileHelper::SaveStringToFile(Timeline,*(Folder/TEXT("frames.csv")),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
            Test->TestTrue(TEXT("Recorded a useful sequence of actual game frames"),Frame>300);
            Test->AddInfo(FString::Printf(TEXT("Demo captured %d frames; restart observed: %s"),Frame,bRestarted?TEXT("yes"):TEXT("no")));
            return true;
        }
        if (Now-LastFrame>=1./15.)
        {
            // Read the PIE viewport explicitly. A global screenshot request can
            // be consumed by an editor viewport instead when focus changes.
            auto* Client=W->GetGameViewport();
            auto* Viewport=Client?Client->Viewport:nullptr;
            TArray<FColor> Pixels;
            if(Viewport && Viewport->ReadPixels(Pixels))
            {
                const FIntPoint Size=Viewport->GetSizeXY();
                for(auto& Pixel:Pixels) Pixel.A=255;
                TArray64<uint8> PNG;
                FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
                const FString Name=FString::Printf(TEXT("Frame_%05d.png"),Frame++);
                FFileHelper::SaveArrayToFile(PNG,*(Folder/Name));
                Timeline+=FString::Printf(TEXT("%s,%.6f,%d,%d,%d\n"),*Name,T,int32(Mode->State),Mode->Score,Mode->Lives);
            }
            LastFrame=Now;
        }
        return false;
    }
private:
    FAutomationTestBase* Test;
    bool bStarted=false,bLaunched=false,bRestarted=false,bPairShown=false;
    int32 ShowcaseBonus=0;
    bool Right=false,Up=false,Left=false,Down=false,Fire=false;
    double Start=0,LastFrame=0,GameOverAt=-1;
    int32 Frame=0;
    FString Folder,Timeline;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOrbitDemoCapture,"SpaceShooter.Delivery.RecordDemo",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FOrbitDemoCapture::RunTest(const FString&)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Maps/L_Arena")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
    ADD_LATENT_AUTOMATION_COMMAND(FRecordOrbitDemo(this));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
