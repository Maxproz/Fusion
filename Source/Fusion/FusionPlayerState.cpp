// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameState.h"

#include "FusionCharacter.h"

#include "FusionPlayerState.h"


AFusionPlayerState::AFusionPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/*  Players are updated to the correct team through the GameMode::InitNewPlayer */
	TeamColor = ETeamColors::ETC_NONE;

	NumKills = 0;
	NumDeaths = 0;
	Score = 0;
}

void AFusionPlayerState::ClientInitialize(AController* InController)
{
	Super::ClientInitialize(InController);

	UpdateTeamColors();
}

void AFusionPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	/*
	AFusionPlayerState* FusionPlayer = Cast<AFusionPlayerState>(PlayerState);
	if (FusionPlayer)
	{
		FusionPlayer->TeamColor = TeamColor;
	}*/
	
}


void AFusionPlayerState::OnRep_TeamColor()
{
	UpdateTeamColors();
}

void AFusionPlayerState::Reset()
{
	Super::Reset();

	NumKills = 0;
	NumDeaths = 0;
	
	// Going to have the score == flag scores/bomb scores etc....
	Score = 0;

}

void AFusionPlayerState::AddKill()
{
	NumKills++;
}

void AFusionPlayerState::AddDeath()
{
	NumDeaths++;
}

void AFusionPlayerState::AddScore(int32 Amount)
{
	// Amount should never really exceed 1 for slayer/bomb/ctf type games.
	Score += Amount;

	/* Add the score to the global score count */
	AFusionGameState* GameState = GetWorld()->GetGameState<AFusionGameState>();
	if (GameState)
	{
		GameState->AddScore(Amount, TeamColor);
	}
}

void AFusionPlayerState::UpdateTeamColors()
{
	AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController != NULL)
	{
		AFusionCharacter* FusionCharacter = Cast<AFusionCharacter>(OwnerController->GetCharacter());
		if (FusionCharacter != NULL)
		{
			//FusionCharacter->UpdateTeamColorsAllMIDs();
			FusionCharacter->Update3rdPersonMeshColor();
		}
	}
}


void AFusionPlayerState::SetTeamColor(ETeamColors NewTeamColor)
{
	TeamColor = NewTeamColor;

	UpdateTeamColors();
}


ETeamColors AFusionPlayerState::GetTeamColor() const
{
	return TeamColor;
}

int32 AFusionPlayerState::GetKills() const
{
	return NumKills;
}

int32 AFusionPlayerState::GetDeaths() const
{
	return NumDeaths;
}

float AFusionPlayerState::GetScore() const
{
	return Score;
}

/*
int32 AFusionPlayerState::GetAssists() const
{
	return NumAssists;
}
*/


void AFusionPlayerState::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFusionPlayerState, NumKills);
	DOREPLIFETIME(AFusionPlayerState, NumDeaths);
	DOREPLIFETIME(AFusionPlayerState, TeamColor);
	//DOREPLIFETIME(AFusionPlayerState, NumAssists);
}

