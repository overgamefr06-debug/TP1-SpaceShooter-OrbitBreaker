#include "SpaceHUD.h"
#include "SpaceGameMode.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Engine/Font.h"
#include "Fonts/SlateFontInfo.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

ASpaceHUD::ASpaceHUD()
{
    static ConstructorHelpers::FObjectFinder<UFont> Font(TEXT("/Engine/EngineFonts/Roboto.Roboto"));
    TextFont=Font.Object;
}

void ASpaceHUD::Panel(float X, float Y, float W, float H, FLinearColor Color)
{
    DrawRect(Color, OffsetX + X*Scale, OffsetY + Y*Scale, W*Scale, H*Scale);
}

void ASpaceHUD::Label(const FString& Text, float X, float Y, float Size, FLinearColor Color)
{
    FSlateFontInfo Font(TextFont, FMath::Max(8,FMath::RoundToInt(Size*14.f*Scale)),Size>=1.5f?FName(TEXT("Bold")):FName(TEXT("Regular")));
    FCanvasTextItem Item(FVector2D(OffsetX+X*Scale,OffsetY+Y*Scale),FText::FromString(Text),Font,Color);
    Canvas->DrawItem(Item);
}

void ASpaceHUD::Button(FName Name, const FString& Text, float X, float Y, bool bPrimary)
{
    float MX=0, MY=0;
    GetOwningPlayerController()->GetMousePosition(MX, MY);
    MX=(MX-OffsetX)/Scale; MY=(MY-OffsetY)/Scale;
    const bool Hover=MX>=X && MX<=X+390 && MY>=Y && MY<=Y+62;
    Panel(X,Y,390,62,bPrimary ? (Hover ? FLinearColor(.5f,.95f,1) : Accent) : FLinearColor(.08f,.13f,.2f,Hover ? 1.f : .85f));
    Label(Text,X+24,Y+17,1.12f,bPrimary ? FLinearColor(.01f,.035f,.065f) : FLinearColor(.8f,.87f,.95f));
    Label(TEXT(">"),X+354,Y+16,1.2f,bPrimary ? FLinearColor(.01f,.035f,.065f) : Accent);
    AddHitBox(FVector2D(OffsetX+X*Scale,OffsetY+Y*Scale),FVector2D(390*Scale,62*Scale),Name,true);
}

void ASpaceHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;
    auto* Mode=GetWorld()->GetAuthGameMode<ASpaceGameMode>();
    if (!Mode) return;
    Scale=FMath::Min(Canvas->SizeX/1600.f,Canvas->SizeY/900.f);
    OffsetX=(Canvas->SizeX-1600*Scale)*.5f;
    OffsetY=(Canvas->SizeY-900*Scale)*.5f;
    const FLinearColor White(.91f,.95f,1.f), Muted(.4f,.55f,.68f), Dark(.002f,.006f,.014f,.94f);
    if (Mode->IsPlaying())
    {
        Panel(40,28,260,78,Dark); Panel(40,28,3,78,Accent);
        Label(TEXT("SCORE"),58,40,.7f,Muted);
        Label(FString::Printf(TEXT("%06d"),Mode->Score),58,62,1.55f,White);
        Panel(1290,28,270,78,Dark);
        Label(TEXT("COQUE"),1308,39,.7f,Muted);
        for (int32 i=0;i<FMath::Min(Mode->StartingLives,8);++i)
            Panel(1308+i*72.f/FMath::Max(1,Mode->StartingLives)*3,72,54.f*3/FMath::Max(1,Mode->StartingLives),12,i<Mode->Lives?Accent:FLinearColor(.12f,.17f,.23f));
        Label(TEXT("ORBIT / 07"),698,30,.82f,Muted);
        Label(FString::Printf(TEXT("%02d:%02d"),int32(Mode->SurvivalTime)/60,int32(Mode->SurvivalTime)%60),738,54,1.25f,White);
        Label(TEXT("ZQSD / WASD / FLÈCHES   DÉPLACER     ESPACE / CLIC   TIRER"),42,855,.62f,Muted);
        Label(TEXT("R  RECOMMENCER     ÉCHAP  MENU"),1210,855,.62f,Muted);
        if (Mode->IsInvulnerable()) Label(TEXT("BOUCLIER DE RÉCUPÉRATION"),651,815,.75f,Accent);
        return;
    }
    Panel(0,0,1600,900,FLinearColor(.005f,.012f,.028f,.35f));
    Panel(42,72,660,756,Dark);
    Panel(42,72,4,756,Accent);
    Label(TEXT("SECTEUR 07  /  CEINTURE D'ASTÉROÏDES"),86,114,.7f,Accent);
    if (Mode->State==ESpaceRunState::Menu)
    {
        Label(GameTitle,82,183,2.45f,White);
        Label(TEXT("TRAVERSEZ LE CHAOS."),88,270,1.04f,Warm);
        Label(TEXT("Pilotez. Esquivez. Brisez les astéroïdes."),88,333,.95f,White);
        Label(TEXT("Chaque cible encaisse de 1 à 3 impacts."),88,374,.78f,Muted);
        Label(TEXT("Trois vies. Jusqu’où tiendrez-vous ?"),88,407,.78f,Muted);
        Button(TEXT("Start"),TEXT("LANCER LA MISSION"),88,481,true);
        Button(TEXT("Quit"),TEXT("QUITTER"),88,561,false);
        Label(TEXT("ENTRÉE  LANCER     ZQSD / WASD  DÉPLACER"),88,671,.63f,Muted);
        Label(TEXT("ESPACE / CLIC  TIRER     ÉCHAP  MENU"),88,703,.63f,Muted);
        Label(TEXT("TP1  /  Unreal Engine     |     ")+Author,88,776,.62f,Muted);
        Label(TEXT("INTERCEPTEUR  /  K-07"),1020,715,.75f,Accent);
        Label(TEXT("SYSTÈMES EN LIGNE"),1047,748,.58f,Muted);
    }
    else
    {
        Label(TEXT("MISSION TERMINÉE"),84,192,2.1f,White);
        Label(TEXT("SCORE FINAL"),88,296,.8f,Muted);
        Label(FString::Printf(TEXT("%06d"),Mode->Score),86,331,3.1f,Accent);
        Label(FString::Printf(TEXT("Durée de survie : %d s"),int32(Mode->SurvivalTime)),88,422,.88f,Muted);
        Button(TEXT("Start"),TEXT("REJOUER"),88,497,true);
        Button(TEXT("Menu"),TEXT("RETOUR AU MENU"),88,577,false);
        Label(TEXT("R / ENTRÉE  REJOUER"),88,683,.7f,Muted);
        Label(TEXT("TP1  /  ")+Author,88,776,.62f,Muted);
    }
}

void ASpaceHUD::NotifyHitBoxClick(FName BoxName)
{
    Super::NotifyHitBoxClick(BoxName);
    auto* Mode=GetWorld()->GetAuthGameMode<ASpaceGameMode>();
    if (!Mode || Mode->IsPlaying()) return;
    if (BoxName==TEXT("Start")) Mode->StartRun();
    else if (BoxName==TEXT("Menu")) Mode->ReturnToMenu();
    else if (BoxName==TEXT("Quit")) UKismetSystemLibrary::QuitGame(this,GetOwningPlayerController(),EQuitPreference::Quit,false);
}
