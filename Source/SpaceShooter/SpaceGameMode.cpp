#include "SpaceGameMode.h"
#include "ShipPawn.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ASpaceGameMode::ASpaceGameMode()
{
    DefaultPawnClass = AShipPawn::StaticClass();
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
        PC->SetInputMode(FInputModeGameOnly());
    }
}
