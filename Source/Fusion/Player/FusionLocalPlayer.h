// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/LocalPlayer.h"
#include "FusionLocalPlayer.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UFusionLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()
	

	/* Set a player name if no online system like Steam is available */
	virtual FString GetNickname() const override;

};
