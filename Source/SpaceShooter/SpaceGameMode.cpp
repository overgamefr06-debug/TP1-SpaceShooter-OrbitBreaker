#include "SpaceGameMode.h"
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
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->bAutoManageActiveCameraTarget = false;
        auto* Camera = GetWorld()->SpawnActor<ACameraActor>(FVector(0, 0, 2000), FRotator(-90, 0, 0));
        Camera->GetCameraComponent()->ProjectionMode = ECameraProjectionMode::Orthographic;
        Camera->GetCameraComponent()->OrthoWidth = 2000.f;
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
}

void ASpaceGameMode::StartRun()
{
    ClearCombatActors();
    Score = 0;
    Lives = FMath::Max(1, StartingLives);
    SurvivalTime = 0;
    SpawnCountdown = 1.2f;
    ProtectedUntil = GetWorld()->GetTimeSeconds() + FMath::Max(.1f, InvulnerabilitySeconds);
    State = ESpaceRunState::Playing;
    if (auto* Ship = Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
    {
        Ship->SetActorLocation(FVector(-180, 0, 0));
        Ship->SetActorScale3D(FVector::OneVector);
        Ship->SetActorHiddenInGame(false);
        Ship->Movement->StopMovementImmediately();
    }
    SetMenuInput(false);
}

void ASpaceGameMode::ReturnToMenu()
{
    State = ESpaceRunState::Menu;
    ClearCombatActors();
    if (auto* Ship = Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
    {
        Ship->SetActorHiddenInGame(false);
        Ship->Movement->StopMovementImmediately();
        Ship->SetActorLocation(FVector(0, 400, 0));
        Ship->SetActorScale3D(FVector(2.7f));
    }
    SetMenuInput(true);
}

bool ASpaceGameMode::IsInvulnerable() const
{
    return GetWorld()->GetTimeSeconds() < ProtectedUntil;
}

bool ASpaceGameMode::LoseLife()
{
    if (!IsPlaying() || IsInvulnerable()) return false;
    Lives = FMath::Max(0, Lives - 1);
    ProtectedUntil = GetWorld()->GetTimeSeconds() + FMath::Max(.1f, InvulnerabilitySeconds);
    if (Lives == 0)
    {
        State = ESpaceRunState::GameOver;
        if (auto* Ship = Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
        {
            Ship->Movement->StopMovementImmediately();
            Ship->SetActorHiddenInGame(true);
        }
        SetMenuInput(true);
    }
    return true;
}

void ASpaceGameMode::AwardAsteroid()
{
    if (IsPlaying()) Score += FMath::Max(0, PointsPerAsteroid);
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
    if (!bSpawningEnabled) return;
    SpawnCountdown -= DeltaSeconds;
    if (SpawnCountdown <= 0)
    {
        SpawnAsteroid();
        const float Low = FMath::Max(.1f, MinimumSpawnDelay);
        SpawnCountdown = FMath::FRandRange(Low, FMath::Max(Low, MaximumSpawnDelay));
    }
}
