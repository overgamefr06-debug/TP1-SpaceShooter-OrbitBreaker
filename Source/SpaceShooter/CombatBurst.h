#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatBurst.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;

UCLASS()
class SPACESHOOTER_API ACombatBurst : public AActor
{
    GENERATED_BODY()
public:
    ACombatBurst();
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect", meta=(ClampMin="0.01"))
    float Duration = .4f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect", meta=(ClampMin="0"))
    float ExpansionRadius = 90.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect", meta=(ClampMin="0.1"))
    float FragmentSize = 8.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect", meta=(ClampMin="0",ClampMax="16"))
    int32 FragmentCount = 16;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
    TObjectPtr<UMaterialInterface> Material;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect") TObjectPtr<USoundBase> Sound;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect") TObjectPtr<UStaticMesh> FragmentMesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect") TObjectPtr<UStaticMesh> ShockwaveMesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect") TObjectPtr<UMaterialInterface> ShockwaveMaterial;
protected:
    virtual void BeginPlay() override;
private:
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Fragments;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Shockwave;
    UPROPERTY() TObjectPtr<class UMaterialInstanceDynamic> WaveDynamic;
    float Age = 0.f;
};
