// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Player/Pickups/LevelPickupActor.h"
#include "WeaponPickupActor.generated.h"

/**
 * 
 */
UCLASS(ABSTRACT)
class FUSION_API AWeaponPickupActor : public ALevelPickupActor
{
	GENERATED_BODY()
	
	AWeaponPickupActor(const FObjectInitializer& ObjectInitializer);
	
public:

	/* Class to add to inventory when picked up */
	UPROPERTY(EditDefaultsOnly, Category = "WeaponClass")
	TSubclassOf<class AMasterWeapon> WeaponClass;

	virtual void OnUsed(APawn* InstigatorPawn) override;

};
