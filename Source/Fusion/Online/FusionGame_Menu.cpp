// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameSession.h"
#include "FusionPlayerController_Menu.h"
#include "FusionHUD.h"

#include "Player/Menu_Pawn.h"

#include "FusionGame_Menu.h"


AFusionGame_Menu::AFusionGame_Menu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
	PlayerControllerClass = AFusionPlayerController_Menu::StaticClass();


}

void AFusionGame_Menu::RestartPlayer(class AController* NewPlayer)
{
	// don't restart
	//Super::RestartPlayer(NewPlayer);


}



/** Returns game session class to use */
TSubclassOf<AGameSession> AFusionGame_Menu::GetGameSessionClass() const
{
	return AFusionGameSession::StaticClass();
}



/** Perform some final tasks before hosting/joining a session. Remove menus, set king state etc */
/*
void AFusionGame_Menu::BeginSession()
{

	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AFusionPlayerController_Menu* PC = Cast<AFusionPlayerController_Menu>(*It);
		if (PC)
		{
			AFusionHUD* PlayerHUD = Cast<AFusionPlayerController_Menu>(PC)->GetFusionHUD();

			if (PlayerHUD)
			{
				PlayerHUD->CreateGameWidgets();
				PlayerHUD->GetMainMenuUIWidget()->ShowMainMenu();
			}

		}
	}
	
}
*/