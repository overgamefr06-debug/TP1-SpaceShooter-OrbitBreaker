#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SpaceHUD.generated.h"

/** A resolution-independent arcade HUD. The Blueprint child owns art direction and copy. */
UCLASS()
class SPACESHOOTER_API ASpaceHUD : public AHUD
{
    GENERATED_BODY()
public:
    ASpaceHUD();
    virtual void DrawHUD() override;
    virtual void NotifyHitBoxClick(FName BoxName) override;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interface") FString GameTitle = TEXT("ORBIT BREAKER");
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interface") TObjectPtr<UFont> TextFont;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interface") FString Author = TEXT("Kevin Ortega");
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interface") FLinearColor Accent = FLinearColor(.13f,.86f,1.f);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interface") FLinearColor Warm = FLinearColor(1.f,.49f,.2f);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interface|Art") TArray<TObjectPtr<UTexture2D>> ShipPortraits;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interface|Art") TObjectPtr<UTexture2D> ScoreEmblem;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interface|Art") TObjectPtr<UTexture2D> TitleLogo;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interface|Art") TArray<TObjectPtr<UTexture2D>> BonusIcons;
private:
    float Scale = 1, OffsetX = 0, OffsetY = 0;
    void Label(const FString& Text, float X, float Y, float Size, FLinearColor Color, bool Center = false);
    void Panel(float X, float Y, float Width, float Height, FLinearColor Color);
    void Button(FName Name, const FString& Text, float X, float Y, bool bPrimary);
    void Sprite(UTexture2D* Texture,float X,float Y,float W,float H,FLinearColor Tint=FLinearColor::White);
    void Line(float X,float Y,float X2,float Y2,FLinearColor Color,float Width=1.f);
    void Frame(float X,float Y,float W,float H,FLinearColor Color,bool Active);
    bool Hover(float X,float Y,float W,float H) const;
};
