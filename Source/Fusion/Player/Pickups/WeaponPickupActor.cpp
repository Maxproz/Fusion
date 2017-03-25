// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "Weapons/MasterWeapon.h"

#include "FusionPlayerController.h"
#include "FusionCharacter.h"

#include "WeaponPickupActor.h"



AWeaponPickupActor::AWeaponPickupActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAllowRespawn = false;

	/* Enabled to support simulated physics movement when weapons are dropped by a player */
	bReplicateMovement = true;
}


void AWeaponPickupActor::OnUsed(APawn* InstigatorPawn)
{
	AFusionCharacter* MyPawn = Cast<AFusionCharacter>(InstigatorPawn);
	if (MyPawn)
	{
		/* Fetch the default variables of the class we are about to pick up and check if the storage slot is available on the pawn. */
		if (MyPawn->WeaponSlotAvailable(WeaponClass->GetDefaultObject<AMasterWeapon>()->GetWeaponSlot()))
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AMasterWeapon* NewWeapon = GetWorld()->SpawnActor<AMasterWeapon>(WeaponClass, SpawnInfo);

			MyPawn->AddWeapon(NewWeapon);

			Super::OnUsed(InstigatorPawn);
		}
		else
		{
			AFusionPlayerController* PC = Cast<AFusionPlayerController>(MyPawn->GetController());
			if (PC)
			{
				//PC->ClientHUDMessage(EHUDMessage::Weapon_Picked_Up);
			}
		}
	}
}


