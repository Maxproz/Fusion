// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionCharacter.h"

#include "FusionArms.h"


void UFusionArms::UpdateAnimationProperties()
{
	//Get the pawn which is affected by our anim instance
	APawn* Pawn = TryGetPawnOwner();

	if (Pawn)
	{
		//Update our falling property
		//bIsInAir = Pawn->GetMovementComponent()->IsFalling();
		//Update our movement speed
		//Speed = Pawn->GetVelocity().Size();


		Player = Cast<AFusionCharacter>(Pawn);

		if (Player)
		{
			bIsSprinting = Player->IsSprinting();
			//bIsReloading = Player->bIsReloading;
			bIsZooming = Player->bIsZooming;
			bIsWalking = (Player->GetVelocity() != FVector(0.f,0.f,0.f));

		}
	}
}

void UFusionArms::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	UpdateAnimationProperties();
}


