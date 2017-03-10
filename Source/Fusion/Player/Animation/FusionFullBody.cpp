// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionCharacter.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

#include "FusionFullBody.h"


void UFusionFullBody::UpdateAnimationProperties(float DeltaTime)
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

			FVector PlayerVelocity = Player->GetVelocity();

			FRotator PlayerRot = Player->K2_GetActorRotation();

			FRotator RotatedXVec = UKismetMathLibrary::MakeRotFromX(PlayerVelocity);
			FRotator InvertPlayerRot = UKismetMathLibrary::NegateRotator(PlayerRot);

			FRotator CombinedRots = UKismetMathLibrary::ComposeRotators(RotatedXVec, InvertPlayerRot);

			Speed = PlayerVelocity.Size();
			UE_LOG(LogTemp, Warning, TEXT("Player Speed %f"), Speed);

			if (Speed > 0.f)
			{
				if (CombinedRots.Yaw >= 180.f)
				{
					Direction = CombinedRots.Yaw - 360.f;
				}
				else
				{
					Direction = CombinedRots.Yaw;
				}
			}
			else
			{
				Direction = 0.f;
			}

			FRotator AimOffset = Player->GetAimOffsets();
			AimPitch = AimOffset.Pitch / 180.f;
			AimYaw = AimOffset.Yaw / 180.f;

			//bIsJumping = Player->bIsJumping;
			bIsJumping = Player->GetVelocity().Z > 1.f;

			if (bIsJumping)
			{
				JumpTime = JumpTime + DeltaTime;
			}
			else
			{
				JumpTime = 0.f;
			}

			bIsZooming = Player->bIsZooming;
			bIsSprinting = Player->IsSprinting();
			bIsReloading = Player->bIsReloading;
			bIsWalking = (Player->GetVelocity() != FVector(0.f, 0.f, 0.f));
			bIsCrouched = Player->bIsCrouched;
		}
	}
}

void UFusionFullBody::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	UpdateAnimationProperties(DeltaSeconds);
}


