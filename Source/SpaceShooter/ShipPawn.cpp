#include "ShipPawn.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Engine/LocalPlayer.h"
#include "UObject/ConstructorHelpers.h"

AShipPawn::AShipPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->SetBoxExtent(FVector(38.f, 45.f, 18.f));
    Collision->SetCollisionProfileName(TEXT("Pawn"));
    Hull = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hull"));
    Hull->SetupAttachment(Collision);
    Hull->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Hull->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
    Hull->SetRelativeScale3D(FVector(.4f, .4f, .85f));
    Wings = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wings"));
    Wings->SetupAttachment(Collision);
    Wings->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Wings->SetRelativeLocation(FVector(-15.f, 0.f, 0.f));
    Wings->SetRelativeScale3D(FVector(.25f, .9f, .12f));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Hull->SetStaticMesh(Cone.Object);
    Wings->SetStaticMesh(Cube.Object);
    Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
    AddTickPrerequisiteComponent(Movement);
    Movement->SetPlaneConstraintEnabled(true);
    Movement->SetPlaneConstraintNormal(FVector::UpVector);
    Movement->Acceleration = 8000.f;
    Movement->Deceleration = 8000.f;
    MoveAction = CreateDefaultSubobject<UInputAction>(TEXT("MoveAction"));
    MoveAction->ValueType = EInputActionValueType::Axis2D;
}

void AShipPawn::BeginPlay()
{
    Super::BeginPlay();
    Movement->MaxSpeed = FMath::Max(1.f, MoveSpeed);
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !PC->GetLocalPlayer()) return;
    auto* Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
    if (!Input) return;
    MappingContext = NewObject<UInputMappingContext>(this);
    const auto Bind = [this](FKey Key, bool Vertical, bool Negative)
    {
        auto& Mapping = MappingContext->MapKey(MoveAction, Key);
        if (Negative) Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(MappingContext));
        if (Vertical)
        {
            auto* Swizzle = NewObject<UInputModifierSwizzleAxis>(MappingContext);
            Swizzle->Order = EInputAxisSwizzle::YXZ;
            Mapping.Modifiers.Add(Swizzle);
        }
    };
    Bind(EKeys::D, false, false); Bind(EKeys::Right, false, false);
    Bind(EKeys::A, false, true); Bind(EKeys::Q, false, true); Bind(EKeys::Left, false, true);
    Bind(EKeys::W, true, false); Bind(EKeys::Z, true, false); Bind(EKeys::Up, true, false);
    Bind(EKeys::S, true, true); Bind(EKeys::Down, true, true);
    Input->AddMappingContext(MappingContext, 0);
}

void AShipPawn::EndPlay(const EEndPlayReason::Type Reason)
{
    if (const APlayerController* PC = Cast<APlayerController>(GetController()))
        if (PC->GetLocalPlayer())
            if (auto* Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
                Input->RemoveMappingContext(MappingContext);
    Super::EndPlay(Reason);
}

void AShipPawn::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    if (auto* Enhanced = Cast<UEnhancedInputComponent>(Input))
        Enhanced->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShipPawn::Move);
}

void AShipPawn::Move(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>().GetClampedToMaxSize(1.f);
    AddMovementInput(FVector(Axis.Y, Axis.X, 0.f));
}

void AShipPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const FVector Position = GetActorLocation();
    const FVector Clamped(FMath::Clamp(Position.X, -ArenaHalfSize.X, ArenaHalfSize.X),
        FMath::Clamp(Position.Y, -ArenaHalfSize.Y, ArenaHalfSize.Y), 0.f);
    if (!Position.Equals(Clamped)) SetActorLocation(Clamped);
}
