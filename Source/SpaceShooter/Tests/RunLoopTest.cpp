#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "../SpaceGameMode.h"
#include "../SpaceAsteroid.h"
#include "../ShipPawn.h"
#include "Components/SphereComponent.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

class FCheckRunLoop : public IAutomationLatentCommand
{
public:
    explicit FCheckRunLoop(FAutomationTestBase* T): Test(T) {}
    bool Update() override
    {
        UWorld* W=GEditor?GEditor->PlayWorld:nullptr;
        auto* Mode=W?W->GetAuthGameMode<ASpaceGameMode>():nullptr;
        auto* Ship=W?Cast<AShipPawn>(W->GetFirstPlayerController()->GetPawn()):nullptr;
        if (!Mode || !Ship) { Test->AddError(TEXT("Missing run actors")); return true; }
        const double Now=W->GetTimeSeconds();
        if (Now<Next) return false;
        auto Clear=[W]() { for(TActorIterator<ASpaceAsteroid> It(W);It;++It) It->Destroy(); };
        auto Contact=[W,Ship]() {
            auto* A=W->SpawnActor<ASpaceAsteroid>(Ship->GetActorLocation()+FVector(-150,0,0),FRotator::ZeroRotator);
            A->Launch(FVector(900,0,0));
        };
        switch(Step++)
        {
        case 0:
            Mode->bPersistProgress=false;
            Test->TestTrue(TEXT("The initial screen is the menu"),Mode->State==ESpaceRunState::Menu);
            Mode->BestScore=15000;
            Mode->bBonusesEnabled=false;
            for(int32 Index=0;Index<3;++Index)
            {
                Mode->SelectShip(Index);
                Test->TestEqual(TEXT("Menu selects each ship"),Mode->SelectedShip,Index);
                Test->TestEqual(TEXT("Selection applies the matching material"),Ship->ShipStyle,Index);
            }
            Mode->SelectShip(8);
            Test->TestEqual(TEXT("Invalid selection is ignored"),Mode->SelectedShip,2);
            Mode->StartRun();
            Mode->SelectShip(0);
            Test->TestEqual(TEXT("Fleet cannot switch during combat"),Mode->SelectedShip,2);
            Test->TestEqual(TEXT("Selected ship persists into combat"),Ship->ShipStyle,2);
            Mode->MinimumSpawnDelay=.12f; Mode->MaximumSpawnDelay=.2f;
            Test->TestEqual(TEXT("Run starts with three lives"),Mode->Lives,3);
            Test->TestFalse(TEXT("Recovery shield prevents immediate life loss"),Mode->LoseLife());
            Next=Now+2.f; break;
        case 1:
        {
            int32 Count=0; for(TActorIterator<ASpaceAsteroid> It(W);It;++It) ++Count;
            Test->TestTrue(TEXT("The running timer generates asteroids"),Count>=2);
            Mode->bSpawningEnabled=false; Clear();
            for(int32 i=0;i<12;++i)
            {
                auto* A=Mode->SpawnAsteroid();
                Test->TestNotNull(TEXT("Random border spawn succeeds"),A);
                if (!A) continue;
                const FVector P=A->GetActorLocation();
                Test->TestTrue(TEXT("Asteroid starts on a configured border"),FMath::IsNearlyEqual(FMath::Abs(P.X),double(Mode->SpawnHalfSize.X),.1) || FMath::IsNearlyEqual(FMath::Abs(P.Y),double(Mode->SpawnHalfSize.Y),.1));
                Test->TestTrue(TEXT("Spawn health lies in configured range"),A->RemainingHits==static_cast<int32>(A->SizeClass)+1);
                A->Destroy();
            }
            Rock=W->SpawnActor<ASpaceAsteroid>(FVector(0,600,0),FRotator::ZeroRotator);
            Rock->Launch(FVector(200,0,0));
            Next=Now+.3f; break;
        }
        case 2:
            Test->TestTrue(TEXT("Initial impulse produces continuing planar movement"),Rock.IsValid() && Rock->GetActorLocation().X>35 && FMath::Abs(Rock->GetActorLocation().Z)<.1f);
            if(Rock.IsValid())
            {
                const int32 Hits=Rock->RemainingHits;
                const int32 ExpectedScore=Rock->ScoreValue;
                for(int32 i=0;i<Hits;++i) Rock->ReceiveShot();
                Test->TestEqual(TEXT("Only destruction awards one score increment"),Mode->Score,ExpectedScore);
            }
            Contact(); Next=Now+.3f; break;
        case 3:
            Test->TestEqual(TEXT("A physical asteroid overlap removes exactly one life"),Mode->Lives,2);
            Contact(); Next=Now+.3f; break;
        case 4:
            Test->TestEqual(TEXT("Second collision during recovery cannot remove another life"),Mode->Lives,2);
            Next=Now+1.5f; break;
        case 5: Contact(); Next=Now+.3f; break;
        case 6:
            Test->TestEqual(TEXT("Damage resumes after recovery"),Mode->Lives,1);
            Next=Now+1.5f; break;
        case 7: Contact(); Next=Now+.3f; break;
        case 8:
            Test->TestTrue(TEXT("Last life ends the run"),Mode->Lives==0 && Mode->State==ESpaceRunState::GameOver);
            {
                const int32 FinalScore=Mode->Score; Mode->AwardAsteroid();
                Test->TestEqual(TEXT("Score cannot change after game over"),Mode->Score,FinalScore);
            }
            Test->TestNull(TEXT("No spawn after game over"),Mode->SpawnAsteroid());
            Mode->StartRun();
            Test->TestTrue(TEXT("Restart restores lives, score and visible ship"),Mode->Lives==3 && Mode->Score==0 && !Ship->IsHidden());
            Test->TestEqual(TEXT("Restart preserves ship choice"),Ship->ShipStyle,2);
            Mode->ReturnToMenu();
            Test->TestTrue(TEXT("Return to menu stops the run"),Mode->State==ESpaceRunState::Menu);
            return true;
        }
        return false;
    }
private:
    FAutomationTestBase* Test;
    int32 Step=0; double Next=0;
    TWeakObjectPtr<ASpaceAsteroid> Rock;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunLoopTest,"SpaceShooter.Gameplay.RunLoop",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRunLoopTest::RunTest(const FString&)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Maps/L_Arena")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(.5f));
    ADD_LATENT_AUTOMATION_COMMAND(FCheckRunLoop(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
