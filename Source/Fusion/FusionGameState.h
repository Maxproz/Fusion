// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"

#include "FusionPlayerController.h"

#include "Player/TeamColors.h"

#include "FusionGameState.generated.h"


/** ranked PlayerState map, created from the GameState */
typedef TMap <int32,TWeakObjectPtr<class AFusionPlayerState>> RankedPlayerMap;

/**
 * 
 */
UCLASS()
class FUSION_API AFusionGameState : public AGameState
{
	GENERATED_BODY()
	

public:

	AFusionGameState(const class FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;



	/** number of teams in current game (doesn't deprecate when no players are left in a team) */
	UPROPERTY(Transient, Replicated)
	int32 NumTeams;

	/** accumulated score per team */
	UPROPERTY(Transient, Replicated)
	TArray<int32> TeamScores;

	/** time left for warmup / match */
	UPROPERTY(Transient, Replicated)
	int32 RemainingTime;

	/** is timer paused? */
	UPROPERTY(Transient, Replicated)
	bool bTimerPaused;

	/** gets ranked PlayerState map for specific team */
	void GetRankedMap(int32 TeamIndex, RankedPlayerMap& OutRankedMap) const;

	void RequestFinishAndExitToMainMenu();




	/* NetMulticast will send this event to all clients that know about this object, in the case of GameState that means every client. */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastGameMessage(EHUDMessage NewMessage);
	void BroadcastGameMessage_Implementation(EHUDMessage MessageID);

	
};
