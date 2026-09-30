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
    Mesh->SetCastShadow(false);
    Mesh->bUseAsOccluder=false;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Mesh->SetStaticMesh(Sphere.Object);
    Mesh->SetRelativeScale3D(FVector(1.15f, 1.f, .75f));
}

void ASpaceAsteroid::BeginPlay()
{
    Super::BeginPlay();
    Collision->OnComponentBeginOverlap.AddDynamic(this,&ASpaceAsteroid::OnRockOverlap);
    FragmentAfter=GetWorld()->GetTimeSeconds()+FMath::Max(.1f,FragmentGraceSeconds);
    // Pick a category once: silhouette, resistance and reward stay linked.
    if (bRandomSize) SizeClass = static_cast<EAsteroidSize>(FMath::RandRange(0,2));
    const int32 Tier = FMath::Clamp(static_cast<int32>(SizeClass),0,2);
    RemainingHits = FMath::Max(1, HitsBySize[Tier]);
    InitialHits=RemainingHits;
    ScoreValue = FMath::Max(0, PointsBySize[Tier]);
    SetActorScale3D(FVector(FMath::Max(.1, SizeScales[Tier])));
    Spin = FMath::FRandRange(-24.f, 24.f);
    if(!RockMaterials.IsEmpty()) Mesh->SetMaterial(0,RockMaterials[FMath::RandHelper(RockMaterials.Num())]);
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
        if (auto* Mode = GetWorld()->GetAuthGameMode<ASpaceGameMode>()) Mode->AwardAsteroid(ScoreValue);
        Collision->SetSimulatePhysics(false);
        Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        if (DestructionEffectClass)
            GetWorld()->SpawnActor<ACombatBurst>(DestructionEffectClass, GetActorLocation(), FRotator::ZeroRotator);
        Destroy();
    }
}

bool ASpaceAsteroid::CanFragment() const
{
    return bMoving && !bDestroyedByShot && RemainingHits>0 && SizeClass!=EAsteroidSize::Small && GetWorld()->GetTimeSeconds()>=FragmentAfter;
}
void ASpaceAsteroid::ConsumeForFragmentation()
{
    bDestroyedByShot=true; RemainingHits=0; bMoving=false;
    Collision->SetSimulatePhysics(false);
    Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void ASpaceAsteroid::OnRockOverlap(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,int32,bool,const FHitResult&)
{
    if(auto* Rock=Cast<ASpaceAsteroid>(Other))
        if(auto* Mode=GetWorld()->GetAuthGameMode<ASpaceGameMode>()) Mode->SplitAsteroidPair(this,Rock);
}
