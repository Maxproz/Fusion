// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"

#include "Player/TeamColors.h"

#include "FusionPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API AFusionPlayerState : public APlayerState
{
	GENERATED_BODY()
	

	AFusionPlayerState(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Transient, Replicated)
	int32 NumKills;

	UPROPERTY(Transient, Replicated)
	int32 NumDeaths;

	/* Team color/number assigned to player */
	UPROPERTY(Transient, Replicated)
	ETeamColors TeamColor;


	/*
	UPROPERTY(Transient, Replicated) // TODO: implement later if kills/deaths is working.
	int32 NumAssists;
	*/

	virtual void Reset() override;

	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const override;

public:

	void AddKill();

	void AddDeath();

	void AddScore(int32 Amount);

	void SetTeamColor(ETeamColors NewTeamColor);

	UFUNCTION(BlueprintCallable, Category = "Teams")
	ETeamColors GetTeamColor() const;

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetKills() const;

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetDeaths() const;

	UFUNCTION(BlueprintCallable, Category = "Score")
	float GetScore() const;


	

	/*
	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetAssists() const;
	*/

};
