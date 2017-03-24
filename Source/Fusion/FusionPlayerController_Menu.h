// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "FusionPlayerController_Menu.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API AFusionPlayerController_Menu : public APlayerController
{
	GENERATED_UCLASS_BODY()

	/** After game is initialized */
	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

public:

	/** Returns a pointer to the Fusion game hud. May return NULL. */
	class AFusionHUD* GetFusionHUD() const;


	//virtual void BeginPlay() override;
};