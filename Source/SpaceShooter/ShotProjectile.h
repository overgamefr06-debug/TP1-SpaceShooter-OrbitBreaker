#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShotProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class SPACESHOOTER_API AShotProjectile : public AActor
{
    GENERATED_BODY()
public:
    AShotProjectile();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
    TObjectPtr<USphereComponent> Collision;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
    TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
    TObjectPtr<UProjectileMovementComponent> Movement;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Projectile", meta=(ClampMin="1"))
    float Speed = 1400.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Projectile", meta=(ClampMin="0.05"))
    float Lifetime = 2.f;
protected:
    virtual void BeginPlay() override;
private:
    bool bConsumed = false;
    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* Component, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
        int32 BodyIndex, bool bFromSweep, const FHitResult& Hit);
};
