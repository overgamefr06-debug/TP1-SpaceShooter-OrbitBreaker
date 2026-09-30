#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpacePickup.h"
#include "SpaceGameMode.generated.h"

class ASpaceAsteroid;
class UAudioComponent;
class USoundBase;
UENUM(BlueprintType)
enum class EOrbitUICue : uint8 { Hover, Select, Confirm, Back };
UENUM(BlueprintType)
enum class ESpaceRunState : uint8 { Menu, Playing, GameOver };

UCLASS()
class SPACESHOOTER_API ASpaceGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ASpaceGameMode();
    UFUNCTION(BlueprintCallable, Category="Audio") void PlayUICue(EOrbitUICue Cue);
    UFUNCTION(BlueprintCallable, Category="Interface") void RequestQuit();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio") TObjectPtr<UAudioComponent> MenuPlayer;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio") TObjectPtr<UAudioComponent> GamePlayer;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio") TObjectPtr<USoundBase> MenuMusic;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio") TObjectPtr<USoundBase> GameMusic;
    // Ordered like EOrbitUICue: hover, select, confirm, back.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio") TArray<TObjectPtr<USoundBase>> InterfaceSounds;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio", meta=(ClampMin="0",ClampMax="1")) float MenuMusicVolume = .55f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio", meta=(ClampMin="0",ClampMax="1")) float GameMusicVolume = .38f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio", meta=(ClampMin="0",ClampMax="1")) float InterfaceVolume = .5f;
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION(BlueprintCallable, Category="Run") void StartRun();
    UFUNCTION(BlueprintCallable, Category="Run") void ReturnToMenu();
    UFUNCTION(BlueprintCallable, Category="Run") bool LoseLife();
    UFUNCTION(BlueprintCallable, Category="Run") void AwardAsteroid(int32 Points = 100);
    UFUNCTION(BlueprintCallable, Category="Fleet") void SelectShip(int32 Index);
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Fleet") int32 SelectedShip = 0;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Fleet") int32 BestScore = 0;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fleet") FIntVector UnlockScores = FIntVector(0,5000,15000);
    UFUNCTION(BlueprintPure, Category="Fleet") bool IsShipUnlocked(int32 Index) const;
    void SaveProgress();
    void LoadProgress();
    FString ProgressSlot = TEXT("OrbitBreakerProgress_v1");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fleet") bool bPersistProgress = true;
    UFUNCTION(BlueprintCallable, Category="Bonus") void ActivateBonus(ESpaceBonus Type);
    UFUNCTION(BlueprintPure, Category="Bonus") float BonusSeconds(ESpaceBonus Type) const;
    UFUNCTION(BlueprintCallable, Category="Bonus") ASpacePickup* SpawnBonus();
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bonus") TArray<TSubclassOf<ASpacePickup>> PickupClasses;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bonus") float DoubleScoreDuration = 15.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bonus") float ShieldDuration = 10.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bonus") float TripleShotDuration = 15.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bonus") float MinimumBonusDelay = 12.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bonus") float MaximumBonusDelay = 18.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bonus",meta=(ClampMin="0",ClampMax="1")) float RepairProbability = .05f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bonus") bool bBonusesEnabled = true;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Bonus") FString BonusMessage;
    float BonusMessageUntil = 0;
    UFUNCTION(BlueprintCallable, Category="Asteroids") bool SplitAsteroidPair(ASpaceAsteroid* A,ASpaceAsteroid* B);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Asteroids") TSubclassOf<class ACombatBurst> CollisionEffectClass;
    UFUNCTION(BlueprintCallable, Category="Asteroids") ASpaceAsteroid* SpawnAsteroid();
    UFUNCTION(BlueprintPure, Category="Run") bool IsPlaying() const { return State == ESpaceRunState::Playing; }
    UFUNCTION(BlueprintPure, Category="Run") bool IsInvulnerable() const;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Run") ESpaceRunState State = ESpaceRunState::Menu;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Run") int32 Score = 0;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Run") int32 Lives = 3;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Run") float SurvivalTime = 0;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Run") int32 StartingLives = 3;
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
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    void UpdateMusic();
    bool bMusicInitialized = false, bGameMusicActive = false, bQuitPending = false;
    double LastHoverTime = -100.;
    FTimerHandle QuitTimer;
    void ClearCombatActors();
    void SetMenuInput(bool bMenu);
    double ProtectedUntil = 0;
    float SpawnCountdown = 1.f;
    float BonusCountdown = 8.f;
    double DoubleScoreUntil = 0, ShieldUntil = 0, TripleShotUntil = 0;
};
