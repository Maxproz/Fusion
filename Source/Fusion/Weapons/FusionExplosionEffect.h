// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"

#include "Types/TakeHitInfo.h"

#include "FusionExplosionEffect.generated.h"


//
// Spawnable effect for explosion - NOT replicated to clients
// Each explosion type should be defined as separate blueprint
//
UCLASS(Abstract, Blueprintable)
class FUSION_API AFusionExplosionEffect : public AActor
{
	GENERATED_BODY()

	/** explosion FX */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
	UParticleSystem* ExplosionFX;

private:
	/** explosion light */
	UPROPERTY(VisibleDefaultsOnly, Category = Effect)
	UPointLightComponent* ExplosionLight;

public:

	AFusionExplosionEffect(const FObjectInitializer& ObjectInitializer);


	/** how long keep explosion light on? */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
	float ExplosionLightFadeOut;

	/** explosion sound */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
	USoundCue* ExplosionSound;

	/** explosion decals */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
	struct FDecalData Decal;

	/** surface data for spawning */
	UPROPERTY(BlueprintReadOnly, Category = Surface)
	FHitResult SurfaceHit;

	/** spawn explosion */
	virtual void BeginPlay() override;

	/** update fading light */
	virtual void Tick(float DeltaSeconds) override;

private:

	/** Point light component name */
	FName ExplosionLightComponentName;

public:
	/** Returns ExplosionLight subobject **/
	FORCEINLINE UPointLightComponent* GetExplosionLight() const { return ExplosionLight; }
};
