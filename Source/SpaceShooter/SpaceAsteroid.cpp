#include "SpaceAsteroid.h"
#include "CombatBurst.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ASpaceAsteroid::ASpaceAsteroid()
{
    PrimaryActorTick.bCanEverTick = false;
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->InitSphereRadius(52.f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
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
}

void ASpaceAsteroid::ReceiveShot()
{
    if (bDestroyedByShot || RemainingHits <= 0) return;
    --RemainingHits;
    if (RemainingHits == 0)
    {
        bDestroyedByShot = true;
        Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        if (DestructionEffectClass)
            GetWorld()->SpawnActor<ACombatBurst>(DestructionEffectClass, GetActorLocation(), FRotator::ZeroRotator);
        Destroy();
    }
}
