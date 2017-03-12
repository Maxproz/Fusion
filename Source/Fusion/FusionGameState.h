// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"

#include "FusionPlayerController.h"

#include "Player/TeamColors.h"

#include "FusionGameState.generated.h"


/**
 * 
 */
UCLASS()
class FUSION_API AFusionGameState : public AGameState
{
	GENERATED_BODY()
	
	
	/* Total score of Red Team  */
	UPROPERTY(Replicated)
	int32 RedTeamScore = 0;
	
	/* Total score of Blue Team  */
	UPROPERTY(Replicated)
	int32 BlueTeamScore = 0;

public:

	AFusionGameState(const class FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetRedTeamScore();

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetBlueTeamScore();

	void AddScore(int32 Score, ETeamColors TeamColor);

	/** number of teams */
	UPROPERTY(Replicated)
	int32 NumTeams;

	/** time left for warmup / match */
	UPROPERTY(Transient, Replicated)
	int32 RemainingTime;

	/* Amount of time since the match started TODO: Figure out how I will track the game time later. */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Time")
	int32 ElapsedGameMinutes = 0;

	/* NetMulticast will send this event to all clients that know about this object, in the case of GameState that means every client. */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastGameMessage(EHUDMessage NewMessage);
	void BroadcastGameMessage_Implementation(EHUDMessage MessageID);




	void RequestFinishAndExitToMainMenu();
};
