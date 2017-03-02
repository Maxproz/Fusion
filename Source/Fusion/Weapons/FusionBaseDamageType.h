// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/DamageType.h"
#include "FusionBaseDamageType.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UFusionBaseDamageType : public UDamageType
{
	GENERATED_BODY()
	
	UFusionBaseDamageType(const FObjectInitializer& ObjectInitializer);

	/* Can player die from this damage type (eg. players don't die from hunger) */
	UPROPERTY(EditDefaultsOnly)
	bool bCanDieFrom = true;

	/* Damage modifier for headshot damage */
	UPROPERTY(EditDefaultsOnly)
	float HeadDmgModifier = 2.0f;

	UPROPERTY(EditDefaultsOnly)
	float LimbDmgModifier = 0.5f;

public:

	bool GetCanDieFrom();

	float GetHeadDamageModifier();

	float GetLimbDamageModifier();

};
