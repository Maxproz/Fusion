// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "FusionCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UFusionCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
	virtual float GetMaxSpeed() const override;

};