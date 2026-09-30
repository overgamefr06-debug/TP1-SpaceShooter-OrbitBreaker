#include "SpaceHUD.h"
#include "SpaceGameMode.h"
#include "SpaceAsteroid.h"
#include "ShipPawn.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "Fonts/SlateFontInfo.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

namespace FleetUI
{
    const TCHAR* Names[] = {TEXT("AEGIS"),TEXT("SPECTRE"),TEXT("HELIOS")};
    const TCHAR* Roles[] = {TEXT("INTERCEPTEUR"),TEXT("CHASSEUR FURTIF"),TEXT("CANONNIÈRE")};
    const FLinearColor Colors[] = {FLinearColor(.22f,.84f,1.f),FLinearColor(.72f,.38f,1.f),FLinearColor(1.f,.56f,.22f)};
}

ASpaceHUD::ASpaceHUD()
{
    static ConstructorHelpers::FObjectFinder<UFont> Font(TEXT("/Engine/EngineFonts/Roboto.Roboto"));
    TextFont=Font.Object;
}

void ASpaceHUD::Panel(float X,float Y,float W,float H,FLinearColor Color)
{
    DrawRect(Color,OffsetX+X*Scale,OffsetY+Y*Scale,W*Scale,H*Scale);
}

void ASpaceHUD::Line(float X,float Y,float X2,float Y2,FLinearColor Color,float Width)
{
    DrawLine(OffsetX+X*Scale,OffsetY+Y*Scale,OffsetX+X2*Scale,OffsetY+Y2*Scale,Color,Width*Scale);
}

void ASpaceHUD::Label(const FString& Text,float X,float Y,float Size,FLinearColor Color,bool Center)
{
    FSlateFontInfo Font(TextFont,FMath::Max(8,FMath::RoundToInt(Size*14.f*Scale)),Size>=1.5f?FName(TEXT("Bold")):FName(TEXT("Regular")));
    FCanvasTextItem Item(FVector2D(OffsetX+X*Scale,OffsetY+Y*Scale),FText::FromString(Text),Font,Color);
    Item.bCentreX=Center;
    Canvas->DrawItem(Item);
}

void ASpaceHUD::Sprite(UTexture2D* Texture,float X,float Y,float W,float H,FLinearColor Tint)
{
    if (!Texture) return;
    DrawTexture(Texture,OffsetX+X*Scale,OffsetY+Y*Scale,W*Scale,H*Scale,0,0,1,1,Tint,BLEND_Translucent);
}

bool ASpaceHUD::Hover(float X,float Y,float W,float H) const
{
    float MX=0,MY=0;
    if (!GetOwningPlayerController()->GetMousePosition(MX,MY)) return false;
    MX=(MX-OffsetX)/Scale; MY=(MY-OffsetY)/Scale;
    return MX>=X && MX<=X+W && MY>=Y && MY<=Y+H;
}

void ASpaceHUD::Frame(float X,float Y,float W,float H,FLinearColor Color,bool Active)
{
    Panel(X,Y,W,H,FLinearColor(.008f,.018f,.033f,Active?.94f:.8f));
    Panel(X+1,Y+1,W-2,H*.38f,FLinearColor(.025f,.048f,.075f,.35f));
    const FLinearColor Edge(Color.R,Color.G,Color.B,Active?.8f:.25f);
    Line(X,Y,X+W-14,Y,Edge); Line(X+W-14,Y,X+W,Y+14,Edge);
    Line(X+W,Y+14,X+W,Y+H,Edge); Line(X+W,Y+H,X+14,Y+H,Edge);
    Line(X+14,Y+H,X,Y+H-14,Edge); Line(X,Y+H-14,X,Y,Edge);
    Line(X+12,Y+1,X+58,Y+1,Color,Active?3:1);
}

void ASpaceHUD::Button(FName Name,const FString& Text,float X,float Y,bool bPrimary)
{
    const bool Hot=Hover(X,Y,440,64);
    if(bPrimary) Panel(X,Y,440,64,Hot?FLinearColor(1.f,.88f,.57f):FLinearColor(.83f,.78f,.63f));
    Label(Text,X+220,Y+17,1.2f,bPrimary?FLinearColor(.009f,.012f,.02f):(Hot?FLinearColor::White:FLinearColor(.5f,.55f,.6f)),true);
    AddHitBox(FVector2D(OffsetX+X*Scale,OffsetY+Y*Scale),FVector2D(440*Scale,64*Scale),Name,true);
}

void ASpaceHUD::DrawHUD()
{
    Super::DrawHUD();
    if(!Canvas) return;
    auto* Mode=GetWorld()->GetAuthGameMode<ASpaceGameMode>();
    if(!Mode) return;
    Scale=FMath::Min(Canvas->SizeX/1600.f,Canvas->SizeY/900.f);
    OffsetX=(Canvas->SizeX-1600*Scale)*.5f; OffsetY=(Canvas->SizeY-900*Scale)*.5f;
    const FLinearColor White(.9f,.89f,.83f),Muted(.38f,.43f,.49f),Gold(.92f,.65f,.25f);
    const int32 Selected=FMath::Clamp(Mode->SelectedShip,0,2);
    UTexture2D* Portrait=ShipPortraits.IsValidIndex(Selected)?ShipPortraits[Selected].Get():nullptr;
    if(Mode->IsPlaying())
    {
        // Small, unframed readouts leave the playfield to the ship and asteroids.
        Label(FString::Printf(TEXT("%06d"),Mode->Score),52,28,2.05f,White);
        Label(TEXT("SCORE"),54,69,.55f,Muted);
        if(Mode->BonusSeconds(ESpaceBonus::DoubleScore)>0) Label(TEXT("×2"),245,39,1.2f,Gold);
        for(int32 i=0;i<FMath::Min(Mode->StartingLives,8);++i)
            Sprite(Portrait,1408+i*43,32,34,42,i<Mode->Lives?FLinearColor::White:FLinearColor(.14f,.14f,.14f,.5f));
        Label(FString::Printf(TEXT("%02d:%02d"),int32(Mode->SurvivalTime)/60,int32(Mode->SurvivalTime)%60),800,34,.82f,Muted,true);
        int32 Slot=0;
        const TCHAR* Names[]={TEXT("SCORE ×2"),TEXT("BOUCLIER"),TEXT("RÉPARATION"),TEXT("TRIPLE")};
        for(int32 i=0;i<4;++i)
        {
            const float Seconds=Mode->BonusSeconds(static_cast<ESpaceBonus>(i));
            if(Seconds<=0) continue;
            const float X=54+Slot++*172;
            if(BonusIcons.IsValidIndex(i)) Sprite(BonusIcons[i],X,804,35,35);
            Label(Names[i],X+44,806,.58f,White);
            Label(FString::Printf(TEXT("%d s"),FMath::CeilToInt(Seconds)),X+44,826,.6f,Gold);
        }
        for(TActorIterator<ASpaceAsteroid> It(GetWorld());It;++It)
        {
            auto* Rock=*It;
            if(!IsValid(Rock) || Rock->RemainingHits<=0) continue;
            FVector2D P;
            const float Radius=52.f*Rock->GetActorScale3D().X;
            if(!GetOwningPlayerController()->ProjectWorldLocationToScreen(Rock->GetActorLocation()+FVector(-Radius-12,0,0),P)) continue;
            const float X=(P.X-OffsetX)/Scale,Y=(P.Y-OffsetY)/Scale;
            if(X<24 || X>1576 || Y<96 || Y>798) continue;
            const int32 Total=FMath::Clamp(Rock->InitialHits,1,8);
            const float W=Total*12.f;
            Panel(X-W/2-2,Y-2,W+2,7,FLinearColor(0,0,0,.65f));
            for(int32 h=0;h<Total;++h) Panel(X-W/2+h*12,Y,10,3,h<Rock->RemainingHits?Gold:FLinearColor(.14f,.16f,.18f));
        }
        if(GetWorld()->GetTimeSeconds()<Mode->BonusMessageUntil) Label(Mode->BonusMessage,800,747,.85f,White,true);
        Label(TEXT("ÉCHAP  MENU"),1430,854,.55f,Muted);
        return;
    }
    Panel(0,0,1600,900,FLinearColor(.003f,.004f,.009f,.63f));
    if(Mode->State==ESpaceRunState::Menu)
    {
        if(TitleLogo)
        {
            const float T=GetWorld()->GetTimeSeconds();
            const float W=560.f*(1.f+.008f*FMath::Sin(T*.9f));
            const float H=W*TitleLogo->GetSizeY()/FMath::Max(1,TitleLogo->GetSizeX());
            const float X=800-W*.5f,Y=68+4.f*FMath::Sin(T*1.15f);
            DrawTexture(TitleLogo,OffsetX+X*Scale,OffsetY+Y*Scale,W*Scale,H*Scale,
                0,0,1,1,FLinearColor::White,BLEND_Translucent,1.f,false,.45f*FMath::Sin(T*.7f),FVector2D(.5f,.5f));
        }
        else Label(GameTitle,800,140,3.2f,White,true);
        Label(FString::Printf(TEXT("MEILLEUR SCORE   %06d"),Mode->BestScore),800,267,.72f,Muted,true);
        for(int32 i=0;i<3;++i)
        {
            const float X=477+i*230.f,Y=320;
            const bool Unlocked=Mode->IsShipUnlocked(i),Active=Selected==i;
            const bool Hot=Hover(X,Y,186,227);
            if(Hot && Unlocked) Panel(X,Y,186,227,FLinearColor(.09f,.1f,.12f,.25f));
            if(ShipPortraits.IsValidIndex(i)) Sprite(ShipPortraits[i],X+20,Y,146,146,Unlocked?FLinearColor::White:FLinearColor(.16f,.17f,.19f,.7f));
            Label(FleetUI::Names[i],X+93,Y+155,.9f,Unlocked?White:Muted,true);
            if(Active) Line(X+48,Y+188,X+139,Y+188,Gold,2);
            else if(!Unlocked) Label(FString::Printf(TEXT("%d PTS POUR DÉBLOQUER"),Mode->UnlockScores[i]),X+93,Y+193,.55f,Muted,true);
            if(Unlocked) AddHitBox(FVector2D(OffsetX+X*Scale,OffsetY+Y*Scale),FVector2D(186*Scale,227*Scale),FName(*FString::Printf(TEXT("Ship%d"),i)),true);
        }
        Button(TEXT("Start"),TEXT("JOUER"),580,598,true);
        Button(TEXT("Quit"),TEXT("QUITTER"),580,677,false);
        Label(TEXT("1 / 2 / 3  VAISSEAU     ENTRÉE  JOUER"),800,783,.58f,Muted,true);
    }
    else
    {
        Label(TEXT("PARTIE TERMINÉE"),800,185,2.7f,White,true);
        Label(FString::Printf(TEXT("%06d"),Mode->Score),800,293,3.5f,Gold,true);
        Label(FString::Printf(TEXT("RECORD  %06d     TEMPS  %02d:%02d"),Mode->BestScore,int32(Mode->SurvivalTime)/60,int32(Mode->SurvivalTime)%60),800,388,.8f,Muted,true);
        Button(TEXT("Start"),TEXT("REJOUER"),580,510,true);
        Button(TEXT("Menu"),TEXT("RETOUR AU MENU"),580,595,false);
    }
    Label(TEXT("ZQSD / WASD / FLÈCHES   DÉPLACER     ESPACE / CLIC   TIRER"),800,830,.55f,Muted,true);
    Label(Author,54,857,.53f,Muted);
}

void ASpaceHUD::NotifyHitBoxClick(FName BoxName)
{
    Super::NotifyHitBoxClick(BoxName);
    auto* Mode=GetWorld()->GetAuthGameMode<ASpaceGameMode>();
    if (!Mode || Mode->IsPlaying()) return;
    if (BoxName==TEXT("Start")) Mode->StartRun();
    else if(BoxName==TEXT("Menu")) Mode->ReturnToMenu();
    else if(BoxName==TEXT("Quit")) UKismetSystemLibrary::QuitGame(this,GetOwningPlayerController(),EQuitPreference::Quit,false);
    else for(int32 i=0;i<3;++i) if(BoxName==FName(*FString::Printf(TEXT("Ship%d"),i))) Mode->SelectShip(i);
}
