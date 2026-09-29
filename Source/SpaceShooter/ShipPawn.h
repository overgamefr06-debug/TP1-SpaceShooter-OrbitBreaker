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

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void Move(const FInputActionValue& Value);
    UPROPERTY() TObjectPtr<UInputAction> MoveAction;
    UPROPERTY() TObjectPtr<UInputMappingContext> MappingContext;
};
