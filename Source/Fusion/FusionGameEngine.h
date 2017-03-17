// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameEngine.h"
#include "FusionGameEngine.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UFusionGameEngine : public UGameEngine
{
	GENERATED_BODY()

	UFusionGameEngine(const FObjectInitializer& ObjectInitializer);

	/* Hook up specific callbacks */
	virtual void Init(IEngineLoop* InEngineLoop);

public:

	/**
	* 	All regular engine handling, plus update ShooterKing state appropriately.
	*/
	virtual void HandleNetworkFailure(UWorld *World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString) override;
};

