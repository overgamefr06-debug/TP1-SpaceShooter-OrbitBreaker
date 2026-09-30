#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "../SpaceGameMode.h"
#include "../ShipPawn.h"
#include "../SpaceAsteroid.h"
#include "../CombatBurst.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "UnrealClient.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

// Pixel regression: firing must not erase the opaque pixels of any ship.
// Uses the real game viewport, Blueprint materials and muzzle effect.
class FCheckOrbitEffects : public IAutomationLatentCommand
{
public:
    explicit FCheckOrbitEffects(FAutomationTestBase* T):Test(T) {}
    bool Update() override
    {
        UWorld* W=GEditor?GEditor->PlayWorld:nullptr;
        auto* Mode=W?W->GetAuthGameMode<ASpaceGameMode>():nullptr;
        auto* PC=W?W->GetFirstPlayerController():nullptr;
        auto* Ship=PC?Cast<AShipPawn>(PC->GetPawn()):nullptr;
        if(!Ship || !Mode) { Test->AddError(TEXT("Missing visual test world")); return true; }
        const double Now=W->GetTimeSeconds();
        if(Step==0)
        {
            Mode->bPersistProgress=false; Mode->bSpawningEnabled=false; Mode->bBonusesEnabled=false;
            Mode->BestScore=15000; Mode->StartRun(); Ship->SetActorLocation(FVector::ZeroVector);
            Folder=FPaths::ProjectSavedDir()/TEXT("EffectsValidation");
            IFileManager::Get().MakeDirectory(*Folder,true);
            Next=Now+2.2; Step=1;
        }
        if(Now<Next) return false;
        auto* V=W->GetGameViewport()->Viewport;
        TArray<FColor> Pixels;
        if(!V || !V->ReadPixels(Pixels)) { Test->AddError(TEXT("No rendered pixels")); return true; }
        const auto Size=V->GetSizeXY();
        for(auto& P:Pixels) P.A=255;
        auto Save=[&](const FString& Name) { TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG); FFileHelper::SaveArrayToFile(PNG,*(Folder/Name)); };
        if(Step==1)
        {
            Baseline=Pixels; Save(FString::Printf(TEXT("Ship%d_before.png"),Style));
            Sample=0; Step=2;
        }
        else if(Step==2)
        {
            FVector2D A,B;
            PC->ProjectWorldLocationToScreen(FVector(48,-48,0),A);
            PC->ProjectWorldLocationToScreen(FVector(-48,48,0),B);
            int32 Bright=0,Lost=0;
            for(int32 Y=FMath::Max(0,int32(A.Y));Y<FMath::Min(Size.Y,int32(B.Y));++Y)
                for(int32 X=FMath::Max(0,int32(A.X));X<FMath::Min(Size.X,int32(B.X));++X)
                {
                    const int32 I=Y*Size.X+X;
                    if(Baseline.IsValidIndex(I) && Baseline[I].R+Baseline[I].G+Baseline[I].B>150)
                    { ++Bright; if(Pixels[I].R+Pixels[I].G+Pixels[I].B<40) ++Lost; }
                }
            const float Ratio=Bright?float(Lost)/Bright:0.f;
            if(Sample==0)
            {
                Test->TestTrue(TEXT("Baseline contains a visible ship"),Bright>100);
            }
            Worst=FMath::Max(Worst,Ratio);
            Save(FString::Printf(TEXT("Ship%d_fire_%02d.png"),Style,Sample));
            if(++Sample>=18)
            {
                Test->AddInfo(FString::Printf(TEXT("Ship %d maximum erased bright pixels: %.1f%%"),Style,Worst*100));
                Test->TestTrue(TEXT("Firing keeps at least 95 percent of opaque ship pixels visible"),Worst<.05f);
                Worst=0;
                if(++Style<3) { Ship->ApplyShipStyle(Style); Step=1; Next=Now+1.; return false; }
                Mode->ActivateBonus(ESpaceBonus::Shield);
                for(int32 I=0;I<Mode->PickupClasses.Num();++I)
                    W->SpawnActor<ASpacePickup>(Mode->PickupClasses[I],FVector(180,-300+I*200,0),FRotator::ZeroRotator);
                Step=3; Next=Now+.5; return false;
            }
        }
        else
        {
            auto Describe=[this](UStaticMeshComponent* M) {
                Test->AddInfo(FString::Printf(TEXT("FX %s mesh=%s material=%s visible=%d position=%s scale=%s"),*M->GetName(),*GetNameSafe(M->GetStaticMesh()),*GetNameSafe(M->GetMaterial(0)),M->IsVisible(),*M->GetComponentLocation().ToString(),*M->GetComponentScale().ToString()));
            };
            Describe(Ship->ShieldGlow);
            int32 Count=0;
            for(TActorIterator<ASpacePickup> It(W);It;++It) { Describe(It->Visual); ++Count; }
            Test->TestEqual(TEXT("Four showcase pickups exist"),Count,4);
            Save(TEXT("Shield_and_pickups.png"));
            return true;
        }
        Ship->TryFire(); Next=Now+.025;
        return false;
    }
private:
    FAutomationTestBase* Test;
    int32 Step=0,Style=0,Sample=0;
    double Next=0;
    float Worst=0;
    FString Folder;
    TArray<FColor> Baseline;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEffectsVisualTest,"SpaceShooter.Visual.Effects",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FEffectsVisualTest::RunTest(const FString&)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Maps/L_Arena")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(.5f));
    ADD_LATENT_AUTOMATION_COMMAND(FCheckOrbitEffects(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
class FCheckRockEffects : public IAutomationLatentCommand
{
public:
    explicit FCheckRockEffects(FAutomationTestBase* T):Test(T) {}
    bool Update() override
    {
        UWorld* W=GEditor?GEditor->PlayWorld:nullptr;
        auto* Mode=W?W->GetAuthGameMode<ASpaceGameMode>():nullptr;
        auto* PC=W?W->GetFirstPlayerController():nullptr;
        if(!Mode || !PC) { Test->AddError(TEXT("Missing asteroid visual world")); return true; }
        const double Now=W->GetTimeSeconds();
        if(Step==0)
        {
            Mode->bPersistProgress=false; Mode->bSpawningEnabled=false; Mode->bBonusesEnabled=false;
            Mode->StartRun(); PC->GetPawn()->SetActorLocation(FVector(-450,-850,0));
            auto* Rock=W->SpawnActorDeferred<ASpaceAsteroid>(Mode->AsteroidClass,FTransform(FVector::ZeroVector));
            Rock->bRandomSize=false; Rock->SizeClass=EAsteroidSize::Large;
            Rock->FinishSpawning(FTransform(FVector::ZeroVector)); Rock->SetActorTickEnabled(false);
            Folder=FPaths::ProjectSavedDir()/TEXT("RockEffectsValidation"); IFileManager::Get().MakeDirectory(*Folder,true);
            Step=1; Next=Now+2.;
        }
        if(Now<Next) return false;
        auto* V=W->GetGameViewport()->Viewport;
        TArray<FColor> Pixels;
        if(!V || !V->ReadPixels(Pixels)) { Test->AddError(TEXT("Missing rock pixels")); return true; }
        const auto Size=V->GetSizeXY(); for(auto& P:Pixels) P.A=255;
        auto Save=[&](FString Name) { TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG); FFileHelper::SaveArrayToFile(PNG,*(Folder/Name)); };
        if(Step==1)
        {
            Baseline=Pixels; Save(FString::Printf(TEXT("Case%d_before.png"),Case));
            if(Case==0)
            {
                // Real laser destruction beside a surviving rock.
                const FVector Origin(75,0,0);
                auto* Target=W->SpawnActorDeferred<ASpaceAsteroid>(Mode->AsteroidClass,FTransform(Origin));
                Target->bRandomSize=false; Target->SizeClass=EAsteroidSize::Small;
                Target->FinishSpawning(FTransform(Origin)); Target->ReceiveShot();
            }
            else
            {
                auto* Ship=Cast<AShipPawn>(PC->GetPawn());
                W->SpawnActor<ACombatBurst>(Ship->DamageEffectClass,FVector(75,0,0),FRotator::ZeroRotator);
            }
            Step=2; Sample=0; Next=Now+.025; return false;
        }
        FVector2D A,B;
        PC->ProjectWorldLocationToScreen(FVector(60,-60,0),A); PC->ProjectWorldLocationToScreen(FVector(-60,60,0),B);
        int32 Bright=0,Lost=0;
        for(int32 Y=FMath::Max(0,int32(A.Y));Y<FMath::Min(Size.Y,int32(B.Y));++Y)
            for(int32 X=FMath::Max(0,int32(A.X));X<FMath::Min(Size.X,int32(B.X));++X)
            { const int32 I=Y*Size.X+X; if(Baseline[I].R+Baseline[I].G+Baseline[I].B>170) { ++Bright; if(Pixels[I].R+Pixels[I].G+Pixels[I].B<40) ++Lost; } }
        Worst=FMath::Max(Worst,Bright?float(Lost)/Bright:1.f);
        Save(FString::Printf(TEXT("Case%d_burst_%02d.png"),Case,Sample));
        if(++Sample>=20)
        {
            Test->TestTrue(TEXT("Reference asteroid visible"),Bright>100);
            Test->AddInfo(FString::Printf(TEXT("Asteroid effect %d erased %.1f percent of surviving rock"),Case,Worst*100));
            Test->TestTrue(TEXT("Asteroid blast must preserve 99 percent of surviving rock pixels"),Worst<.01f);
            if(++Case>=2) return true;
            Worst=0; Step=1; Next=Now+1.; return false;
        }
        Next=Now+.025; return false;
    }
private:
    FAutomationTestBase* Test;
    int32 Step=0,Sample=0,Case=0;
    double Next=0;
    float Worst=0;
    FString Folder;
    TArray<FColor> Baseline;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRockEffectsVisualTest,"SpaceShooter.Visual.AsteroidEffects",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRockEffectsVisualTest::RunTest(const FString&)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Maps/L_Arena")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(.5f));
    ADD_LATENT_AUTOMATION_COMMAND(FCheckRockEffects(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
