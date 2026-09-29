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

    UFUNCTION(BlueprintCallable, Category="Ship|Weapon")
    void TryFire();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void Move(const FInputActionValue& Value);
    void RestartArena();
    double LastShotTime = -1.e10;
    UPROPERTY() TObjectPtr<UInputAction> MoveAction;
    UPROPERTY() TObjectPtr<UInputAction> FireAction;
    UPROPERTY() TObjectPtr<UInputAction> RestartAction;
    UPROPERTY() TObjectPtr<UInputMappingContext> MappingContext;
};
