// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "WeaponTypes.generated.h"

/**
 * 
 */
UENUM()
enum class EWeaponTypes : uint8
{
	Rifle,
	Sniper,
	Rocket_Launcher,
	SMG,
	ShotGun,
	Sword,
};