// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameState.h"
#include "FusionCharacter.h"

#include "Player/FusionPlayerCameraManager.h"

#include "FusionPlayerController.h"


/* Define a log category for error messages */
DEFINE_LOG_CATEGORY_STATIC(LogGame, Log, All);


AFusionPlayerController::AFusionPlayerController(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* Assign the class types we wish to use */
	PlayerCameraManagerClass = AFusionPlayerCameraManager::StaticClass();
	/* Example - Can be set to true for debugging, generally a value like this would exist in the GameMode instead */
	bRespawnImmediately = false;

}


void AFusionPlayerController::UnFreeze()
{
	Super::UnFreeze();

	// Check if match is ending or has ended.
	AFusionGameState* MyGameState = GetWorld()->GetGameState<AFusionGameState>();
	if (MyGameState && MyGameState->HasMatchEnded())
	{
		/* Don't allow spectating or respawns */
		return;
	}

	/* Respawn or spectate */
	if (bRespawnImmediately)
	{
		ServerRestartPlayer();
	}
	else
	{
		StartSpectating();
	}
}


void AFusionPlayerController::StartSpectating()
{
	/* Update the state on server */
	PlayerState->bIsSpectator = true;
	/* Waiting to respawn */
	bPlayerIsWaiting = true;
	ChangeState(NAME_Spectating);
	/* Push the state update to the client */
	ClientGotoState(NAME_Spectating);

	/* Focus on the remaining alive player */
	ViewAPlayer(1);

	/* Update the HUD to show the spectator screen */
	ClientHUDStateChanged(EHUDState::Spectating);
}


void AFusionPlayerController::Suicide()
{
	if (IsInState(NAME_Playing))
	{
		ServerSuicide();
	}
}

void AFusionPlayerController::ServerSuicide_Implementation()
{
	AFusionCharacter* MyPawn = Cast<AFusionCharacter>(GetPawn());
	if (MyPawn && ((GetWorld()->TimeSeconds - MyPawn->CreationTime > 1) || (GetNetMode() == NM_Standalone)))
	{
		MyPawn->Suicide();
	}

}


bool AFusionPlayerController::ServerSuicide_Validate()
{
	return true;
}


void AFusionPlayerController::ClientHUDStateChanged_Implementation(EHUDState NewState)
{
	AFusionHUD* HUD = Cast<AFusionHUD>(GetHUD());
	if (HUD)
	{
		HUD->OnStateChanged(NewState);
	}
}


void AFusionPlayerController::ClientHUDMessage_Implementation(EHUDMessage MessageID)
{
	/* Turn the ID into a message for the HUD to display */
	FText TextMessage = GetText(MessageID);

	AFusionHUD* HUD = Cast<AFusionHUD>(GetHUD());
	if (HUD)
	{
		/* Implemented in SurvivalHUD Blueprint */
		HUD->MessageReceived(TextMessage);
	}
}

/* Temporarily set the namespace. If this was omitted, we should call NSLOCTEXT(Namespace, x, y) instead */
#define LOCTEXT_NAMESPACE "HUDMESSAGES"

FText AFusionPlayerController::GetText(EHUDMessage MsgID)
{
	switch (MsgID)
	{
	case EHUDMessage::Weapon_Picked_Up:
		return LOCTEXT("Weapon_Picked_Up", "We already have that weapon.");
	case EHUDMessage::Character_Shields_Recharged:
		return LOCTEXT("Character_Shields_Recharged", "Shields Recharging");
	case EHUDMessage::Game_CTF:
		return LOCTEXT("Game_CTF", "Capture the Flag");
	case EHUDMessage::Game_Slayer:
		return LOCTEXT("Game_Slayer", "Kill the enemy team.");
	case EHUDMessage::Game_FFA:
		return LOCTEXT("Game_FFA", "Kill enemy players.");
	case EHUDMessage::Game_Assault:
		return LOCTEXT("Game_Assault", "Plant the bomb in the enemy base.");
	default:
		UE_LOG(LogGame, Warning, TEXT("No Message set for enum value in SPlayerContoller::GetText(). "))
			return FText::FromString("No Message Set");
	}
}

/* Remove the namespace definition so it doesn't exist in other files compiled after this one. */
#undef LOCTEXT_NAMESPACE

