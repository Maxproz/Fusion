// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameMode.h"

#include "Components/FusionCharacterMovementComponent.h"

#include "Weapons/FusionBaseDamageType.h"

#include "FusionPlayerController.h"

#include "FusionBaseCharacter.h"


AFusionBaseCharacter::AFusionBaseCharacter(const class FObjectInitializer& ObjectInitializer)
/* Override the movement class from the base class to our own to support multiple speeds (eg. sprinting) */
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UFusionCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	CrouchingSpeedModifier = 0.5f;
	SprintingSpeedModifier = 2.0f;

	/* Don't collide with camera checks to keep 3rd person camera at position when zombies or other players are standing behind us */ // TODO: Is setting this causing anything bad?
	//GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	//GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

float AFusionBaseCharacter::GetHealth() const
{
	return Health;
}

float AFusionBaseCharacter::GetMaxHealth() const
{
	// Retrieve the default value of the health property that is assigned on instantiation.
	return GetClass()->GetDefaultObject<AFusionBaseCharacter>()->Health;
}

bool AFusionBaseCharacter::IsAlive() const
{
	return Health > 0;
}

float AFusionBaseCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	if (Health <= 0.f)
	{
		return 0.f;
	}

	/* Modify based based on gametype rules */
	AFusionGameMode* MyGameMode = Cast<AFusionGameMode>(GetWorld()->GetAuthGameMode());
	Damage = MyGameMode ? MyGameMode->ModifyDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser) : Damage;

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	
	// TODO: Make it so the player cannot have health and shields subtracted at the same time... small bug.

	if (ActualDamage > 0.f)
	{
		if (Shields > 0)
		{
			Shields -= ActualDamage;
			Recharge = 0;
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("Shields after damage: Shields: %f"), Shields));
		}
		
		if (Shields <= 0)
		{

			Health -= ActualDamage;
			Recharge = 0;
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Health after damage: Health: %f"), Health));

			if (Health <= 0)
			{
				bool bCanDie = true;

				/* Check the damagetype, always allow dying if the cast fails, otherwise check the property if player can die from damagetype */
				if (DamageEvent.DamageTypeClass)
				{
					UFusionBaseDamageType* DmgType = Cast<UFusionBaseDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
					bCanDie = (DmgType == nullptr || (DmgType && DmgType->GetCanDieFrom()));
				}

				if (bCanDie)
				{
					Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
				}
				else
				{
					/* Player cannot die from this damage type, set hitpoints to 1.0 */
					Health = 1.0f;
				}
			}
			else
			{
				/* Shorthand for - if x != null pick1 else pick2 */
				APawn* Pawn = EventInstigator ? EventInstigator->GetPawn() : nullptr;
				PlayHit(ActualDamage, DamageEvent, Pawn, DamageCauser, false);
			}
		}
	}

	return ActualDamage;
}


bool AFusionBaseCharacter::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const
{
	/* Check if character is already dying, destroyed or if we have authority */
	if (bIsDying ||
		IsPendingKill() ||
		Role != ROLE_Authority ||
		GetWorld()->GetAuthGameMode() == NULL)
	{
		return false;
	}

	return true;
}


void AFusionBaseCharacter::FellOutOfWorld(const class UDamageType& DmgType)
{
	Die(Health, FDamageEvent(DmgType.GetClass()), NULL, NULL);
}


bool AFusionBaseCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if (!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		return false;
	}

	Health = FMath::Min(0.0f, Health);

	/* Fallback to default DamageType if none is specified */
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	/* Notify the gamemode we got killed for scoring and game over state */
	AController* KilledPlayer = Controller ? Controller : Cast<AController>(GetOwner());
	GetWorld()->GetAuthGameMode<AFusionGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}

// TODO: need to sync this functionality with my FPS template. Need mesh etc...
void AFusionBaseCharacter::OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser)
{
	if (bIsDying)
	{
		return;
	}

	bReplicateMovement = false;
	bTearOff = true;
	bIsDying = true;

	PlayHit(KillingDamage, DamageEvent, PawnInstigator, DamageCauser, true);

	DetachFromControllerPendingDestroy();

	/* Disable all collision on capsule */
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	USkeletalMeshComponent* Mesh3P = GetMesh();
	if (Mesh3P)
	{
		Mesh3P->SetCollisionProfileName(TEXT("Ragdoll"));
	}
	SetActorEnableCollision(true);

	SetRagdollPhysics();

	/* Apply physics impulse on the bone of the enemy skeleton mesh we hit (ray-trace damage only) */
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		FPointDamageEvent PointDmg = *((FPointDamageEvent*)(&DamageEvent));
		{
			// TODO: Use DamageTypeClass->DamageImpulse
			Mesh3P->AddImpulseAtLocation(PointDmg.ShotDirection * 12000, PointDmg.HitInfo.ImpactPoint, PointDmg.HitInfo.BoneName);
		}
	}
	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		FRadialDamageEvent RadialDmg = *((FRadialDamageEvent const*)(&DamageEvent));
		{
			Mesh3P->AddRadialImpulse(RadialDmg.Origin, RadialDmg.Params.GetMaxRadius(), 100000 /*RadialDmg.DamageTypeClass->DamageImpulse*/, ERadialImpulseFalloff::RIF_Linear);
		}
	}
}


void AFusionBaseCharacter::SetRagdollPhysics()
{
	bool bInRagdoll = false;
	USkeletalMeshComponent* Mesh3P = GetMesh();

	if (IsPendingKill())
	{
		bInRagdoll = false;
	}
	else if (!Mesh3P || !Mesh3P->GetPhysicsAsset())
	{
		bInRagdoll = false;
	}
	else
	{
		Mesh3P->SetAllBodiesSimulatePhysics(true);
		Mesh3P->SetSimulatePhysics(true);
		Mesh3P->WakeAllRigidBodies();
		Mesh3P->bBlendPhysics = true;

		bInRagdoll = true;
	}

	UCharacterMovementComponent* CharacterComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
	if (CharacterComp)
	{
		CharacterComp->StopMovementImmediately();
		CharacterComp->DisableMovement();
		CharacterComp->SetComponentTickEnabled(false);
	}

	if (!bInRagdoll)
	{
		// Immediately hide the pawn
		TurnOff();
		SetActorHiddenInGame(true);
		SetLifeSpan(1.0f);
	}
	else
	{
		SetLifeSpan(10.0f);
	}
}


void AFusionBaseCharacter::PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled)
{
	if (Role == ROLE_Authority)
	{
		ReplicateHit(DamageTaken, DamageEvent, PawnInstigator, DamageCauser, bKilled);
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		if (bKilled && SoundDeath)
		{
			UGameplayStatics::SpawnSoundAttached(SoundDeath, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true);
		}
		else if (SoundTakeHit)
		{
			UGameplayStatics::SpawnSoundAttached(SoundTakeHit, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true);
		}
	}
}


void AFusionBaseCharacter::ReplicateHit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled)
{
	const float TimeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	FDamageEvent const& LastDamageEvent = LastTakeHitInfo.GetDamageEvent();
	if (PawnInstigator == LastTakeHitInfo.PawnInstigator.Get() && LastDamageEvent.DamageTypeClass == LastTakeHitInfo.DamageTypeClass)
	{
		// Same frame damage
		if (bKilled && LastTakeHitInfo.bKilled)
		{
			// Redundant death take hit, ignore it
			return;
		}

		DamageTaken += LastTakeHitInfo.ActualDamage;
	}

	LastTakeHitInfo.ActualDamage = DamageTaken;
	LastTakeHitInfo.PawnInstigator = Cast<AFusionBaseCharacter>(PawnInstigator);
	LastTakeHitInfo.DamageCauser = DamageCauser;
	LastTakeHitInfo.SetDamageEvent(DamageEvent);
	LastTakeHitInfo.bKilled = bKilled;
	LastTakeHitInfo.EnsureReplication();
}


void AFusionBaseCharacter::OnRep_LastTakeHitInfo()
{
	if (LastTakeHitInfo.bKilled)
	{
		OnDeath(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
	}
	else
	{
		PlayHit(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get(), LastTakeHitInfo.bKilled);
	}
}


void AFusionBaseCharacter::SetSprinting(bool NewSprinting)
{
	bWantsToSprint = NewSprinting;

	if (bIsCrouched)
	{
		UnCrouch();
	}

	if (Role < ROLE_Authority)
	{
		ServerSetSprinting(NewSprinting);
	}
}


void AFusionBaseCharacter::ServerSetSprinting_Implementation(bool NewSprinting)
{
	SetSprinting(NewSprinting);
}


bool AFusionBaseCharacter::ServerSetSprinting_Validate(bool NewSprinting)
{
	return true;
}


bool AFusionBaseCharacter::IsSprinting() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	return bWantsToSprint && !GetVelocity().IsZero()
		// Don't allow sprint while strafing sideways or standing still (1.0 is straight forward, -1.0 is backward while near 0 is sideways or standing still)
		&& (FVector::DotProduct(GetVelocity().GetSafeNormal2D(), GetActorRotation().Vector()) > 0.8); // Changing this value to 0.1 allows for diagonal sprinting. (holding W+A or W+D keys)
}


float AFusionBaseCharacter::GetSprintingSpeedModifier() const
{
	return SprintingSpeedModifier;
}

void AFusionBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Value is already updated locally, skip in replication step
	DOREPLIFETIME_CONDITION(AFusionBaseCharacter, bWantsToSprint, COND_SkipOwner);
	//DOREPLIFETIME_CONDITION(AFusionBaseCharacter, bIsTargeting, COND_SkipOwner);

	// Replicate to every client, no special condition required
	DOREPLIFETIME(AFusionBaseCharacter, Health);
	DOREPLIFETIME(AFusionBaseCharacter, Shields);
	DOREPLIFETIME(AFusionBaseCharacter, LastTakeHitInfo);
}


float AFusionBaseCharacter::GetCrouchingSpeedModifier() const
{
	return CrouchingSpeedModifier;
}

void AFusionBaseCharacter::ServerTakeDamage_Implementation(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

bool AFusionBaseCharacter::ServerTakeDamage_Validate(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return true;
}


// Recharge shields Tick
void AFusionBaseCharacter::RechargeShields()
{
	// Not bad... sorta works, need to clean this up into two different timers later
	// - so the shield recharge will look clean in the UI. Maybe filter out the other half of this function into a 0.2 - 0.4 second timer that will refill to 100

	Recharge++;
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("RechargeTime: %u"), Recharge));
	

	// TODO: Hopefully this works.
	if (Recharge > StartRecharging)
	{
		if (IsAlive() && Shields < 100) // do not tick if Player is dead
		{
			Shields = Shields + ShieldsRechargeRate;
			ForceShieldsCap();
		}
	}

	// Call Regen again after 1 second
	FTimerHandle CastTimer;
	GetWorldTimerManager().SetTimer(CastTimer, this, &AFusionBaseCharacter::RechargeShields, 1.f, false);
}

// Force HP Caps
void AFusionBaseCharacter::ForceHealthCap()
{
	// If HP is greater than Max, set HP to Max
	if (Health > GetMaxHealth())
	{
		Health = GetMaxHealth();
	}
	// If HP is less than Zero, set HP to Zero
	if (Health < 0)
	{
		Health = 0;
	}
}

// Force HP Caps
void AFusionBaseCharacter::ForceShieldsCap()
{
	// If Shields is greater than Max, set Shields to Max
	if (Shields > GetMaxShields())
	{
		AFusionPlayerController* PC = Cast<AFusionPlayerController>(Controller);
		if (PC)
		{
			PC->ClientHUDMessage(EHUDMessage::Character_Shields_Recharged);
		}
		Shields = GetMaxShields();
	}
	// If Shields is less than Zero, set Shields to Zero
	if (Shields < 0)
	{
		Shields = 0;
	}
}

// HealthPickup etc..
void AFusionBaseCharacter::RestoreHealth(float HealthRestored)
{
	// Restore Hitpoints
	Health = FMath::Clamp(Health + HealthRestored, 0.0f, GetMaxHealth());
}
