// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameState.h"
#include "FusionPlayerController.h"

#include "FusionCharacter.h"

#include "FusionPlayerState.h"


AFusionPlayerState::AFusionPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/*  Players are updated to the correct team through the GameMode::InitNewPlayer */
	TeamNumber = 0;
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
	bQuitter = false;
	Score = 0;
}


void AFusionPlayerState::ClientInitialize(AController* InController)
{
	Super::ClientInitialize(InController);

	UpdateTeamColors();

}

void AFusionPlayerState::UnregisterPlayerWithSession()
{
	if (!bFromPreviousLevel)
	{
		Super::UnregisterPlayerWithSession();
	}
}

void AFusionPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	AFusionPlayerState* FusionPlayer = Cast<AFusionPlayerState>(PlayerState);
	if (FusionPlayer)
	{
		FusionPlayer->TeamNumber = TeamNumber;
	}
}

void AFusionPlayerState::OnRep_TeamColor()
{
	UpdateTeamColors();
}

void AFusionPlayerState::Reset()
{
	Super::Reset();

	//PlayerStates persist across seamless travel.  Keep the same teams as previous match.
	//SetTeamNum(0);
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
	bQuitter = false;
	
	// Going to have the score == flag scores/bomb scores etc....
	Score = 0;

}


void AFusionPlayerState::AddScore(int32 Amount)
{
	// Amount should never really exceed 1 for slayer/bomb/ctf type games.
	Score += Amount;

	/* Add the score to the global score count */
	AFusionGameState* GameState = GetWorld()->GetGameState<AFusionGameState>();
	if (GameState)
	{
		//GameState->AddScore(Amount, TeamColor);
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
			FusionCharacter->UpdateTeamColorsAllMIDs();

		}
	}
}


void AFusionPlayerState::SetTeamNum(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;

	UpdateTeamColors();
}

int32 AFusionPlayerState::GetTeamNum() const
{
	return TeamNumber;
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

int32 AFusionPlayerState::GetNumBulletsFired() const
{
	return NumBulletsFired;
}

int32 AFusionPlayerState::GetNumRocketsFired() const
{
	return NumRocketsFired;
}


void AFusionPlayerState::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFusionPlayerState, NumKills);
	DOREPLIFETIME(AFusionPlayerState, NumDeaths);
	DOREPLIFETIME(AFusionPlayerState, TeamNumber);
}

void AFusionPlayerState::ScoreKill(AFusionPlayerState* Victim, int32 Points)
{
	NumKills++;
	ScorePoints(Points);
}

void AFusionPlayerState::ScoreDeath(AFusionPlayerState* KilledBy, int32 Points)
{
	NumDeaths++;
	ScorePoints(Points);
}

void AFusionPlayerState::ScorePoints(int32 Points)
{
	AFusionGameState* const MyGameState = GetWorld()->GetGameState<AFusionGameState>();
	if (MyGameState && TeamNumber >= 0)
	{
		if (TeamNumber >= MyGameState->TeamScores.Num())
		{
			MyGameState->TeamScores.AddZeroed(TeamNumber - MyGameState->TeamScores.Num() + 1);
		}

		MyGameState->TeamScores[TeamNumber] += Points;
	}

	Score += Points;
}

void AFusionPlayerState::InformAboutKill_Implementation(class AFusionPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AFusionPlayerState* KilledPlayerState)
{
	//id can be null for bots
	if (KillerPlayerState->UniqueId.IsValid())
	{
		//search for the actual killer before calling OnKill()	
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AFusionPlayerController* TestPC = Cast<AFusionPlayerController>(*It);
			if (TestPC && TestPC->IsLocalController())
			{
				// a local player might not have an ID if it was created with CreateDebugPlayer.
				ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(TestPC->Player);
				TSharedPtr<const FUniqueNetId> LocalID = LocalPlayer->GetCachedUniqueNetId();
				if (LocalID.IsValid() && *LocalPlayer->GetCachedUniqueNetId() == *KillerPlayerState->UniqueId)
				{
					TestPC->OnKill();
				}
			}
		}
	}
}

void AFusionPlayerState::BroadcastDeath_Implementation(class AFusionPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AFusionPlayerState* KilledPlayerState)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		// all local players get death messages so they can update their huds.
		AFusionPlayerController* TestPC = Cast<AFusionPlayerController>(*It);
		if (TestPC && TestPC->IsLocalController())
		{
			TestPC->OnDeathMessage(KillerPlayerState, this, KillerDamageType);
		}
	}
}

void AFusionPlayerState::SetQuitter(bool bInQuitter)
{
	bQuitter = bInQuitter;
}

bool AFusionPlayerState::IsQuitter() const
{
	return bQuitter;
}


FString AFusionPlayerState::GetShortPlayerName() const
{
	if (PlayerName.Len() > MAX_PLAYER_NAME_LENGTH)
	{
		return PlayerName.Left(MAX_PLAYER_NAME_LENGTH) + "...";
	}
	return PlayerName;
}
