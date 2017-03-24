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

