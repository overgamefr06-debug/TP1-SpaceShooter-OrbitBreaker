#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpaceGameMode.generated.h"

UCLASS()
class SPACESHOOTER_API ASpaceGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ASpaceGameMode();
protected:
    virtual void BeginPlay() override;
};
