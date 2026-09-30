#include "CombatBurst.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
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
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    for (int32 Index = 0; Index < 16; ++Index)
    {
        auto* Fragment = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Fragment%d"), Index));
        Fragment->SetupAttachment(RootComponent);
        Fragment->SetStaticMesh(Cube.Object);
        Fragment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Fragment->SetCastShadow(false);
        Fragment->bUseAsOccluder=false;
        // Safe initial bounds even before Blueprint defaults and BeginPlay.
        Fragment->SetRelativeScale3D(FVector(.001f));
        Fragments.Add(Fragment);
    }
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
    if (Sound) UGameplayStatics::PlaySound2D(this, Sound, .28f, FMath::FRandRange(.92f,1.08f));
    for (int32 Index=0;Index<Fragments.Num();++Index)
    {
        auto* Fragment=Fragments[Index].Get();
        Fragment->SetVisibility(Index<FragmentCount);
        if(FragmentMesh) Fragment->SetStaticMesh(FragmentMesh);
        Fragment->SetRelativeScale3D(FVector(FragmentSize / 100.f));
        if (Material) Fragment->SetMaterial(0, Material);
    }
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
    for (int32 Index = 0; Index < FMath::Clamp(FragmentCount,0,Fragments.Num()); ++Index)
    {
        const float Angle = Index * (2.f * PI / FMath::Max(1,FragmentCount));
        Fragments[Index]->SetRelativeLocation(FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * ExpansionRadius * Progress);
        Fragments[Index]->SetRelativeScale3D(FVector((FragmentSize / 100.f) * (1.f - Progress)));
    }
}
