#include "CombatBurst.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ACombatBurst::ACombatBurst()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    for (int32 Index = 0; Index < 8; ++Index)
    {
        auto* Fragment = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Fragment%d"), Index));
        Fragment->SetupAttachment(RootComponent);
        Fragment->SetStaticMesh(Cube.Object);
        Fragment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Fragment->SetCastShadow(false);
        Fragments.Add(Fragment);
    }
}

void ACombatBurst::BeginPlay()
{
    Super::BeginPlay();
    for (const auto& Fragment : Fragments)
    {
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
    for (int32 Index = 0; Index < Fragments.Num(); ++Index)
    {
        const float Angle = Index * (2.f * PI / Fragments.Num());
        Fragments[Index]->SetRelativeLocation(FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * ExpansionRadius * Progress);
        Fragments[Index]->SetRelativeScale3D(FVector((FragmentSize / 100.f) * (1.f - Progress)));
    }
}
