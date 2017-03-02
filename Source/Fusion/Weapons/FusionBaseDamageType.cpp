// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"
#include "FusionBaseDamageType.h"

UFusionBaseDamageType::UFusionBaseDamageType(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}


bool UFusionBaseDamageType::GetCanDieFrom()
{
	return bCanDieFrom;
}


float UFusionBaseDamageType::GetHeadDamageModifier()
{
	return HeadDmgModifier;
}

float UFusionBaseDamageType::GetLimbDamageModifier()
{
	return LimbDmgModifier;
}