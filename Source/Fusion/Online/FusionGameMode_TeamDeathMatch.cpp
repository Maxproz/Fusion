// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionPlayerState.h"
#include "FusionGameState.h"

#include "Player/FusionPlayerStart.h"

#include "FusionGameMode_TeamDeathMatch.h"



AFusionGameMode_TeamDeathMatch::AFusionGameMode_TeamDeathMatch(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NumTeams = 2;
	bDelayedStart = true;
}

void AFusionGameMode_TeamDeathMatch::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, *FString::Printf(TEXT("Current GameMode MatchState: %s"), *GetMatchState().ToString()));
}

void AFusionGameMode_TeamDeathMatch::PostLogin(APlayerController* NewPlayer)
{
	// Place player on a team before Super (VoIP team based init, findplayerstart, etc)
	AFusionPlayerState* NewPlayerState = CastChecked<AFusionPlayerState>(NewPlayer->PlayerState);
	const int32 TeamNum = ChooseTeam(NewPlayerState);
	NewPlayerState->SetTeamNum(TeamNum);

	Super::PostLogin(NewPlayer);
}

void AFusionGameMode_TeamDeathMatch::InitGameState()
{
	Super::InitGameState();

	AFusionGameState* const MyGameState = Cast<AFusionGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->NumTeams = NumTeams;
	}
}

bool AFusionGameMode_TeamDeathMatch::CanDealDamage(AFusionPlayerState* DamageInstigator, class AFusionPlayerState* DamagedPlayer) const
{
	if (bAllowFriendlyFireDamage)
	{
		return true;
	}

	/* Allow damage to self */
	if (bAllowSelfDamage)
	{
		return true;
	}

	// Compare Team Colors
	return DamageInstigator && DamagedPlayer && (DamageInstigator->GetTeamNum() != DamagedPlayer->GetTeamNum());
}

int32 AFusionGameMode_TeamDeathMatch::ChooseTeam(AFusionPlayerState* ForPlayerState) const
{
	TArray<int32> TeamBalance;
	TeamBalance.AddZeroed(NumTeams);

	// get current team balance
	for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
	{
		AFusionPlayerState const* const TestPlayerState = Cast<AFusionPlayerState>(GameState->PlayerArray[i]);
		if (TestPlayerState && TestPlayerState != ForPlayerState && TeamBalance.IsValidIndex(TestPlayerState->GetTeamNum()))
		{
			TeamBalance[TestPlayerState->GetTeamNum()]++;
		}
	}

	// find least populated one
	int32 BestTeamScore = TeamBalance[0];
	for (int32 i = 1; i < TeamBalance.Num(); i++)
	{
		if (BestTeamScore > TeamBalance[i])
		{
			BestTeamScore = TeamBalance[i];
		}
	}

	// there could be more than one...
	TArray<int32> BestTeams;
	for (int32 i = 0; i < TeamBalance.Num(); i++)
	{
		if (TeamBalance[i] == BestTeamScore)
		{
			BestTeams.Add(i);
		}
	}

	// get random from best list
	const int32 RandomBestTeam = BestTeams[FMath::RandHelper(BestTeams.Num())];
	return RandomBestTeam;
}

void AFusionGameMode_TeamDeathMatch::DetermineMatchWinner()
{
	AFusionGameState const* const MyGameState = Cast<AFusionGameState>(GameState);
	int32 BestScore = MAX_uint32;
	int32 BestTeam = -1;
	int32 NumBestTeams = 1;

	for (int32 i = 0; i < MyGameState->TeamScores.Num(); i++)
	{
		const int32 TeamScore = MyGameState->TeamScores[i];
		if (BestScore < TeamScore)
		{
			BestScore = TeamScore;
			BestTeam = i;
			NumBestTeams = 1;
		}
		else if (BestScore == TeamScore)
		{
			NumBestTeams++;
		}
	}

	WinnerTeam = (NumBestTeams == 1) ? BestTeam : NumTeams;
}

bool AFusionGameMode_TeamDeathMatch::IsWinner(AFusionPlayerState* PlayerState) const
{
	return PlayerState && !PlayerState->IsQuitter() && PlayerState->GetTeamNum() == WinnerTeam;
}

bool AFusionGameMode_TeamDeathMatch::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	if (Player)
	{
		AFusionPlayerStart* TeamStart = Cast<AFusionPlayerStart>(SpawnPoint);
		AFusionPlayerState* PlayerState = Cast<AFusionPlayerState>(Player->PlayerState);

		if (PlayerState && TeamStart && TeamStart->SpawnTeam != PlayerState->GetTeamNum())
		{
			return false;
		}
	}

	return Super::IsSpawnpointAllowed(SpawnPoint, Player);
}


/*
void AFusionGameMode_TeamDeathMatch::InitBot(AShooterAIController* AIC, int32 BotNum)
{
	AShooterPlayerState* BotPlayerState = CastChecked<AShooterPlayerState>(AIC->PlayerState);
	const int32 TeamNum = ChooseTeam(BotPlayerState);
	BotPlayerState->SetTeamNum(TeamNum);

	Super::InitBot(AIC, BotNum);
}
*/



/*
void ASGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
Super::SetPlayerDefaults(PlayerPawn);

SpawnDefaultInventory(PlayerPawn);
}
*/


/*
void ASGameMode::SpawnDefaultInventory(APawn* PlayerPawn)
{
ASCharacter* MyPawn = Cast<ASCharacter>(PlayerPawn);
if (MyPawn)
{
for (int32 i = 0; i < DefaultInventoryClasses.Num(); i++)
{
if (DefaultInventoryClasses[i])
{
FActorSpawnParameters SpawnInfo;
SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
ASWeapon* NewWeapon = GetWorld()->SpawnActor<ASWeapon>(DefaultInventoryClasses[i], SpawnInfo);

MyPawn->AddWeapon(NewWeapon);
}
}
}
}
*/