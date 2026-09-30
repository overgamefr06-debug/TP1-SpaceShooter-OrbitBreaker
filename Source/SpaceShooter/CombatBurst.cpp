#include "CombatBurst.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

ACombatBurst::ACombatBurst()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    Shockwave=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shockwave"));
    Shockwave->SetupAttachment(RootComponent);
    Shockwave->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Shockwave->SetCastShadow(false);
    Shockwave->bUseAsOccluder=false;
}

void ACombatBurst::BeginPlay()
{
    Super::BeginPlay();
    Shockwave->SetStaticMesh(ShockwaveMesh);
    if(ShockwaveMaterial)
    {
        WaveDynamic=UMaterialInstanceDynamic::Create(ShockwaveMaterial,this);
        Shockwave->SetMaterial(0,WaveDynamic);
    }
    USoundBase* SelectedSound=SoundVariants.IsEmpty()?Sound.Get():SoundVariants[FMath::RandHelper(SoundVariants.Num())].Get();
    if (SelectedSound) UGameplayStatics::PlaySound2D(this, SelectedSound, .28f,
        SoundVariants.IsEmpty()?FMath::FRandRange(.92f,1.08f):FMath::FRandRange(.98f,1.02f));
    SetLifeSpan(FMath::Max(.01f, Duration));
}

void ACombatBurst::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    Age += DeltaSeconds;
    const float Progress = FMath::Clamp(Age / FMath::Max(.01f, Duration), 0.f, 1.f);
    if(WaveDynamic) WaveDynamic->SetScalarParameterValue(TEXT("Strength"),FMath::Square(1.f-Progress));
    Shockwave->SetRelativeScale3D(FVector((ExpansionRadius/50.f)*Progress));
    Shockwave->SetVisibility(Progress<.75f);
}
