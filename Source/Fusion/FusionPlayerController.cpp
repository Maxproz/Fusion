// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameState.h"
#include "FusionCharacter.h"
#include "FusionPlayerState.h"
#include "FusionGameInstance.h"
#include "FusionHUD.h"

#include "Player/FusionPlayerCameraManager.h"

#include "FusionPlayerController.h"


/* Define a log category for error messages */
DEFINE_LOG_CATEGORY_STATIC(LogGame, Log, All);


AFusionPlayerController::AFusionPlayerController(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* Assign the class types we wish to use */
	//PlayerCameraManagerClass = AFusionPlayerCameraManager::StaticClass();
	
	
	/* Example - Can be set to true for debugging, generally a value like this would exist in the GameMode instead */
	bRespawnImmediately = true;

}


void AFusionPlayerController::UnFreeze()
{
	Super::UnFreeze();
	
	ServerRestartPlayer();
	
	/*
	// Check if match is ending or has ended.
	AFusionGameState* MyGameState = GetWorld()->GetGameState<AFusionGameState>();
	if (MyGameState && MyGameState->HasMatchEnded())
	{
		// Don't allow spectating or respawns 
		return;
	}

	// Respawn or spectate 
	if (bRespawnImmediately)
	{
		ServerRestartPlayer();
	}
	else
	{
		StartSpectating();
	}

	*/
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


void AFusionPlayerController::ClientGameStarted_Implementation()
{
	bAllowGameActions = true;

	// Enable controls mode now the game has started
	SetIgnoreMoveInput(false);

	AFusionHUD* FusionHUD = GetFusionHUD();
	if (FusionHUD)
	{
		FusionHUD->SetMatchState(EHUDState::Playing);
	//	ShooterHUD->ShowScoreboard(false);
	}
	bGameEndedFrame = false;

	//QueryAchievements();

	// Send round start event
	const auto Events = Online::GetEventsInterface();
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	if (LocalPlayer != nullptr && Events.IsValid())
	{
		auto UniqueId = LocalPlayer->GetPreferredUniqueNetId();

		if (UniqueId.IsValid())
		{
			// Generate a new session id
			Events->SetPlayerSessionId(*UniqueId, FGuid::NewGuid());

			FString MapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());

			// Fire session start event for all cases
			FOnlineEventParms Params;
			Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
			Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("MapName"), FVariantData(MapName));

			Events->TriggerEvent(*UniqueId, TEXT("PlayerSessionStart"), Params);

			// Online matches require the MultiplayerRoundStart event as well
			UFusionGameInstance* FGI = GetWorld() != NULL ? Cast<UFusionGameInstance>(GetWorld()->GetGameInstance()) : NULL;

			if (FGI->GetIsOnline())
			{
				FOnlineEventParms MultiplayerParams;

				// @todo: fill in with real values
				MultiplayerParams.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
				MultiplayerParams.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
				MultiplayerParams.Add(TEXT("MatchTypeId"), FVariantData((int32)1)); // @todo abstract the specific meaning of this value across platforms
				MultiplayerParams.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

				Events->TriggerEvent(*UniqueId, TEXT("MultiplayerRoundStart"), MultiplayerParams);
			}

			bHasSentStartEvents = true;
		}
	}
}

/** Starts the online game using the session name in the PlayerState */
void AFusionPlayerController::ClientStartOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	AFusionPlayerState* FusionPlayerState = Cast<AFusionPlayerState>(PlayerState);
	if (FusionPlayerState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				UE_LOG(LogOnline, Log, TEXT("Starting session %s on client"), *FusionPlayerState->SessionName.ToString());
				Sessions->StartSession(FusionPlayerState->SessionName);
			}
		}
	}
	else
	{
		// Keep retrying until player state is replicated
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ClientStartOnlineGame, this, &AFusionPlayerController::ClientStartOnlineGame_Implementation, 0.2f, false);
	}
}

/** Ends the online game using the session name in the PlayerState */
void AFusionPlayerController::ClientEndOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	AFusionPlayerState* FusionPlayerState = Cast<AFusionPlayerState>(PlayerState);
	if (FusionPlayerState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				UE_LOG(LogOnline, Log, TEXT("Ending session %s on client"), *FusionPlayerState->SessionName.ToString());
				Sessions->EndSession(FusionPlayerState->SessionName);
			}
		}
	}
}

void AFusionPlayerController::ClientSendRoundEndEvent_Implementation(bool bIsWinner, int32 ExpendedTimeInSeconds)
{
	const auto Events = Online::GetEventsInterface();
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	if (bHasSentStartEvents && LocalPlayer != nullptr && Events.IsValid())
	{
		auto UniqueId = LocalPlayer->GetPreferredUniqueNetId();

		if (UniqueId.IsValid())
		{
			FString MapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());
			AFusionPlayerState* FusionPlayerState = Cast<AFusionPlayerState>(PlayerState);
			int32 PlayerScore = FusionPlayerState ? FusionPlayerState->GetScore() : 0;

			// Fire session end event for all cases
			FOnlineEventParms Params;
			Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
			Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("ExitStatusId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("PlayerScore"), FVariantData((int32)PlayerScore));
			Params.Add(TEXT("PlayerWon"), FVariantData((bool)bIsWinner));
			Params.Add(TEXT("MapName"), FVariantData(MapName));
			Params.Add(TEXT("MapNameString"), FVariantData(MapName)); // @todo workaround for a bug in backend service, remove when fixed

			Events->TriggerEvent(*UniqueId, TEXT("PlayerSessionEnd"), Params);

			// Online matches require the MultiplayerRoundEnd event as well
			UFusionGameInstance* FGI = GetWorld() != NULL ? Cast<UFusionGameInstance>(GetWorld()->GetGameInstance()) : NULL;
			if (FGI->GetIsOnline())
			{
				FOnlineEventParms MultiplayerParams;

				AFusionGameState* const MyGameState = GetWorld() != NULL ? GetWorld()->GetGameState<AFusionGameState>() : NULL;
				if (ensure(MyGameState != nullptr))
				{
					MultiplayerParams.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
					MultiplayerParams.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
					MultiplayerParams.Add(TEXT("MatchTypeId"), FVariantData((int32)1)); // @todo abstract the specific meaning of this value across platforms
					MultiplayerParams.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
					MultiplayerParams.Add(TEXT("TimeInSeconds"), FVariantData((float)ExpendedTimeInSeconds));
					MultiplayerParams.Add(TEXT("ExitStatusId"), FVariantData((int32)0)); // unused

					Events->TriggerEvent(*UniqueId, TEXT("MultiplayerRoundEnd"), MultiplayerParams);
				}
			}
		}

		bHasSentStartEvents = false;
	}
}


bool AFusionPlayerController::IsGameInputAllowed() const
{
	return bAllowGameActions && !bCinematicMode;
}

AFusionHUD* AFusionPlayerController::GetFusionHUD() const
{
	return Cast<AFusionHUD>(GetHUD());
}

void AFusionPlayerController::HandleReturnToMainMenu()
{
	//OnHideScoreboard();
	CleanupSessionOnReturnToMenu();
}


void AFusionPlayerController::ShowInGameMenu()
{
	AFusionHUD* FusionHUD = GetFusionHUD();
	
	
	/* TODO: Implement Functionality for showing in-game menu UUserWidget
	if (ShooterIngameMenu.IsValid() && !ShooterIngameMenu->GetIsGameMenuUp() && ShooterHUD && (ShooterHUD->IsMatchOver() == false))
	{
		ShooterIngameMenu->ToggleGameMenu();
	}
	*/
}

/** Ends and/or destroys game session */
void AFusionPlayerController::CleanupSessionOnReturnToMenu()
{
	UFusionGameInstance * FGI = GetWorld() != NULL ? Cast<UFusionGameInstance>(GetWorld()->GetGameInstance()) : NULL;

	if (ensure(FGI != NULL))
	{
		FGI->CleanupSessionOnReturnToMenu();
	}
}

bool AFusionPlayerController::IsGameMenuVisible() const
{
	bool Result = false;

	/* TODO: Needs implemented
	if (ShooterIngameMenu.IsValid())
	{
		Result = ShooterIngameMenu->GetIsGameMenuUp();
	}*/

	return Result;
}



