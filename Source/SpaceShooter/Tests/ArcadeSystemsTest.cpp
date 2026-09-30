#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "../SpaceGameMode.h"
#include "../SpaceAsteroid.h"
#include "../SpacePickup.h"
#include "../ShipPawn.h"
#include "../ShotProjectile.h"
#include "../CombatBurst.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Editor.h"

template<class T> static int32 ArcadeCount(UWorld* W)
{
    int32 N=0; for(TActorIterator<T> It(W);It;++It) if(IsValid(*It)) ++N; return N;
}
template<class T> static void ArcadeClear(UWorld* W)
{
    for(TActorIterator<T> It(W);It;++It) It->Destroy();
}
class FCheckArcadeSystems : public IAutomationLatentCommand
{
public:
    explicit FCheckArcadeSystems(FAutomationTestBase* T): Test(T) {}
    bool Update() override
    {
        UWorld* W=GEditor?GEditor->PlayWorld:nullptr;
        auto* Mode=W?W->GetAuthGameMode<ASpaceGameMode>():nullptr;
        auto* Ship=W?Cast<AShipPawn>(W->GetFirstPlayerController()->GetPawn()):nullptr;
        if(!Mode || !Ship) { Test->AddError(TEXT("Missing arcade world")); return true; }
        const double Now=W->GetTimeSeconds(); if(Now<Next) return false;
        auto Rock=[W,Mode](EAsteroidSize Size,FVector P,FVector Velocity) {
            auto* R=W->SpawnActorDeferred<ASpaceAsteroid>(Mode->AsteroidClass,FTransform(P),nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
            R->bRandomSize=false; R->SizeClass=Size;
            R->FinishSpawning(FTransform(P)); R->Launch(Velocity); return R;
        };
        auto CheckChildren=[this,W](EAsteroidSize Size) {
            Test->TestEqual(TEXT("Two parents become exactly three children"),ArcadeCount<ASpaceAsteroid>(W),3);
            for(TActorIterator<ASpaceAsteroid> It(W);It;++It)
            {
                Test->TestTrue(TEXT("Child category steps down once"),It->SizeClass==Size);
                Test->TestEqual(TEXT("Child has full category HP"),It->RemainingHits,int32(Size)+1);
            }
        };
        switch(Step++)
        {
        case 0:
            Test->TestFalse(TEXT("Automation is isolated from real save"),Mode->bPersistProgress);
            Mode->bSpawningEnabled=false; Mode->bBonusesEnabled=false;
            Mode->BestScore=0;
            Test->TestTrue(TEXT("Aegis starts available"),Mode->IsShipUnlocked(0));
            Test->TestFalse(TEXT("Spectre starts locked"),Mode->IsShipUnlocked(1));
            Mode->SelectShip(1); Test->TestEqual(TEXT("Locked selection rejected"),Mode->SelectedShip,0);
            Mode->BestScore=4999; Test->TestFalse(TEXT("4999 is insufficient"),Mode->IsShipUnlocked(1));
            Mode->BestScore=5000; Test->TestTrue(TEXT("5000 unlocks Spectre"),Mode->IsShipUnlocked(1));
            Mode->BestScore=14999; Test->TestFalse(TEXT("14999 is insufficient"),Mode->IsShipUnlocked(2));
            Mode->BestScore=15000; Test->TestTrue(TEXT("15000 unlocks Helios"),Mode->IsShipUnlocked(2));
            {
                const FString RealSlot=Mode->ProgressSlot;
                Mode->ProgressSlot=TEXT("OrbitAutomation_")+FGuid::NewGuid().ToString();
                Mode->bPersistProgress=true; Mode->SaveProgress(); Mode->BestScore=0; Mode->LoadProgress();
                Test->TestEqual(TEXT("Best score survives disk save and reload"),Mode->BestScore,15000);
                Test->TestTrue(TEXT("Saved score restores unlocks"),Mode->IsShipUnlocked(2));
                Test->TestTrue(TEXT("Temporary save cleaned up"),UGameplayStatics::DeleteGameInSlot(Mode->ProgressSlot,0));
                Mode->bPersistProgress=false; Mode->ProgressSlot=RealSlot;
            }
            Mode->BestScore=0; Mode->StartRun(); Mode->AwardAsteroid(3000); Mode->StartRun(); Mode->AwardAsteroid(3000);
            Test->TestEqual(TEXT("Best score is not accumulated between runs"),Mode->BestScore,3000);
            Test->TestFalse(TEXT("Two 3000 runs do not unlock Spectre"),Mode->IsShipUnlocked(1));
            Mode->StartRun(); Ship->SetActorLocation(FVector(-400,-700,0));
            A=Rock(EAsteroidSize::Large,FVector(200,-280,0),FVector(0,120,0));
            B=Rock(EAsteroidSize::Large,FVector(200,280,0),FVector(0,-120,0));
            Test->TestFalse(TEXT("Fresh fragments have collision grace"),Mode->SplitAsteroidPair(A.Get(),B.Get()));
            Next=Now+1.85; break;
        case 1:
            Test->TestFalse(TEXT("Large parent A consumed by real overlap"),A.IsValid());
            Test->TestFalse(TEXT("Large parent B consumed by real overlap"),B.IsValid());
            CheckChildren(EAsteroidSize::Medium);
            Test->TestEqual(TEXT("Collision fragmentation does not award score"),Mode->Score,0);
            Test->TestTrue(TEXT("Collision spawns visual/audio effect"),ArcadeCount<ACombatBurst>(W)>0);
            Test->TestFalse(TEXT("Repeated parent callback cannot split again"),Mode->SplitAsteroidPair(A.Get(),B.Get()));
            ArcadeClear<ASpaceAsteroid>(W);
            A=Rock(EAsteroidSize::Medium,FVector(200,-200,0),FVector(0,100,0));
            B=Rock(EAsteroidSize::Medium,FVector(200,200,0),FVector(0,-100,0));
            Next=Now+1.75; break;
        case 2:
            CheckChildren(EAsteroidSize::Small); ArcadeClear<ASpaceAsteroid>(W);
            A=Rock(EAsteroidSize::Small,FVector(200,-200,0),FVector(0,100,0));
            B=Rock(EAsteroidSize::Medium,FVector(200,200,0),FVector(0,-100,0));
            Next=Now+2.; break;
        case 3:
            Test->TestTrue(TEXT("Mixed sizes pass without fragmenting"),A.IsValid() && B.IsValid());
            Test->TestFalse(TEXT("Mixed sizes cannot split"),Mode->SplitAsteroidPair(A.Get(),B.Get()));
            ArcadeClear<ASpaceAsteroid>(W);
            Test->TestEqual(TEXT("Shield default is ten seconds"),Mode->ShieldDuration,10.f);
            Test->TestEqual(TEXT("Repair is rare: five percent"),Mode->RepairProbability,.05f);
            Mode->DoubleScoreDuration=.5f; Mode->ShieldDuration=.5f; Mode->TripleShotDuration=.5f;
            Mode->ActivateBonus(ESpaceBonus::DoubleScore); Mode->AwardAsteroid(400);
            Test->TestEqual(TEXT("Score multiplier doubles reward"),Mode->Score,800);
            Mode->ActivateBonus(ESpaceBonus::Shield);
            Test->TestFalse(TEXT("Shield blocks life loss"),Mode->LoseLife());
            Mode->Lives=1; Mode->ActivateBonus(ESpaceBonus::Repair);
            Test->TestEqual(TEXT("Repair adds exactly one life"),Mode->Lives,2);
            Mode->ActivateBonus(ESpaceBonus::Repair); Mode->ActivateBonus(ESpaceBonus::Repair);
            Test->TestEqual(TEXT("Repair capped at starting lives"),Mode->Lives,3);
            Mode->ActivateBonus(ESpaceBonus::TripleShot); Ship->TryFire();
            Test->TestEqual(TEXT("Triple emits three simultaneous projectiles"),ArcadeCount<AShotProjectile>(W),3);
            {
                bool L=false,C=false,R=false;
                for(TActorIterator<AShotProjectile> It(W);It;++It)
                { const float Y=It->Movement->Velocity.Y; L|=Y<-10; C|=FMath::Abs(Y)<1; R|=Y>10; }
                Test->TestTrue(TEXT("Triple spreads left, centre and right"),L&&C&&R);
            }
            Ship->TryFire(); Test->TestEqual(TEXT("Triple respects fire cooldown"),ArcadeCount<AShotProjectile>(W),3);
            Next=Now+.65; break;
        case 4:
            Test->TestEqual(TEXT("Triple expires"),Mode->BonusSeconds(ESpaceBonus::TripleShot),0.f);
            Test->TestEqual(TEXT("Shield expires"),Mode->BonusSeconds(ESpaceBonus::Shield),0.f);
            Test->TestTrue(TEXT("Damage resumes after shield expiry"),Mode->LoseLife());
            Mode->AwardAsteroid(100); Test->TestEqual(TEXT("Multiplier expiry restores base reward"),Mode->Score,900);
            ArcadeClear<AShotProjectile>(W); Ship->TryFire();
            Test->TestEqual(TEXT("Normal single shot resumes"),ArcadeCount<AShotProjectile>(W),1);
            Test->TestEqual(TEXT("Four real pickup Blueprint classes configured"),Mode->PickupClasses.Num(),4);
            if(Mode->PickupClasses.Num()!=4) return true;
            Mode->Lives=1;
            Pickup=W->SpawnActor<ASpacePickup>(Mode->PickupClasses[2],Ship->GetActorLocation()+FVector(0,180,0),FRotator::ZeroRotator);
            Ship->SetActorLocation(Pickup->GetActorLocation()); Next=Now+.1; break;
        case 5:
            Test->TestFalse(TEXT("Collected bonus removed"),Pickup.IsValid());
            Test->TestEqual(TEXT("Actual overlap repairs exactly once"),Mode->Lives,2);
            Mode->RepairProbability=1;
            Pickup=Mode->SpawnBonus();
            Test->TestTrue(TEXT("Repair probability controls spawned type"),Pickup.IsValid() && Pickup->BonusType==ESpaceBonus::Repair);
            if(Pickup.IsValid()) Pickup->Destroy();
            Mode->RepairProbability=0;
            for(int32 i=0;i<20;++i)
            {
                auto* P=Mode->SpawnBonus();
                Test->TestTrue(TEXT("Zero repair probability only creates common bonuses"),P && P->BonusType!=ESpaceBonus::Repair);
                if(P) P->Destroy();
            }
            Mode->ActivateBonus(ESpaceBonus::Shield); Mode->SpawnBonus(); Mode->StartRun();
            Test->TestEqual(TEXT("Restart clears pickup actors"),ArcadeCount<ASpacePickup>(W),0);
            Test->TestEqual(TEXT("Restart clears timed bonus"),Mode->BonusSeconds(ESpaceBonus::Shield),0.f);
            Test->TestEqual(TEXT("Restart preserves best score"),Mode->BestScore,3000);
            return true;
        }
        return false;
    }
private:
    FAutomationTestBase* Test;
    int32 Step=0; double Next=0;
    TWeakObjectPtr<ASpaceAsteroid> A,B;
    TWeakObjectPtr<ASpacePickup> Pickup;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArcadeSystemsTest,"SpaceShooter.Gameplay.ArcadeSystems",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FArcadeSystemsTest::RunTest(const FString&)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Maps/L_Arena")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(.5f));
    ADD_LATENT_AUTOMATION_COMMAND(FCheckArcadeSystems(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
