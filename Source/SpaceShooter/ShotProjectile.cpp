#include "ShotProjectile.h"
#include "SpaceAsteroid.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AShotProjectile::AShotProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->InitSphereRadius(8.f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    Collision->SetGenerateOverlapEvents(true);
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Collision);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetCastShadow(false);
    Mesh->bUseAsOccluder=false;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Mesh->SetStaticMesh(Cube.Object);
    Mesh->SetRelativeScale3D(FVector(.32f, .075f, .075f));
    Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
    Movement->SetUpdatedComponent(Collision);
    Movement->ProjectileGravityScale = 0.f;
    Movement->bSweepCollision = true;
    Movement->bRotationFollowsVelocity = true;
    Movement->bInitialVelocityInLocalSpace = true;
}

void AShotProjectile::BeginPlay()
{
    Super::BeginPlay();
    Collision->OnComponentBeginOverlap.AddDynamic(this, &AShotProjectile::OnOverlap);
    if (GetOwner()) Collision->IgnoreActorWhenMoving(GetOwner(), true);
    Movement->Velocity = GetActorForwardVector() * FMath::Max(1.f, Speed);
    Movement->MaxSpeed = FMath::Max(1.f, Speed);
    SetLifeSpan(FMath::Max(.05f, Lifetime));
}

void AShotProjectile::OnOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*,
    int32, bool, const FHitResult&)
{
    if (bConsumed || OtherActor == GetOwner()) return;
    if (ASpaceAsteroid* Asteroid = Cast<ASpaceAsteroid>(OtherActor))
    {
        // Consume first: repeated overlap notifications must never apply a second hit.
        bConsumed = true;
        Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Asteroid->ReceiveShot();
        Destroy();
    }
}
