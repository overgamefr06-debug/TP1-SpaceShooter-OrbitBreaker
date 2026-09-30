#include "SpacePickup.h"
#include "SpaceGameMode.h"
#include "ShipPawn.h"
#include "CombatBurst.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ASpacePickup::ASpacePickup()
{
    PrimaryActorTick.bCanEverTick=true;
    Collision=CreateDefaultSubobject<USphereComponent>(TEXT("PickupRange"));
    SetRootComponent(Collision); Collision->InitSphereRadius(32);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
    Collision->SetGenerateOverlapEvents(true);
    Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BonusSprite"));
    Visual->SetupAttachment(Collision); Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visual->SetCastShadow(false);
    Aura=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnergyMotes"));
    Aura->SetupAttachment(Collision);
    Aura->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Aura->SetCastShadow(false);
    Aura->bUseAsOccluder=false;
    Aura->SetRelativeLocation(FVector(0,0,-1));
    Aura->SetRelativeScale3D(FVector(.65f));
}
void ASpacePickup::BeginPlay()
{
    Super::BeginPlay();
    Aura->SetStaticMesh(AuraMesh);
    if(AuraMaterial) Aura->SetMaterial(0,AuraMaterial);
    Collision->OnComponentBeginOverlap.AddDynamic(this,&ASpacePickup::OnContact);
    SetLifeSpan(FMath::Max(1.f,Lifetime));
}
void ASpacePickup::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds); Age+=DeltaSeconds;
    const float Pulse=1.f+.06f*FMath::Sin(Age*4.f);
    Visual->SetRelativeScale3D(FVector(.48f*Pulse));
    Visual->SetRelativeLocation(FVector(FMath::Sin(Age*2.f)*3.f,0,2));
    Visual->SetVisibility(Age<Lifetime-2.f || FMath::Fmod(Age,.22f)>.06f);
    Aura->SetVisibility(Visual->IsVisible());
}
void ASpacePickup::OnContact(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,int32,bool,const FHitResult&)
{
    auto* Mode=GetWorld()->GetAuthGameMode<ASpaceGameMode>();
    if(bCollected || !Cast<AShipPawn>(Other) || !Mode || !Mode->IsPlaying()) return;
    bCollected=true;
    Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mode->ActivateBonus(BonusType);
    if(CollectSound) UGameplayStatics::PlaySound2D(this,CollectSound,.4f);
    if(CollectEffectClass) GetWorld()->SpawnActor<ACombatBurst>(CollectEffectClass,GetActorLocation()+FVector(0,0,4),FRotator::ZeroRotator);
    Destroy();
}
