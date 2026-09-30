#include "SpaceGameMode.h"
#include "OrbitSaveGame.h"
#include "Components/SphereComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "ShipPawn.h"
#include "SpaceAsteroid.h"
#include "ShotProjectile.h"
#include "CombatBurst.h"
#include "SpaceHUD.h"
#include "EngineUtils.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ASpaceGameMode::ASpaceGameMode()
{
    DefaultPawnClass = AShipPawn::StaticClass();
    HUDClass = ASpaceHUD::StaticClass();
    PrimaryActorTick.bCanEverTick = true;
    AsteroidClass = ASpaceAsteroid::StaticClass();
}

void ASpaceGameMode::BeginPlay()
{
    Super::BeginPlay();
    // Automation never modifies the player's real progression save.
    if (FParse::Param(FCommandLine::Get(),TEXT("OrbitTestMode"))) bPersistProgress=false;
    LoadProgress();
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->bAutoManageActiveCameraTarget = false;
        auto* Camera = GetWorld()->SpawnActor<ACameraActor>(FVector(0, 0, 2000), FRotator(-90, 0, 0));
        Camera->GetCameraComponent()->ProjectionMode = ECameraProjectionMode::Orthographic;
        Camera->GetCameraComponent()->OrthoWidth = 2000.f;
        // Fixed 2D depth range: automatic ortho planes can slice coplanar sprites.
        Camera->GetCameraComponent()->SetAutoCalculateOrthoPlanes(false);
        Camera->GetCameraComponent()->SetUpdateOrthoPlanes(false);
        Camera->GetCameraComponent()->SetOrthoNearClipPlane(1.f);
        Camera->GetCameraComponent()->SetOrthoFarClipPlane(5000.f);
        Camera->GetCameraComponent()->AspectRatio = 16.f / 9.f;
        Camera->GetCameraComponent()->bConstrainAspectRatio = true;
        PC->SetViewTarget(Camera);
        SetMenuInput(true);
    }
    ReturnToMenu();
}

void ASpaceGameMode::SetMenuInput(bool bMenu)
{
    if (auto* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->bShowMouseCursor = bMenu;
        PC->bEnableClickEvents = true;
        if (bMenu)
        {
            FInputModeGameAndUI Input;
            Input.SetHideCursorDuringCapture(false);
            PC->SetInputMode(Input);
        }
        else PC->SetInputMode(FInputModeGameOnly());
    }
}

void ASpaceGameMode::ClearCombatActors()
{
    for (TActorIterator<ASpaceAsteroid> It(GetWorld()); It; ++It) It->Destroy();
    for (TActorIterator<AShotProjectile> It(GetWorld()); It; ++It) It->Destroy();
    for (TActorIterator<ACombatBurst> It(GetWorld()); It; ++It) It->Destroy();
    for (TActorIterator<ASpacePickup> It(GetWorld()); It; ++It) It->Destroy();
}

void ASpaceGameMode::StartRun()
{
    SaveProgress();
    ClearCombatActors();
    DoubleScoreUntil=ShieldUntil=TripleShotUntil=0;
    BonusMessageUntil=0; BonusCountdown=8.f;
    if(!IsShipUnlocked(SelectedShip)) SelectedShip=0;
    Score = 0;
    Lives = FMath::Max(1, StartingLives);
    SurvivalTime = 0;
    SpawnCountdown = 1.2f;
    ProtectedUntil = GetWorld()->GetTimeSeconds() + FMath::Max(.1f, InvulnerabilitySeconds);
    State = ESpaceRunState::Playing;
    if (auto* Ship = Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
    {
        Ship->ApplyShipStyle(SelectedShip);
        Ship->SetActorLocation(FVector(-180, 0, 0));
        Ship->SetActorScale3D(FVector::OneVector);
        Ship->SetActorHiddenInGame(false);
        Ship->Movement->StopMovementImmediately();
    }
    SetMenuInput(false);
}

void ASpaceGameMode::ReturnToMenu()
{
    SaveProgress();
    State = ESpaceRunState::Menu;
    ClearCombatActors();
    if (auto* Ship = Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
    {
        Ship->SetActorHiddenInGame(true);
        Ship->Movement->StopMovementImmediately();
        Ship->SetActorLocation(FVector(0, 400, 0));
        Ship->SetActorScale3D(FVector(2.7f));
    }
    SetMenuInput(true);
}

bool ASpaceGameMode::IsInvulnerable() const
{
    return GetWorld()->GetTimeSeconds() < ProtectedUntil || BonusSeconds(ESpaceBonus::Shield)>0;
}

bool ASpaceGameMode::LoseLife()
{
    if (!IsPlaying() || IsInvulnerable()) return false;
    Lives = FMath::Max(0, Lives - 1);
    ProtectedUntil = GetWorld()->GetTimeSeconds() + FMath::Max(.1f, InvulnerabilitySeconds);
    if (Lives == 0)
    {
        State = ESpaceRunState::GameOver;
        SaveProgress();
        if (auto* Ship = Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
        {
            Ship->Movement->StopMovementImmediately();
            Ship->SetActorHiddenInGame(true);
        }
        SetMenuInput(true);
    }
    return true;
}

void ASpaceGameMode::AwardAsteroid(int32 Points)
{
    if (!IsPlaying()) return;
    const int32 PreviousBest=BestScore;
    Score += FMath::Max(0, Points)*(BonusSeconds(ESpaceBonus::DoubleScore)>0?2:1);
    BestScore=FMath::Max(BestScore,Score);
    for(int32 i=1;i<3;++i)
        if(PreviousBest<UnlockScores[i] && BestScore>=UnlockScores[i])
        {
            BonusMessage=i==1?TEXT("SPECTRE DÉBLOQUÉ"):TEXT("HELIOS DÉBLOQUÉ");
            BonusMessageUntil=GetWorld()->GetTimeSeconds()+4.f;
            SaveProgress();
        }
}

void ASpaceGameMode::SelectShip(int32 Index)
{
    if (State != ESpaceRunState::Menu || !IsShipUnlocked(Index)) return;
    SelectedShip = Index;
    if (auto* Ship = Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(this, 0))) Ship->ApplyShipStyle(Index);
}

ASpaceAsteroid* ASpaceGameMode::SpawnAsteroid()
{
    if (!IsPlaying() || !AsteroidClass) return nullptr;
    int32 Count = 0;
    for (TActorIterator<ASpaceAsteroid> It(GetWorld()); It; ++It) if (IsValid(*It)) ++Count;
    if (Count >= FMath::Max(1, MaximumAsteroids)) return nullptr;
    const float X = FMath::Max(580.f, SpawnHalfSize.X);
    const float Y = FMath::Max(1000.f, SpawnHalfSize.Y);
    const int32 Side = FMath::RandRange(0, 3);
    const FVector Position = Side < 2 ? FVector(Side == 0 ? -X : X, FMath::FRandRange(-Y, Y), 0)
        : FVector(FMath::FRandRange(-X, X), Side == 2 ? -Y : Y, 0);
    FVector Target = FVector::ZeroVector;
    if (auto* Ship = UGameplayStatics::GetPlayerPawn(this, 0)) Target = Ship->GetActorLocation();
    Target += FVector(FMath::FRandRange(-180.f,180.f), FMath::FRandRange(-220.f,220.f), 0);
    const float Difficulty = 1.f + FMath::Min(SurvivalTime / 180.f, .5f);
    const float Speed = FMath::FRandRange(FMath::Max(1.f,MinimumSpeed), FMath::Max(MinimumSpeed,MaximumSpeed)) * Difficulty;
    auto* Rock = GetWorld()->SpawnActor<ASpaceAsteroid>(AsteroidClass, Position, FRotator::ZeroRotator);
    if (Rock) Rock->Launch((Target - Position).GetSafeNormal() * Speed);
    return Rock;
}

void ASpaceGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!IsPlaying()) return;
    SurvivalTime += DeltaSeconds;
    if(bBonusesEnabled)
    {
        BonusCountdown-=DeltaSeconds;
        if(BonusCountdown<=0)
        {
            SpawnBonus();
            BonusCountdown=FMath::FRandRange(FMath::Max(1.f,MinimumBonusDelay),FMath::Max(MinimumBonusDelay,MaximumBonusDelay));
        }
    }
    if (!bSpawningEnabled) return;
    SpawnCountdown -= DeltaSeconds;
    if (SpawnCountdown <= 0)
    {
        SpawnAsteroid();
        const float Low = FMath::Max(.1f, MinimumSpawnDelay);
        SpawnCountdown = FMath::FRandRange(Low, FMath::Max(Low, MaximumSpawnDelay));
    }
}

bool ASpaceGameMode::IsShipUnlocked(int32 Index) const
{
    return Index>=0 && Index<3 && (Index==0 || BestScore>=UnlockScores[Index]);
}
void ASpaceGameMode::SaveProgress()
{
    if(!bPersistProgress || BestScore<=0) return;
    auto* Save=Cast<UOrbitSaveGame>(UGameplayStatics::CreateSaveGameObject(UOrbitSaveGame::StaticClass()));
    Save->BestScore=BestScore;
    if(!UGameplayStatics::SaveGameToSlot(Save,ProgressSlot,0))
        UE_LOG(LogTemp,Warning,TEXT("Unable to save Orbit Breaker progression"));
}
void ASpaceGameMode::LoadProgress()
{
    if(bPersistProgress && UGameplayStatics::DoesSaveGameExist(ProgressSlot,0))
        if(auto* Save=Cast<UOrbitSaveGame>(UGameplayStatics::LoadGameFromSlot(ProgressSlot,0)))
            BestScore=FMath::Max(BestScore,Save->BestScore);
}
void ASpaceGameMode::EndPlay(const EEndPlayReason::Type Reason)
{
    SaveProgress(); Super::EndPlay(Reason);
}
float ASpaceGameMode::BonusSeconds(ESpaceBonus Type) const
{
    const double Until=Type==ESpaceBonus::DoubleScore?DoubleScoreUntil:Type==ESpaceBonus::Shield?ShieldUntil:Type==ESpaceBonus::TripleShot?TripleShotUntil:0;
    return FMath::Max(0.,Until-GetWorld()->GetTimeSeconds());
}
void ASpaceGameMode::ActivateBonus(ESpaceBonus Type)
{
    if(!IsPlaying()) return;
    const double Now=GetWorld()->GetTimeSeconds();
    switch(Type)
    {
        case ESpaceBonus::DoubleScore: DoubleScoreUntil=Now+DoubleScoreDuration; BonusMessage=TEXT("SCORE ×2"); break;
        case ESpaceBonus::Shield: ShieldUntil=Now+ShieldDuration; BonusMessage=TEXT("BOUCLIER · 10 S"); break;
        case ESpaceBonus::Repair: Lives=FMath::Min(StartingLives,Lives+1); BonusMessage=TEXT("RÉPARATION +1"); break;
        case ESpaceBonus::TripleShot: TripleShotUntil=Now+TripleShotDuration; BonusMessage=TEXT("TIR TRIPLE"); break;
    }
    BonusMessageUntil=Now+2.5f;
}
ASpacePickup* ASpaceGameMode::SpawnBonus()
{
    if(!IsPlaying() || PickupClasses.Num()!=4) return nullptr;
    int32 Count=0; for(TActorIterator<ASpacePickup> It(GetWorld());It;++It) if(IsValid(*It)) ++Count;
    if(Count>=2) return nullptr;
    int32 Type;
    if(FMath::FRand()<FMath::Clamp(RepairProbability,0.f,1.f)) Type=2;
    else { const int32 Common[]={0,1,3}; Type=Common[FMath::RandRange(0,2)]; }
    return GetWorld()->SpawnActor<ASpacePickup>(PickupClasses[Type],FVector(FMath::FRandRange(-340.f,340.f),FMath::FRandRange(-760.f,760.f),0),FRotator::ZeroRotator);
}
bool ASpaceGameMode::SplitAsteroidPair(ASpaceAsteroid* A,ASpaceAsteroid* B)
{
    if(!IsPlaying() || !IsValid(A) || !IsValid(B) || A==B || A->SizeClass!=B->SizeClass || !A->CanFragment() || !B->CanFragment()) return false;
    const auto NextSize=static_cast<EAsteroidSize>(static_cast<int32>(A->SizeClass)-1);
    const FVector Centre=(A->GetActorLocation()+B->GetActorLocation())*.5f;
    const TSubclassOf<ASpaceAsteroid> ChildClass=A->GetClass();
    const float Gap=FMath::Max(45.f,52.f*float(A->SizeScales[static_cast<int32>(NextSize)])*1.45f);
    // Consume BOTH parents before any collision callbacks from newly spawned children.
    A->ConsumeForFragmentation(); B->ConsumeForFragmentation();
    if(CollisionEffectClass) GetWorld()->SpawnActor<ACombatBurst>(CollisionEffectClass,Centre,FRotator::ZeroRotator);
    A->Destroy(); B->Destroy();
    const float StartAngle=FMath::FRandRange(0.f,2*PI);
    for(int32 i=0;i<3;++i)
    {
        const float Angle=StartAngle+i*2*PI/3;
        const FVector Direction(FMath::Cos(Angle),FMath::Sin(Angle),0);
        const FVector Position=Centre+Direction*Gap;
        auto* Child=GetWorld()->SpawnActorDeferred<ASpaceAsteroid>(ChildClass,FTransform(Position),nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if(!Child) continue;
        Child->bRandomSize=false; Child->SizeClass=NextSize;
        Child->FinishSpawning(FTransform(Position));
        Child->Launch(Direction*FMath::FRandRange(145.f,210.f));
    }
    return true;
}
