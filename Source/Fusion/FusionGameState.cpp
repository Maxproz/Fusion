// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameMode.h"
#include "FusionGameInstance.h"
#include "FusionPlayerController.h"

#include "FusionGameState.h"

AFusionGameState::AFusionGameState(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NumTeams = 0;
}

void AFusionGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFusionGameState, ElapsedGameMinutes);
	DOREPLIFETIME(AFusionGameState, RedTeamScore);
	DOREPLIFETIME(AFusionGameState, BlueTeamScore);
	DOREPLIFETIME(AFusionGameState, NumTeams);

}

int32 AFusionGameState::GetRedTeamScore()
{
	return RedTeamScore;
}

int32 AFusionGameState::GetBlueTeamScore()
{
	return BlueTeamScore;
}

void AFusionGameState::AddScore(int32 Score, ETeamColors TeamColor)
{

	switch (TeamColor)
	{
		case ETeamColors::ETC_RED:
		{
			RedTeamScore += Score;
			break;
		}
		case ETeamColors::ETC_BLUE:
		{
			BlueTeamScore += Score;
			break;
		}
		default:
			UE_LOG(LogTemp, Warning, TEXT("No TeamColor set for enum value in AFusionGameState::AddScore()."))
		
	}
}

/* As with Server side functions, NetMulticast functions have a _Implementation body */
void AFusionGameState::BroadcastGameMessage_Implementation(EHUDMessage MessageID)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		AFusionPlayerController* MyController = Cast<AFusionPlayerController>(*It);
		if (MyController && MyController->IsLocalController())
		{
			MyController->ClientHUDMessage(MessageID);
		}
	}
}

void AFusionGameState::RequestFinishAndExitToMainMenu()
{
	if (AuthorityGameMode)
	{
		// we are server, tell the gamemode
		AFusionGameMode* const GameMode = Cast<AFusionGameMode>(AuthorityGameMode);
		if (GameMode)
		{
			GameMode->RequestFinishAndExitToMainMenu();
		}
	}
	else
	{
		// we are client, handle our own business
		UFusionGameInstance* GameInstance = Cast<UFusionGameInstance>(GetGameInstance());
		if (GameInstance)
		{
			GameInstance->RemoveSplitScreenPlayers();
		}

		AFusionPlayerController* const PrimaryPC = Cast<AFusionPlayerController>(GetGameInstance()->GetFirstLocalPlayerController());
		if (PrimaryPC)
		{
			check(PrimaryPC->GetNetMode() == ENetMode::NM_Client);
			PrimaryPC->HandleReturnToMainMenu();
		}
	}

}
