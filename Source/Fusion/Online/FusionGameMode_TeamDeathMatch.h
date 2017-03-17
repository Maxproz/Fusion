// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FusionGameMode.h"
#include "FusionGameMode_TeamDeathMatch.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API AFusionGameMode_TeamDeathMatch : public AFusionGameMode
{
	GENERATED_BODY()
	
	AFusionGameMode_TeamDeathMatch(const FObjectInitializer& ObjectInitializer);

	/** setup team changes at player login */
	void PostLogin(APlayerController* NewPlayer) override;

	/** initialize replicated game data */
	virtual void InitGameState() override;

	/** can players damage each other? */
	virtual bool CanDealDamage(class AFusionPlayerState* DamageInstigator, AFusionPlayerState* DamagedPlayer) const override;



protected:

	/* Can we deal damage to players in the same team */
	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	bool bAllowFriendlyFireDamage = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	bool bAllowSelfDamage = true;

	/** number of teams */
	int32 NumTeams;

	/** best team */
	int32 WinnerTeam;

	/** pick team with least players in or random when it's equal */
	int32 ChooseTeam(class AFusionPlayerState* ForPlayerState) const;

	/** check who won */
	virtual void DetermineMatchWinner() override;

	/** check if PlayerState is a winner */
	virtual bool IsWinner(class AFusionPlayerState* PlayerState) const override;

	/** check team constraints */
	virtual bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const;



	/** initialization for bot after spawning */
	//virtual void InitBot(AShooterAIController* AIC, int32 BotNum) override;


	/* Debug */
	/*
	UPROPERTY(VisibleInstanceOnly)
	int32 RedTeamPlayers = 0;

	UPROPERTY(VisibleInstanceOnly)
	int32 BlueTeamPlayers = 0;
	*/


	//virtual void SpawnDefaultInventory(APawn* PlayerPawn);

	/**
	* Make sure pawn properties are back to default
	* Also a good place to modify them on spawn
	*/
	//virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

	/* The default weapons to spawn with */
	//UPROPERTY(EditDefaultsOnly, Category = "Player")
	//TArray<TSubclassOf<class ASWeapon>> DefaultInventoryClasses;
};






		


