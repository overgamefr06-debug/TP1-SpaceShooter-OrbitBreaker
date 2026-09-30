#include "ShipPawn.h"
#include "ShotProjectile.h"
#include "CombatBurst.h"
#include "SpaceAsteroid.h"
#include "SpaceGameMode.h"
#include "Engine/World.h"
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
#include "Materials/MaterialInstanceDynamic.h"

AShipPawn::AShipPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->SetBoxExtent(FVector(27.f, 31.f, 18.f));
    Collision->SetCollisionProfileName(TEXT("Pawn"));
    Collision->SetGenerateOverlapEvents(true);
    Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
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
    EngineGlow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EngineGlow"));
    EngineGlow->SetupAttachment(Collision);
    EngineGlow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShieldGlow=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldGlow"));
    ShieldGlow->SetupAttachment(Collision);
    ShieldGlow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShieldGlow->SetCastShadow(false);
    ShieldGlow->SetRelativeLocation(FVector(0,0,8));
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
    FireAction = CreateDefaultSubobject<UInputAction>(TEXT("FireAction"));
    FireAction->ValueType = EInputActionValueType::Boolean;
    RestartAction = CreateDefaultSubobject<UInputAction>(TEXT("RestartAction"));
    RestartAction->ValueType = EInputActionValueType::Boolean;
    MenuAction = CreateDefaultSubobject<UInputAction>(TEXT("MenuAction"));
    MenuAction->ValueType = EInputActionValueType::Boolean;
    StartAction = CreateDefaultSubobject<UInputAction>(TEXT("StartAction"));
    StartAction->ValueType = EInputActionValueType::Boolean;
    ProjectileClass = AShotProjectile::StaticClass();
}

void AShipPawn::BeginPlay()
{
    Super::BeginPlay();
    ShieldGlow->SetStaticMesh(ShieldMesh);
    if(ShieldMaterial)
    {
        ShieldDynamic=UMaterialInstanceDynamic::Create(ShieldMaterial,this);
        ShieldGlow->SetMaterial(0,ShieldDynamic);
    }
    Collision->OnComponentBeginOverlap.AddDynamic(this, &AShipPawn::OnContact);
    Movement->MaxSpeed = FMath::Max(1.f, MoveSpeed);
    ApplyShipStyle(0);
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
    MappingContext->MapKey(FireAction, EKeys::SpaceBar);
    MappingContext->MapKey(FireAction, EKeys::LeftMouseButton);
    MappingContext->MapKey(RestartAction, EKeys::R);
    MappingContext->MapKey(MenuAction, EKeys::Escape);
    MappingContext->MapKey(StartAction, EKeys::Enter);
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
    {
        Enhanced->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShipPawn::Move);
        Enhanced->BindAction(FireAction, ETriggerEvent::Triggered, this, &AShipPawn::TryFire);
        Enhanced->BindAction(RestartAction, ETriggerEvent::Started, this, &AShipPawn::RestartArena);
        Enhanced->BindAction(MenuAction, ETriggerEvent::Started, this, &AShipPawn::OpenMenu);
        Enhanced->BindAction(StartAction, ETriggerEvent::Started, this, &AShipPawn::StartFromMenu);
    }
}

void AShipPawn::ApplyShipStyle(int32 Index)
{
    if (!ShipMaterials.IsValidIndex(Index) || !ShipMaterials[Index]) return;
    ShipStyle = Index;
    Hull->SetMaterial(0, ShipMaterials[Index]);
}

void AShipPawn::TryFire()
{
    if (!ProjectileClass || !GetWorld()) return;
    if (auto* Mode = GetWorld()->GetAuthGameMode<ASpaceGameMode>(); Mode && !Mode->IsPlaying()) return;
    const double Now = GetWorld()->GetTimeSeconds();
    if (Now - LastShotTime < FMath::Max(.05f, FireInterval)) return;
    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const FVector Position = GetActorLocation() + FVector(FMath::Max(50.f, MuzzleOffset), 0.f, 0.f);
    auto* Mode=GetWorld()->GetAuthGameMode<ASpaceGameMode>();
    const bool Triple=Mode && Mode->BonusSeconds(ESpaceBonus::TripleShot)>0;
    bool Fired=false;
    for(int32 i=Triple?-1:0;i<=(Triple?1:0);++i)
    {
        const FVector Origin=Position+FVector(0,i*12.f,0);
        if(GetWorld()->SpawnActor<AShotProjectile>(ProjectileClass,Origin,FRotator(0,i*12.f,0),Params)) Fired=true;
    }
    if(Fired)
    {
        LastShotTime=Now;
        if(MuzzleEffectClass) GetWorld()->SpawnActor<ACombatBurst>(MuzzleEffectClass,Position,FRotator::ZeroRotator);
    }
}

void AShipPawn::RestartArena()
{
    if (auto* Mode = GetWorld()->GetAuthGameMode<ASpaceGameMode>(); Mode && Mode->State != ESpaceRunState::Menu) Mode->StartRun();
}

void AShipPawn::OpenMenu()
{
    if (auto* Mode = GetWorld()->GetAuthGameMode<ASpaceGameMode>()) Mode->ReturnToMenu();
}

void AShipPawn::StartFromMenu()
{
    if (auto* Mode = GetWorld()->GetAuthGameMode<ASpaceGameMode>(); Mode && !Mode->IsPlaying()) Mode->StartRun();
}

void AShipPawn::OnContact(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    auto* Rock = Cast<ASpaceAsteroid>(Other);
    auto* Mode = GetWorld()->GetAuthGameMode<ASpaceGameMode>();
    if (!Rock || !IsValid(Rock) || !Rock->CausesContactDamage() || !Mode || !Mode->IsPlaying()) return;
    const bool bDamaged = Mode->LoseLife();
    if(Mode->BonusSeconds(ESpaceBonus::Shield)>0) ShieldImpact=1.f;
    // A contact consumes the asteroid, including during the short recovery shield.
    Rock->Destroy();
    if (bDamaged && DamageEffectClass) GetWorld()->SpawnActor<ACombatBurst>(DamageEffectClass, GetActorLocation(), FRotator::ZeroRotator);
}

void AShipPawn::Move(const FInputActionValue& Value)
{
    if (auto* Mode = GetWorld()->GetAuthGameMode<ASpaceGameMode>(); Mode && !Mode->IsPlaying()) return;
    const FVector2D Axis = Value.Get<FVector2D>().GetClampedToMaxSize(1.f);
    AddMovementInput(FVector(Axis.Y, Axis.X, 0.f));
}

void AShipPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Mode = GetWorld()->GetAuthGameMode<ASpaceGameMode>();
    if (Mode && Mode->State == ESpaceRunState::Menu)
    {
        if (auto* PC = Cast<APlayerController>(GetController()))
        {
            if (PC->WasInputKeyJustPressed(EKeys::One) || PC->WasInputKeyJustPressed(EKeys::NumPadOne)) Mode->SelectShip(0);
            if (PC->WasInputKeyJustPressed(EKeys::Two) || PC->WasInputKeyJustPressed(EKeys::NumPadTwo)) Mode->SelectShip(1);
            if (PC->WasInputKeyJustPressed(EKeys::Three) || PC->WasInputKeyJustPressed(EKeys::NumPadThree)) Mode->SelectShip(2);
        }
    }
    const bool bBlink = Mode && Mode->IsPlaying() && Mode->IsInvulnerable() && Mode->BonusSeconds(ESpaceBonus::Shield)<=0 && FMath::Fmod(GetWorld()->GetTimeSeconds(), .16f) < .07f;
    Hull->SetVisibility(!bBlink);
    Wings->SetVisibility(!bBlink);
    EngineGlow->SetVisibility(!bBlink);
    const float ShieldSeconds=Mode && Mode->IsPlaying()?Mode->BonusSeconds(ESpaceBonus::Shield):0.f;
    const float Target=FMath::Clamp(ShieldSeconds/.45f,0.f,1.f);
    ShieldFade=FMath::FInterpConstantTo(ShieldFade,Target,DeltaSeconds,4.f);
    ShieldImpact=FMath::Max(0.f,ShieldImpact-DeltaSeconds*3.5f);
    if(!Mode || !Mode->IsPlaying()) ShieldFade=0.f;
    ShieldGlow->SetVisibility(ShieldFade>0.f);
    ShieldGlow->SetRelativeScale3D(FVector(1.04f,.89f,1.f)*FMath::Lerp(.65f,1.f,ShieldFade));
    if(ShieldDynamic)
    {
        ShieldDynamic->SetScalarParameterValue(TEXT("Strength"),ShieldFade);
        ShieldDynamic->SetScalarParameterValue(TEXT("Impact"),ShieldImpact);
    }
    EngineGlow->SetRelativeScale3D(FVector(.92f + .12f * FMath::Sin(GetWorld()->GetTimeSeconds() * 34.f),1,1)*.68f);
    const FVector Position = GetActorLocation();
    const FVector Clamped(FMath::Clamp(Position.X, -ArenaHalfSize.X, ArenaHalfSize.X),
        FMath::Clamp(Position.Y, -ArenaHalfSize.Y, ArenaHalfSize.Y), 0.f);
    if (!Position.Equals(Clamped)) SetActorLocation(Clamped);
}
