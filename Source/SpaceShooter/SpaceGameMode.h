#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpaceGameMode.generated.h"

class ASpaceAsteroid;
UENUM(BlueprintType)
enum class ESpaceRunState : uint8 { Menu, Playing, GameOver };

UCLASS()
class SPACESHOOTER_API ASpaceGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ASpaceGameMode();
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION(BlueprintCallable, Category="Run") void StartRun();
    UFUNCTION(BlueprintCallable, Category="Run") void ReturnToMenu();
    UFUNCTION(BlueprintCallable, Category="Run") bool LoseLife();
    UFUNCTION(BlueprintCallable, Category="Run") void AwardAsteroid();
    UFUNCTION(BlueprintCallable, Category="Asteroids") ASpaceAsteroid* SpawnAsteroid();
    UFUNCTION(BlueprintPure, Category="Run") bool IsPlaying() const { return State == ESpaceRunState::Playing; }
    UFUNCTION(BlueprintPure, Category="Run") bool IsInvulnerable() const;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Run") ESpaceRunState State = ESpaceRunState::Menu;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Run") int32 Score = 0;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Run") int32 Lives = 3;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Run") float SurvivalTime = 0;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Run") int32 StartingLives = 3;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Run") int32 PointsPerAsteroid = 100;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Run", meta=(ClampMin="0.1")) float InvulnerabilitySeconds = 1.5f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Asteroids") TSubclassOf<ASpaceAsteroid> AsteroidClass;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Asteroids") FVector2D SpawnHalfSize = FVector2D(615, 1060);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Asteroids", meta=(ClampMin="0.1")) float MinimumSpawnDelay = .65f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Asteroids", meta=(ClampMin="0.1")) float MaximumSpawnDelay = 1.35f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Asteroids") float MinimumSpeed = 135.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Asteroids") float MaximumSpeed = 235.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Asteroids") int32 MaximumAsteroids = 24;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Asteroids") bool bSpawningEnabled = true;
protected:
    virtual void BeginPlay() override;
private:
    void ClearCombatActors();
    void SetMenuInput(bool bMenu);
    double ProtectedUntil = 0;
    float SpawnCountdown = 1.f;
};
