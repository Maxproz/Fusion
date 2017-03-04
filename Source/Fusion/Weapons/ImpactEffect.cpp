// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"
#include "ImpactEffect.h"

AImpactEffect::AImpactEffect()
{
	bAutoDestroyWhenFinished = true;
	/* Can ever tick is required to trigger bAutoDestroyWhenFinished, which is checked in AActor::Tick */
	PrimaryActorTick.bCanEverTick = true;

	DecalLifeSpan = 10.0f;
	DecalSize = 16.0f;
}


void AImpactEffect::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	/* Figure out what we hit (SurfaceHit is setting during actor instantiation in weapon class) */
	UPhysicalMaterial* HitPhysMat = SurfaceHit.PhysMaterial.Get();
	EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitPhysMat);

	UParticleSystem* ImpactFX = GetImpactFX(HitSurfaceType);
	
	if (ImpactFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, GetActorLocation(), GetActorRotation());
	}

	USoundCue* ImpactSound = GetImpactSound(HitSurfaceType);

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (DecalMaterial)
	{
		FRotator RandomDecalRotation = SurfaceHit.ImpactNormal.Rotation();
		RandomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);

		UGameplayStatics::SpawnDecalAttached(DecalMaterial, FVector(1.0f, DecalSize, DecalSize),
			SurfaceHit.Component.Get(), SurfaceHit.BoneName,
			SurfaceHit.ImpactPoint, RandomDecalRotation, EAttachLocation::KeepWorldPosition,
			DecalLifeSpan);
	}


}


UParticleSystem* AImpactEffect::GetImpactFX(EPhysicalSurface SurfaceType) const
{
	switch (SurfaceType)
	{
	case SURFACE_DEFAULT:
		return DefaultFX;
	case SURFACE_FLESH:
		return PlayerFleshFX;
	case SURFACE_BODY:
	case SURFACE_HEAD:
	case SURFACE_ARM:
		return PlayerFleshFX;
	default:
		return nullptr;
	}
}

// TODO: Figure out if the custom sounds are not playing, or if my volume was just too low,
// - OR if I was too far away from the impact to hear it. (Since it plays it at post initialization of the decal)
USoundCue* AImpactEffect::GetImpactSound(EPhysicalSurface SurfaceType) const
{
	switch (SurfaceType)
	{
	case SURFACE_DEFAULT:
		return DefaultSound;
	case SURFACE_FLESH:
		return PlayerFleshSound;
	case SURFACE_BODY:
	case SURFACE_HEAD:
	case SURFACE_ARM:
		return PlayerFleshSound;
	default:
		return nullptr;
	}
}