#include "SpaceHUD.h"
#include "SpaceGameMode.h"
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

void ASpaceHUD::Label(const FString& Text,float X,float Y,float Size,FLinearColor Color)
{
    FSlateFontInfo Font(TextFont,FMath::Max(8,FMath::RoundToInt(Size*14.f*Scale)),Size>=1.5f?FName(TEXT("Bold")):FName(TEXT("Regular")));
    FCanvasTextItem Item(FVector2D(OffsetX+X*Scale,OffsetY+Y*Scale),FText::FromString(Text),Font,Color);
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
    Frame(X,Y,440,64,bPrimary?Accent:FLinearColor(.35f,.45f,.58f),Hot||bPrimary);
    if (bPrimary) Panel(X+1,Y+1,438,62,FLinearColor(Accent.R,Accent.G,Accent.B,Hot?.27f:.12f));
    Label(Text,X+24,Y+19,1.03f,bPrimary?FLinearColor(.86f,.97f,1.f):FLinearColor(.56f,.66f,.76f));
    Line(X+396,Y+32,X+413,Y+32,Accent,2);
    Line(X+406,Y+25,X+413,Y+32,Accent,2); Line(X+406,Y+39,X+413,Y+32,Accent,2);
    AddHitBox(FVector2D(OffsetX+X*Scale,OffsetY+Y*Scale),FVector2D(440*Scale,64*Scale),Name,true);
}

void ASpaceHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;
    auto* Mode=GetWorld()->GetAuthGameMode<ASpaceGameMode>();
    if (!Mode) return;
    Scale=FMath::Min(Canvas->SizeX/1600.f,Canvas->SizeY/900.f);
    OffsetX=(Canvas->SizeX-1600*Scale)*.5f; OffsetY=(Canvas->SizeY-900*Scale)*.5f;
    const FLinearColor White(.9f,.94f,.98f),Muted(.43f,.55f,.66f);
    const int32 Selected=FMath::Clamp(Mode->SelectedShip,0,2);
    const FLinearColor ShipColor=FleetUI::Colors[Selected];
    UTexture2D* SelectedArt=ShipPortraits.IsValidIndex(Selected)?ShipPortraits[Selected].Get():nullptr;
    if (Mode->IsPlaying())
    {
        Frame(44,30,302,96,Accent,true);
        Sprite(ScoreEmblem,57,45,68,68);
        Label(TEXT("POINTS DE MISSION"),138,44,.64f,Muted);
        Label(FString::Printf(TEXT("%06d"),Mode->Score),135,64,2.05f,White);
        Frame(1218,30,338,96,ShipColor,true);
        Label(TEXT("INTÉGRITÉ  /  ")+FString(FleetUI::Names[Selected]),1238,42,.63f,Muted);
        const int32 Total=FMath::Clamp(Mode->StartingLives,1,8);
        for(int32 i=0;i<Total;++i)
        {
            const float X=1240+i*200.f/Total;
            const bool Alive=i<Mode->Lives;
            Sprite(SelectedArt,X,65,47,47,Alive?FLinearColor::White:FLinearColor(.13f,.16f,.2f,.45f));
            Line(X+7,115,X+40,115,Alive?ShipColor:FLinearColor(.12f,.17f,.23f),2);
        }
        Label(FString::Printf(TEXT("%02d"),Mode->Lives),1460,62,2.f,Mode->Lives==1?Warm:White);
        Label(TEXT("SECTEUR 07"),735,36,.65f,Muted);
        Label(FString::Printf(TEXT("%02d : %02d"),int32(Mode->SurvivalTime)/60,int32(Mode->SurvivalTime)%60),727,59,1.4f,White);
        Line(699,96,889,96,FLinearColor(.15f,.25f,.35f,.65f));
        Label(TEXT("ZQSD / WASD / FLÈCHES   DÉPLACER     ESPACE / CLIC   TIRER"),48,858,.59f,Muted);
        Label(TEXT("R  REJOUER    ÉCHAP  MENU"),1270,858,.59f,Muted);
        if (Mode->IsInvulnerable()) Label(TEXT("BOUCLIER ACTIF"),726,807,.71f,ShipColor);
        return;
    }
    for(int32 i=0;i<80;++i)
        Panel(i*20,0,20,900,FLinearColor(.004f,.009f,.02f,.88f-.56f*i/79.f));
    Line(72,99,1528,99,FLinearColor(.22f,.37f,.49f,.5f));
    Sprite(ScoreEmblem,70,35,42,42);
    Label(TEXT("ORBITAL COMMAND"),127,46,.7f,White);
    Label(TEXT("SECTEUR 07   /   CEINTURE D'ASTÉROÏDES"),1114,48,.63f,Muted);
    if (Mode->State==ESpaceRunState::Menu)
    {
        Label(TEXT("01  /  PRÉPARATION DU VOL"),82,143,.7f,Accent);
        Label(TEXT("ORBIT"),76,189,4.75f,White);
        Label(TEXT("BREAKER"),76,270,4.75f,White);
        Line(84,378,142,378,Accent,3);
        Label(TEXT("Votre vaisseau. Votre trajectoire."),82,414,1.12f,White);
        Label(TEXT("Traversez la ceinture. Faites grimper le score."),82,452,.81f,Muted);
        Label(TEXT("Trois vies pour repousser vos limites."),82,482,.81f,Muted);
        Button(TEXT("Start"),TEXT("LANCER LA MISSION"),82,574,true);
        Button(TEXT("Quit"),TEXT("QUITTER"),82,654,false);
        Label(TEXT("ENTRÉE  LANCER     1 / 2 / 3  CHOISIR"),85,751,.61f,Muted);
        Label(TEXT("ESPACE  TIRER     ZQSD / WASD  DÉPLACER"),85,777,.61f,Muted);
        const float Bob=4.f*FMath::Sin(GetWorld()->GetTimeSeconds()*1.3f);
        const float CX=1090,CY=348;
        for(int32 Ring=0;Ring<2;++Ring)
            for(int32 Segment=0;Segment<72;++Segment)
            {
                if (Segment%18>13) continue;
                const float A=Segment*2*PI/72, B=(Segment+1)*2*PI/72, R=204.f+Ring*29.f;
                Line(CX+FMath::Cos(A)*R,CY+FMath::Sin(A)*R*.71f,CX+FMath::Cos(B)*R,CY+FMath::Sin(B)*R*.71f,FLinearColor(ShipColor.R,ShipColor.G,ShipColor.B,Ring?.12f:.25f));
            }
        Sprite(SelectedArt,880,136+Bob,420,420);
        Label(FleetUI::Names[Selected],655,162,1.35f,ShipColor);
        Label(FleetUI::Roles[Selected],656,196,.59f,Muted);
        Line(658,229,807,229,FLinearColor(ShipColor.R,ShipColor.G,ShipColor.B,.35f));
        Label(TEXT("ARMEMENT"),1353,281,.57f,Muted);
        Label(TEXT("LASER"),1353,305,.87f,White);
        Label(TEXT("RÉSERVE"),1353,348,.57f,Muted);
        Label(TEXT("03 VIES"),1353,372,.87f,White);
        Label(TEXT("CHOISISSEZ VOTRE VAISSEAU"),654,552,.68f,White);
        for(int32 i=0;i<3;++i)
        {
            const float X=654+i*294.f,Y=588.f,W=276,H=191;
            const bool Active=Selected==i,Hot=Hover(X,Y,W,H);
            Frame(X,Y,W,H,FleetUI::Colors[i],Active||Hot);
            if(Active) Panel(X+1,Y+H-4,W-2,3,FleetUI::Colors[i]);
            Label(FString::Printf(TEXT("0%d"),i+1),X+17,Y+15,.67f,Active?FleetUI::Colors[i]:Muted);
            if(ShipPortraits.IsValidIndex(i)) Sprite(ShipPortraits[i],X+77,Y+10,121,121,Active||Hot?FLinearColor::White:FLinearColor(.68f,.68f,.72f,1));
            Label(FleetUI::Names[i],X+18,Y+132,.96f,White);
            Label(Active?TEXT("SÉLECTIONNÉ"):TEXT("SÉLECTIONNER"),X+18,Y+160,.57f,Active?FleetUI::Colors[i]:Muted);
            AddHitBox(FVector2D(OffsetX+X*Scale,OffsetY+Y*Scale),FVector2D(W*Scale,H*Scale),FName(*FString::Printf(TEXT("Ship%d"),i)),true);
        }
    }
    else
    {
        Sprite(ScoreEmblem,1030,190,220,220);
        Label(TEXT("02  /  RAPPORT DE VOL"),82,163,.7f,Warm);
        Label(TEXT("MISSION"),77,213,3.9f,White);
        Label(TEXT("TERMINÉE"),77,283,3.9f,White);
        Label(TEXT("POINTS DE MISSION"),85,396,.7f,Muted);
        Label(FString::Printf(TEXT("%06d"),Mode->Score),79,435,3.65f,Accent);
        Label(FString::Printf(TEXT("TEMPS DE SURVIE   %02d : %02d"),int32(Mode->SurvivalTime)/60,int32(Mode->SurvivalTime)%60),85,525,.8f,Muted);
        Button(TEXT("Start"),TEXT("REPARTIR EN MISSION"),82,598,true);
        Button(TEXT("Menu"),TEXT("CHOISIR UN VAISSEAU"),82,678,false);
        Sprite(SelectedArt,960,458,355,355);
        Label(FleetUI::Names[Selected],1086,423,1.1f,ShipColor);
    }
    Line(72,827,1528,827,FLinearColor(.22f,.37f,.49f,.5f));
    Label(TEXT("TP1  /  ")+Author,82,851,.62f,Muted);
    Label(TEXT("PETIT  100 PTS / 1 TIR      MOYEN  200 PTS / 2 TIRS      GRAND  400 PTS / 3 TIRS"),730,851,.59f,Muted);
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
