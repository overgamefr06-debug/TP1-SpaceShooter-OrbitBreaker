#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceAsteroid.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ACombatBurst;

UCLASS()
class SPACESHOOTER_API ASpaceAsteroid : public AActor
{
    GENERATED_BODY()
public:
    ASpaceAsteroid();
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION(BlueprintCallable, Category="Asteroid|Movement") void Launch(FVector Velocity);
    bool CausesContactDamage() const { return bMoving; }
    UFUNCTION(BlueprintCallable, Category="Asteroid")
    void ReceiveShot();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Asteroid")
    TObjectPtr<USphereComponent> Collision;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Asteroid")
    TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid|Resistance", meta=(ClampMin="1"))
    int32 MinimumHits = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid|Resistance", meta=(ClampMin="1"))
    int32 MaximumHits = 3;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Asteroid|Resistance")
    int32 RemainingHits = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid|Effects")
    TSubclassOf<ACombatBurst> DestructionEffectClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid|Art") TArray<TObjectPtr<UStaticMesh>> MeshVariants;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid|Art") float MinimumScale = .65f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid|Art") float MaximumScale = 1.05f;
protected:
    virtual void BeginPlay() override;
private:
    bool bDestroyedByShot = false;
    bool bMoving = false;
    float Spin = 0;
};
