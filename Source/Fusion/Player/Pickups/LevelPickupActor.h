// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Player/Pickups/MasterPickupActor.h"
#include "LevelPickupActor.generated.h"

/**
 * 
 */
UCLASS(ABSTRACT)
class FUSION_API ALevelPickupActor : public AMasterPickupActor
{
	GENERATED_BODY()


	void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* PickupSound;

	UFUNCTION()
	void OnRep_IsActive();

protected:

	ALevelPickupActor(const FObjectInitializer& ObjectInitializer);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

	virtual void RespawnPickup();

	virtual void OnPickedUp();

	virtual void OnRespawned();
	
	UPROPERTY(Transient, ReplicatedUsing = OnRep_IsActive)
	bool bIsActive;

public:

	virtual void OnUsed(APawn* InstigatorPawn) override;

	/* Immediately spawn on begin play */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	bool bStartActive;

	/* Will this item ever respawn */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	bool bAllowRespawn;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float RespawnDelay;

	/* Extra delay randomly applied on the respawn timer */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float RespawnDelayRange;


};