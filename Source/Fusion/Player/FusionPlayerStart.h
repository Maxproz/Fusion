// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "FusionPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API AFusionPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
	AFusionPlayerStart(const class FObjectInitializer& ObjectInitializer);

	/* Is only useable by players - automatically a preferred spawn for players */
	UPROPERTY(EditAnywhere, Category = "PlayerStart")
	bool bPlayerOnly;




public:

	bool GetIsPlayerOnly() { return bPlayerOnly; }
	/** Which team can start at this point */
	
	UPROPERTY(EditInstanceOnly, Category = Team)
	int32 SpawnTeam;
	
};
