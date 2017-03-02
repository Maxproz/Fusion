// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "EquippedWeaponTypes.generated.h"

/**
 * 
 */
UENUM()
enum class EEquippedWeaponTypes : uint8
{
	EWT_Primary,
	EWT_Secondary,
	EWT_None,
};