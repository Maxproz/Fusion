// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionBaseCharacter.h"

#include "FusionCharacterMovementComponent.h"






float UFusionCharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AFusionBaseCharacter* CharOwner = Cast<AFusionBaseCharacter>(PawnOwner);
	if (CharOwner)
	{
		// Slow down during targeting or crouching
		if (IsCrouching())
		{
			MaxSpeed *= CharOwner->GetCrouchingSpeedModifier();
		}
		else if (CharOwner->IsSprinting())
		{
			MaxSpeed *= CharOwner->GetSprintingSpeedModifier();
		}
	}

	return MaxSpeed;
}

