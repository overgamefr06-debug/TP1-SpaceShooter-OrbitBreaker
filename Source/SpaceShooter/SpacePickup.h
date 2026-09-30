#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpacePickup.generated.h"

UENUM(BlueprintType)
enum class ESpaceBonus : uint8 { DoubleScore, Shield, Repair, TripleShot };

UCLASS()
class SPACESHOOTER_API ASpacePickup : public AActor
{
    GENERATED_BODY()
public:
    ASpacePickup();
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Bonus") TObjectPtr<class USphereComponent> Collision;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Bonus") TObjectPtr<class UStaticMeshComponent> Visual;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Bonus") ESpaceBonus BonusType = ESpaceBonus::DoubleScore;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Bonus") float Lifetime = 12.f;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Bonus") TObjectPtr<USoundBase> CollectSound;
protected:
    virtual void BeginPlay() override;
private:
    bool bCollected = false;
    float Age = 0.f;
    UFUNCTION() void OnContact(UPrimitiveComponent* Component,AActor* Other,UPrimitiveComponent* OtherComponent,int32 BodyIndex,bool bSweep,const FHitResult& Hit);
};
