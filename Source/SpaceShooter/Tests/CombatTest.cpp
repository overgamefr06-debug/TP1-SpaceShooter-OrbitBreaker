#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "../ShipPawn.h"
#include "../ShotProjectile.h"
#include "../SpaceAsteroid.h"
#include "../CombatBurst.h"
#include "../SpaceGameMode.h"
#include "Editor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"

template<class T> static int32 CountActors(UWorld* World)
{
    int32 Count = 0;
    for (TActorIterator<T> It(World); It; ++It) if (IsValid(*It)) ++Count;
    return Count;
}

class FCheckCombat : public IAutomationLatentCommand
{
public:
    explicit FCheckCombat(FAutomationTestBase* InTest) : Test(InTest) {}
    bool Update() override
    {
        UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
        APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
        AShipPawn* Ship = PC ? Cast<AShipPawn>(PC->GetPawn()) : nullptr;
        if (!Ship) { Test->AddError(TEXT("No ship for the combat test")); return true; }
        const double Now = World->GetTimeSeconds();
        if (Now < NextStep) return false;
        auto Key = [PC](FKey K, bool Down) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(K, Down ? IE_Pressed : IE_Released, Down ? 1.f : 0.f)); };
        auto Target = [World](FVector Position, int32 Minimum, int32 Maximum)
        {
            auto* Rock = World->SpawnActorDeferred<ASpaceAsteroid>(ASpaceAsteroid::StaticClass(), FTransform(Position));
            Rock->bRandomSize = false; Rock->SizeClass = static_cast<EAsteroidSize>(FMath::Clamp(Minimum-1,0,2));
            Rock->FinishSpawning(FTransform(Position));
            return Rock;
        };
        switch (Step++)
        {
        case 0:
            World->GetAuthGameMode<ASpaceGameMode>()->StartRun();
            World->GetAuthGameMode<ASpaceGameMode>()->bSpawningEnabled=false;
            for (TActorIterator<ASpaceAsteroid> It(World); It; ++It) It->Destroy();
            Ship->SetActorLocation(FVector::ZeroVector);
            Rock = Target(FVector(350,0,0),2,2);
            Rock->DestructionEffectClass = ACombatBurst::StaticClass();
            Key(EKeys::SpaceBar,true);
            NextStep=Now+.08;
            break;
        case 1:
            Key(EKeys::SpaceBar,false);
            Test->TestEqual(TEXT("Space creates exactly one projectile"),CountActors<AShotProjectile>(World),1);
            Ship->TryFire(); Ship->TryFire();
            Test->TestEqual(TEXT("Cooldown blocks duplicate shots"),CountActors<AShotProjectile>(World),1);
            Test->TestTrue(TEXT("Muzzle effect appears"),CountActors<ACombatBurst>(World)>0);
            NextStep=Now+.35;
            break;
        case 2:
            Test->TestTrue(TEXT("First projectile damages once, without destroying two-hit asteroid"),Rock.IsValid() && Rock->RemainingHits==1);
            Test->TestEqual(TEXT("Projectile disappears on impact"),CountActors<AShotProjectile>(World),0);
            Key(EKeys::LeftMouseButton,true);
            NextStep=Now+.08;
            break;
        case 3:
            Key(EKeys::LeftMouseButton,false);
            Test->TestEqual(TEXT("Mouse button also fires"),CountActors<AShotProjectile>(World),1);
            NextStep=Now+.3;
            break;
        case 4:
            Test->TestFalse(TEXT("Second hit destroys asteroid"),Rock.IsValid());
            Test->TestTrue(TEXT("Destruction effect appears"),CountActors<ACombatBurst>(World)>0);
            ShortLived=World->SpawnActorDeferred<AShotProjectile>(AShotProjectile::StaticClass(),FTransform(FVector(-400,700,0)),Ship);
            ShortLived->Lifetime=.15f; ShortLived->Speed=100.f;
            ShortLived->FinishSpawning(FTransform(FVector(-400,700,0)));
            NextStep=Now+.25;
            break;
        case 5:
            Test->TestFalse(TEXT("Missed projectile expires"),ShortLived.IsValid());
            Rock=Target(FVector(300,0,0),2,2);
            OtherRock=Target(FVector(300,0,0),2,2);
            {
                auto* FastShot=World->SpawnActorDeferred<AShotProjectile>(AShotProjectile::StaticClass(),FTransform::Identity,Ship);
                FastShot->Speed=30000.f;
                FastShot->FinishSpawning(FTransform::Identity);
            }
            NextStep=Now+.15;
            break;
        case 6:
            Test->TestTrue(TEXT("Fast swept projectile hits one overlapping asteroid only"),
                Rock.IsValid() && OtherRock.IsValid() && Rock->RemainingHits+OtherRock->RemainingHits==3);
            Rock->Destroy(); OtherRock->Destroy();
            for (int32 Tier=0;Tier<3;++Tier)
            {
                auto* Sample=Target(FVector(400,800,0),Tier+1,Tier+1);
                auto* Mode=World->GetAuthGameMode<ASpaceGameMode>();
                const int32 Before=Mode->Score;
                const int32 ExpectedPoints=Tier==2?400:(Tier+1)*100;
                Test->TestEqual(TEXT("Size determines resistance"),Sample->RemainingHits,Tier+1);
                Test->TestEqual(TEXT("Size determines reward"),Sample->ScoreValue,ExpectedPoints);
                Test->TestTrue(TEXT("Size determines visible and collision scale"),FMath::IsNearlyEqual(Sample->GetActorScale3D().X,Sample->SizeScales[Tier]));
                for(int32 Hit=0;Hit<Tier;++Hit) Sample->ReceiveShot();
                Test->TestEqual(TEXT("Nonlethal hits do not award points"),Mode->Score,Before);
                Test->TestEqual(TEXT("Asteroid survives until its final hit"),Sample->RemainingHits,1);
                Sample->ReceiveShot(); Sample->ReceiveShot();
                Test->TestEqual(TEXT("Final hit awards exactly the category reward once"),Mode->Score,Before+ExpectedPoints);
            }
            Rock=Target(FVector(65,0,0),2,2);
            Ship->TryFire();
            NextStep=Now+.15;
            break;
        case 7:
            Test->TestTrue(TEXT("A projectile starting inside an asteroid still applies exactly one hit"),Rock.IsValid() && Rock->RemainingHits==1);
            return true;
        }
        return false;
    }
private:
    FAutomationTestBase* Test;
    int32 Step=0;
    double NextStep=0;
    TWeakObjectPtr<ASpaceAsteroid> Rock, OtherRock;
    TWeakObjectPtr<AShotProjectile> ShortLived;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatTest,"SpaceShooter.Gameplay.Combat",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatTest::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Maps/L_Arena")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
    ADD_LATENT_AUTOMATION_COMMAND(FCheckCombat(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
