#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "ShipPawn.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UFloatingPawnMovement;
class UInputAction;
class UInputMappingContext;
class AShotProjectile;
class ACombatBurst;

UCLASS()
class SPACESHOOTER_API AShipPawn : public APawn
{
    GENERATED_BODY()
public:
    AShipPawn();
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship|Components")
    TObjectPtr<UBoxComponent> Collision;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship|Components")
    TObjectPtr<UStaticMeshComponent> Hull;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship|Components")
    TObjectPtr<UStaticMeshComponent> Wings;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship|Components") TObjectPtr<UStaticMeshComponent> EngineGlow;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship|Components") TObjectPtr<UStaticMeshComponent> ShieldGlow;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ship|Effects") TObjectPtr<UStaticMesh> ShieldMesh;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ship|Effects") TObjectPtr<UMaterialInterface> ShieldMaterial;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship|Components")
    TObjectPtr<UFloatingPawnMovement> Movement;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship|Movement", meta=(ClampMin="1"))
    float MoveSpeed = 650.f;
    // X = vertical extent; Y = horizontal extent, measured from the arena centre.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship|Movement")
    FVector2D ArenaHalfSize = FVector2D(500.f, 900.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship|Weapon")
    TSubclassOf<AShotProjectile> ProjectileClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship|Weapon")
    TSubclassOf<ACombatBurst> MuzzleEffectClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship|Weapon", meta=(ClampMin="0.05"))
    float FireInterval = .22f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship|Weapon", meta=(ClampMin="50"))
    float MuzzleOffset = 65.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ship|Fleet") TArray<TObjectPtr<UMaterialInterface>> ShipMaterials;
    UFUNCTION(BlueprintCallable, Category="Ship|Fleet") void ApplyShipStyle(int32 Index);
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ship|Fleet") int32 ShipStyle = 0;

    UFUNCTION(BlueprintCallable, Category="Ship|Weapon")
    void TryFire();
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship|Effects") TSubclassOf<ACombatBurst> DamageEffectClass;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void Move(const FInputActionValue& Value);
    void RestartArena();
    void OpenMenu();
    void StartFromMenu();
    UFUNCTION() void OnContact(UPrimitiveComponent* Overlapped, AActor* Other, UPrimitiveComponent* OtherComponent, int32 BodyIndex, bool bSweep, const FHitResult& Hit);
    double LastShotTime = -1.e10;
    UPROPERTY() TObjectPtr<UInputAction> MoveAction;
    UPROPERTY() TObjectPtr<UInputAction> FireAction;
    UPROPERTY() TObjectPtr<UInputAction> RestartAction;
    UPROPERTY() TObjectPtr<UInputAction> MenuAction;
    UPROPERTY() TObjectPtr<UInputAction> StartAction;
    UPROPERTY() TObjectPtr<UInputMappingContext> MappingContext;
};
