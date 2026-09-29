#include "SpaceAsteroid.h"
#include "CombatBurst.h"
#include "SpaceGameMode.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ASpaceAsteroid::ASpaceAsteroid()
{
    PrimaryActorTick.bCanEverTick = true;
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->InitSphereRadius(52.f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Collision->SetEnableGravity(false);
    Collision->BodyInstance.bLockZTranslation = true;
    Collision->BodyInstance.bLockXRotation = true;
    Collision->BodyInstance.bLockYRotation = true;
    Collision->BodyInstance.bLockZRotation = true;
    Collision->SetLinearDamping(0.f);
    Collision->SetGenerateOverlapEvents(true);
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Collision);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Mesh->SetStaticMesh(Sphere.Object);
    Mesh->SetRelativeScale3D(FVector(1.15f, 1.f, .75f));
}

void ASpaceAsteroid::BeginPlay()
{
    Super::BeginPlay();
    const int32 Lower = FMath::Max(1, MinimumHits);
    const int32 Upper = FMath::Max(Lower, MaximumHits);
    // Roll once when this asteroid appears, never again during an impact.
    RemainingHits = FMath::RandRange(Lower, Upper);
    Spin = FMath::FRandRange(-38.f, 38.f);
    if (!MeshVariants.IsEmpty())
    {
        Mesh->SetStaticMesh(MeshVariants[FMath::RandHelper(MeshVariants.Num())]);
        Mesh->SetRelativeScale3D(FVector::OneVector);
        const float Scale = FMath::FRandRange(FMath::Max(.2f, MinimumScale), FMath::Max(MinimumScale, MaximumScale));
        SetActorScale3D(FVector(Scale));
        Mesh->SetRelativeRotation(FRotator(0,FMath::FRandRange(0.f,360.f),0));
    }
}

void ASpaceAsteroid::Launch(FVector Velocity)
{
    Velocity.Z = 0;
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Collision->SetSimulatePhysics(true);
    // One initial physical impulse; movement is then handled by Chaos, without homing.
    Collision->AddImpulse(Velocity, NAME_None, true);
    bMoving = true;
    SetLifeSpan(30.f);
}

void ASpaceAsteroid::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    Mesh->AddLocalRotation(FRotator(0,Spin * DeltaSeconds,0));
    if (bMoving && (FMath::Abs(GetActorLocation().X) > 850.f || FMath::Abs(GetActorLocation().Y) > 1300.f)) Destroy();
}

void ASpaceAsteroid::ReceiveShot()
{
    if (bDestroyedByShot || RemainingHits <= 0) return;
    --RemainingHits;
    if (RemainingHits == 0)
    {
        bDestroyedByShot = true;
        if (auto* Mode = GetWorld()->GetAuthGameMode<ASpaceGameMode>()) Mode->AwardAsteroid();
        Collision->SetSimulatePhysics(false);
        Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        if (DestructionEffectClass)
            GetWorld()->SpawnActor<ACombatBurst>(DestructionEffectClass, GetActorLocation(), FRotator::ZeroRotator);
        Destroy();
    }
}
