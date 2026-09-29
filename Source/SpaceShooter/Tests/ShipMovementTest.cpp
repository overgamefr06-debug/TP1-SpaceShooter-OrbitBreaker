#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "../ShipPawn.h"
#include "../SpaceGameMode.h"
#include "Editor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "InputKeyEventArgs.h"

class FCheckShipControls : public IAutomationLatentCommand
{
public:
    explicit FCheckShipControls(FAutomationTestBase* InTest) : Test(InTest) {}
    virtual bool Update() override
    {
        UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
        APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
        AShipPawn* Ship = PC ? Cast<AShipPawn>(PC->GetPawn()) : nullptr;
        if (!Ship)
        {
            Test->AddError(TEXT("No controlled ship in the arena"));
            return true;
        }
        if (Index >= UE_ARRAY_COUNT(Cases)) return true;
        if (!bStarted)
        {
            auto* Mode=World->GetAuthGameMode<ASpaceGameMode>();
            Mode->StartRun(); Mode->bSpawningEnabled=false;
            bStarted=true;
        }
        const FCase& Case = Cases[Index];
        if (!bPressed)
        {
            Ship->SetActorLocation(FVector::ZeroVector);
            Ship->Movement->StopMovementImmediately();
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(Case.Key, IE_Pressed, 1.f));
            StartTime = World->GetTimeSeconds();
            bPressed = true;
            return false;
        }
        if (World->GetTimeSeconds() - StartTime < .35f) return false;
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(Case.Key, IE_Released, 0.f));
        const FVector Position = Ship->GetActorLocation();
        Test->TestTrue(FString::Printf(TEXT("%s moves in the expected direction"), *Case.Key.ToString()),
            FVector::DotProduct(Position, Case.Direction) > 20.f);
        Test->TestTrue(TEXT("Ship remains in its movement plane"), FMath::Abs(Position.Z) < .1f);
        if (Index == UE_ARRAY_COUNT(Cases) - 1)
        {
            Ship->SetActorLocation(FVector(5000, -5000, 50));
            Ship->Tick(0.f);
            const FVector Bounded = Ship->GetActorLocation();
            Test->TestTrue(TEXT("Ship stays inside the arena"),
                FMath::Abs(Bounded.X) <= Ship->ArenaHalfSize.X &&
                FMath::Abs(Bounded.Y) <= Ship->ArenaHalfSize.Y && FMath::IsNearlyZero(Bounded.Z));
        }
        ++Index;
        bPressed = false;
        return false;
    }
private:
    struct FCase { FKey Key; FVector Direction; };
    const FCase Cases[10] = {
        {EKeys::Up, FVector(1,0,0)}, {EKeys::Down, FVector(-1,0,0)},
        {EKeys::Left, FVector(0,-1,0)}, {EKeys::Right, FVector(0,1,0)},
        {EKeys::W, FVector(1,0,0)}, {EKeys::Z, FVector(1,0,0)},
        {EKeys::A, FVector(0,-1,0)}, {EKeys::Q, FVector(0,-1,0)},
        {EKeys::S, FVector(-1,0,0)}, {EKeys::D, FVector(0,1,0)}
    };
    FAutomationTestBase* Test;
    int32 Index = 0;
    float StartTime = 0.f;
    bool bPressed = false;
    bool bStarted = false;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShipMovementTest, "SpaceShooter.Gameplay.ShipControls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShipMovementTest::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Maps/L_Arena")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
    ADD_LATENT_AUTOMATION_COMMAND(FCheckShipControls(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
