#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceAsteroid.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ACombatBurst;

UENUM(BlueprintType)
enum class EAsteroidSize : uint8 { Small, Medium, Large };

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
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid|Size") bool bRandomSize = true;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid|Size") EAsteroidSize SizeClass = EAsteroidSize::Small;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Asteroid|Size") FVector SizeScales = FVector(.55f, .95f, 1.45f);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Asteroid|Size") FIntVector HitsBySize = FIntVector(1,2,3);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Asteroid|Size") FIntVector PointsBySize = FIntVector(100,200,400);
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Asteroid|Size") int32 ScoreValue = 100;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Asteroid|Resistance")
    int32 RemainingHits = 0;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Asteroid|Resistance") int32 InitialHits = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid|Collision") float FragmentGraceSeconds = .8f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid|Art") TArray<TObjectPtr<UMaterialInterface>> RockMaterials;
    bool CanFragment() const;
    void ConsumeForFragmentation();
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid|Effects")
    TSubclassOf<ACombatBurst> DestructionEffectClass;
protected:
    virtual void BeginPlay() override;
private:
    bool bDestroyedByShot = false;
    bool bMoving = false;
    float Spin = 0;
    double FragmentAfter = 0;
    UFUNCTION() void OnRockOverlap(UPrimitiveComponent* Component,AActor* Other,UPrimitiveComponent* OtherComponent,int32 BodyIndex,bool bSweep,const FHitResult& Hit);
};
