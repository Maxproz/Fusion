// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "FusionPlayerCameraManager.generated.h"

/**
 * Not using anymore, this was setup for third person games and didnt work correctly for first person
 */
UCLASS()
class FUSION_API AFusionPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
	

	AFusionPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

	/* Update the FOV */
	virtual void UpdateCamera(float DeltaTime) override;

	virtual void BeginPlay() override;

	float CurrentCrouchOffset;

	/* Maximum camera offset applied when crouch is initiated. Always lerps back to zero */
	float MaxCrouchOffsetZ;

	float CrouchLerpVelocity;

	bool bWasCrouched;

	/* Default relative Z offset of the player camera */
	float DefaultCameraOffsetZ;


	/* default, hip fire FOV */
	//float NormalFOV;

	/* aiming down sight / zoomed FOV */
	//float TargetingFOV;


};
