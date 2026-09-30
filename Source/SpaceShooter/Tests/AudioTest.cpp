#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "../SpaceGameMode.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Editor.h"
#include "Engine/World.h"

class FCheckOrbitAudio : public IAutomationLatentCommand
{
public:
    explicit FCheckOrbitAudio(FAutomationTestBase* T): Test(T) {}
    bool Update() override
    {
        UWorld* W=GEditor?GEditor->PlayWorld:nullptr;
        auto* M=W?W->GetAuthGameMode<ASpaceGameMode>():nullptr;
        if(!M) { Test->AddError(TEXT("Missing audio GameMode")); return true; }
        const double Now=W->GetTimeSeconds();
        if(Now<Next) return false;
        auto Check=[this,M](bool Game)
        {
            Test->TestEqual(TEXT("Menu player follows state after fade"),M->MenuPlayer->IsPlaying(),!Game);
            Test->TestEqual(TEXT("Game player follows state after fade"),M->GamePlayer->IsPlaying(),Game);
        };
        switch(Step++)
        {
        case 0:
            M->bPersistProgress=false; M->bSpawningEnabled=false; M->bBonusesEnabled=false;
            for(auto* S:{M->MenuMusic.Get(),M->GameMusic.Get()})
            {
                auto* Wave=Cast<USoundWave>(S);
                Test->TestNotNull(TEXT("Music is an imported sound wave"),Wave);
                if(Wave) { Test->TestTrue(TEXT("Music loops"),Wave->bLooping); Test->TestTrue(TEXT("Music is a complete phrase"),Wave->Duration>40.f); }
            }
            Test->TestEqual(TEXT("Four interface cues configured"),M->InterfaceSounds.Num(),4);
            for(auto S:M->InterfaceSounds)
            {
                auto* Wave=Cast<USoundWave>(S);
                Test->TestNotNull(TEXT("UI cue imported"),Wave);
                if(Wave) Test->TestFalse(TEXT("Button sound cannot loop"),Wave->bLooping);
            }
            Check(false); M->StartRun(); Next=Now+1.4; break;
        case 1:
            Check(true); M->StartRun(); Next=Now+1.4; break;
        case 2:
            Check(true);
            // Reverse an in-progress fade twice: the outgoing player must stop.
            M->ReturnToMenu(); M->StartRun(); M->ReturnToMenu(); Next=Now+1.4; break;
        case 3:
            Check(false); M->StartRun(); M->Lives=1; Next=Now+1.8; break;
        case 4:
            Check(true); Test->TestTrue(TEXT("Last life lost"),M->LoseLife()); Next=Now+1.4; break;
        case 5:
            Check(false); Test->TestTrue(TEXT("Game over restores menu music"),M->State==ESpaceRunState::GameOver);
            M->StartRun(); Next=Now+1.4; break;
        default:
            Check(true); return true;
        }
        return false;
    }
private:
    FAutomationTestBase* Test;
    int32 Step=0;
    double Next=0;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOrbitAudioTest,"SpaceShooter.Audio.Transitions",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FOrbitAudioTest::RunTest(const FString&)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Maps/L_Arena")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.5f));
    ADD_LATENT_AUTOMATION_COMMAND(FCheckOrbitAudio(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
