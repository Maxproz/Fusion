// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"
#include "LevelPickupActor.h"




ALevelPickupActor::ALevelPickupActor(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* Ignore Pawn - this is to prevent objects shooting through the level or pawns glitching on top of small items. */
	MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	bIsActive = false;
	bStartActive = true;
	bAllowRespawn = true;
	RespawnDelay = 5.0f;
	RespawnDelayRange = 5.0f;

	SetReplicates(true);
}


void ALevelPickupActor::BeginPlay()
{
	Super::BeginPlay();

	//if (bStartActive)
	{
		RespawnPickup();
	}
}


void ALevelPickupActor::OnUsed(APawn* InstigatorPawn)
{
	Super::OnUsed(InstigatorPawn);

	UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());

	bIsActive = false;
	OnPickedUp();

	if (bAllowRespawn)
	{
		FTimerHandle RespawnTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &ALevelPickupActor::RespawnPickup, RespawnDelay + FMath::RandHelper(RespawnDelayRange), false);
	}
	else
	{
		/* Delete from level if respawn is not allowed */
		Destroy();
	}
}


void ALevelPickupActor::OnPickedUp()
{
	if (MeshComp)
	{
		MeshComp->SetVisibility(false);
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}


void ALevelPickupActor::RespawnPickup()
{
	bIsActive = true;
	OnRespawned();
}


void ALevelPickupActor::OnRespawned()
{
	if (MeshComp)
	{
		MeshComp->SetVisibility(true);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}


void ALevelPickupActor::OnRep_IsActive()
{
	if (bIsActive)
	{
		OnRespawned();
	}
	else
	{
		OnPickedUp();
	}
}


void ALevelPickupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALevelPickupActor, bIsActive);
}