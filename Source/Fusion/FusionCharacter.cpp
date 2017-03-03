// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Fusion.h"
#include "FusionCharacter.h"
#include "FusionProjectile.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "FusionPlayerController.h"

#include "Weapons/MasterWeapon.h"

#include "GameFramework/InputSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AFusionCharacter

AFusionCharacter::AFusionCharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint")); // done in beginplay
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);



	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	// Adjust jump to make it less floaty
	MoveComp->GravityScale = 1.5f;
	MoveComp->JumpZVelocity = 620;
	MoveComp->bCanWalkOffLedgesWhenCrouching = true;
	MoveComp->MaxWalkSpeedCrouched = 200;
	// Enable crouching
	MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;

	/* Ignore this channel or it will absorb the trace impacts instead of the skeletal mesh */
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);


	/* Names as specified in the character skeleton */
	WeaponAttachPoint = TEXT("WeaponAttachPoint");
	BackAttachPoint = TEXT("BackAttachPoint");

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.
}

void AFusionCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Mesh1P->SetHiddenInGame(false, true);

	// Initatite our OnTick Function for shield recharging behaviour
	RechargeShields();
}

void AFusionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bWantsToSprint && !IsSprinting())
	{
		SetSprinting(true);
	}

	

}

void AFusionCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	//DestroyInventory();
}

void AFusionCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	/* Equip the weapon on the client side. */
	//SetCurrentWeapon(CurrentWeapon);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFusionCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFusionCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFusionCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFusionCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFusionCharacter::LookUpAtRate);


	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFusionCharacter::AttemptToFire);
	
	// TODO: ill probably need a binding like this for automatic weapons.
	//PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFusionCharacter::OnStopFire);


	PlayerInputComponent->BindAction("SprintHold", IE_Pressed, this, &AFusionCharacter::OnStartSprinting);
	PlayerInputComponent->BindAction("SprintHold", IE_Released, this, &AFusionCharacter::OnStopSprinting);

	PlayerInputComponent->BindAction("CrouchToggle", IE_Released, this, &AFusionCharacter::OnCrouchToggle);

	// use this for "SwapWeapon"
	//PlayerInputComponent->BindAction("SwapWeapon", IE_Pressed, this, &AFusionCharacter::OnSwapWeapon);

	/* Input binding for the carry object component */
	// Use this for the pickup flag/powerup/weapon binding
	//PlayerInputComponent->BindAction("PickupObject", IE_Pressed, this, &AFusionCharacter::OnToggleCarryActor);

}

void AFusionCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{

			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// spawn the projectile at the muzzle
			World->SpawnActor<AFusionProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AFusionCharacter::MoveForward(float Value)
{
	// old
	/*
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
	*/ 

	// new test version
	if (Controller && Value != 0.f)
	{
		// Limit pitch when walking or falling
		const bool bLimitRotation = (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling());
		const FRotator Rotation = bLimitRotation ? GetActorRotation() : Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);

		AddMovementInput(Direction, Value);
	}
}

void AFusionCharacter::MoveRight(float Value)
{
	// old
	/*
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
	*/

	// new test version
	if (Value != 0.f)
	{
		const FRotator Rotation = GetActorRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

	}
}

void AFusionCharacter::OnJump()
{
	SetIsJumping(true);
}


bool AFusionCharacter::IsInitiatedJump() const
{
	return bIsJumping;
}


void AFusionCharacter::SetIsJumping(bool NewJumping)
{
	// Go to standing pose if trying to jump while crouched
	if (bIsCrouched && NewJumping)
	{
		UnCrouch();
	}
	else if (NewJumping != bIsJumping)
	{
		bIsJumping = NewJumping;

		if (bIsJumping)
		{
			/* Perform the built-in Jump on the character */
			Jump();
		}
	}

	if (Role < ROLE_Authority)
	{
		ServerSetIsJumping(NewJumping);
	}
}


void AFusionCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	/* Check if we are no longer falling/jumping */
	if (PrevMovementMode == EMovementMode::MOVE_Falling && 
		GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Falling)
	{
		SetIsJumping(false);
	}
}

void AFusionCharacter::ServerSetIsJumping_Implementation(bool NewJumping)
{
	SetIsJumping(NewJumping);
}


bool AFusionCharacter::ServerSetIsJumping_Validate(bool NewJumping)
{
	return true;
}


void AFusionCharacter::OnStartSprinting()
{
	// TODO: Need to make it so you cant sprint with flag/bomb here later on.
	//if (CarriedObjectComp->GetIsCarryingActor())
	//{
		//CarriedObjectComp->Drop();
	//}

	SetSprinting(true);
}


void AFusionCharacter::OnStopSprinting()
{
	SetSprinting(false);
}

void AFusionCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Value is already updated locally, skip in replication step
	DOREPLIFETIME_CONDITION(AFusionCharacter, bIsJumping, COND_SkipOwner);

	// Replicate to every client, no special condition required
	DOREPLIFETIME(AFusionCharacter, LastTakeHitInfo);



	//DOREPLIFETIME(ASCharacter, CurrentWeapon);
	//DOREPLIFETIME(ASCharacter, Inventory);
	/* If we did not display the current inventory on the player mesh we could optimize replication by using this replication condition. */
	/* DOREPLIFETIME_CONDITION(ASCharacter, Inventory, COND_OwnerOnly);*/
}

void AFusionCharacter::OnCrouchToggle()
{
	if (IsSprinting())
	{
		SetSprinting(false);
	}

	// If we are crouching then CanCrouch will return false. If we cannot crouch then calling Crouch() wont do anything
	if (CanCrouch())
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}

void AFusionCharacter::OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser)
{
	if (bIsDying)
	{
		return;
	}

	StopAllAnimMontages();

	Super::OnDeath(KillingDamage, DamageEvent, PawnInstigator, DamageCauser);
}

void AFusionCharacter::StopAllAnimMontages()
{
	USkeletalMeshComponent* UseMesh = GetMesh();
	if (UseMesh && UseMesh->AnimScriptInstance)
	{
		UseMesh->AnimScriptInstance->Montage_Stop(0.0f);
	}
}


bool AFusionCharacter::CanFire() const
{
	// Add your own checks here, for example non-shooting areas or checking if player is in an NPC dialogue etc. 
	return IsAlive();
}


bool AFusionCharacter::CanReload() const
{
	return IsAlive();
}


bool AFusionCharacter::IsFiring() const
{
	return CurrentWeapon && CurrentWeapon->GetCurrentState() == EWeaponState::Firing;
}


FName AFusionCharacter::GetInventoryAttachPoint(EEquippedWeaponTypes WeaponSlot) const
{
	/* Return the socket name for the specified storage slot */
	switch (WeaponSlot)
	{
		case EEquippedWeaponTypes::EWT_Primary:
		{
			return WeaponAttachPoint;
		}
		case EEquippedWeaponTypes::EWT_Secondary:
		{
			// TODO: Add another switch statement here to attach the secondary to pelvis or back depending on weapon size(rifle, pistol types etc..)
			return BackAttachPoint;
		}
		default:
		// Not implemented.
		return "";
	}
}

AMasterWeapon* AFusionCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

void AFusionCharacter::OnRep_CurrentWeapon(AMasterWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}

void AFusionCharacter::SetCurrentWeapon(class AMasterWeapon* NewWeapon, class AMasterWeapon* LastWeapon)
{
	// Maintain a reference for visual weapon swapping 
	PreviousWeapon = LastWeapon;

	AMasterWeapon* LocalLastWeapon = nullptr;

	if (LastWeapon)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	// UnEquip the current
	bool bHasPreviousWeapon = false;
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
		bHasPreviousWeapon = true;
	}

	CurrentWeapon = NewWeapon;

	if (NewWeapon)
	{
		NewWeapon->SetOwningPawn(this);
		// Only play equip animation when we already hold an item in hands 
		NewWeapon->OnEquip(bHasPreviousWeapon);
	}

	// NOTE: If you don't have an equip animation w/ animnotify to swap the meshes halfway through, then uncomment this to immediately swap instead 
	//SwapToNewWeaponMesh();

}


/*









void ASCharacter::EquipWeapon(ASWeapon* Weapon)
{
	if (Weapon)
	{
		// Ignore if trying to equip already equipped weapon 
		if (Weapon == CurrentWeapon)
			return;

		if (Role == ROLE_Authority)
		{
			SetCurrentWeapon(Weapon, CurrentWeapon);
		}
		else
		{
			ServerEquipWeapon(Weapon);
		}
	}
}


bool ASCharacter::ServerEquipWeapon_Validate(ASWeapon* Weapon)
{
	return true;
}


void ASCharacter::ServerEquipWeapon_Implementation(ASWeapon* Weapon)
{
	EquipWeapon(Weapon);
}


void ASCharacter::AddWeapon(class ASWeapon* Weapon)
{
	if (Weapon && Role == ROLE_Authority)
	{
		Weapon->OnEnterInventory(this);
		Inventory.AddUnique(Weapon);

		// Equip first weapon in inventory
		if (Inventory.Num() > 0 && CurrentWeapon == nullptr)
		{
			EquipWeapon(Inventory[0]);
		}
	}
}


void ASCharacter::RemoveWeapon(class ASWeapon* Weapon, bool bDestroy)
{
	if (Weapon && Role == ROLE_Authority)
	{
		bool bIsCurrent = CurrentWeapon == Weapon;

		if (Inventory.Contains(Weapon))
		{
			Weapon->OnLeaveInventory();
		}
		Inventory.RemoveSingle(Weapon);

		// Replace weapon if we removed our current weapon 
		if (bIsCurrent && Inventory.Num() > 0)
		{
			SetCurrentWeapon(Inventory[0]);
		}

		// Clear reference to weapon if we have no items left in inventory 
		if (Inventory.Num() == 0)
		{
			SetCurrentWeapon(nullptr);
		}

		if (bDestroy)
		{
			Weapon->Destroy();
		}
	}
}

void ASCharacter::OnReload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartReload();
	}
}


void ASCharacter::OnStartFire()
{
	if (IsSprinting())
	{
		SetSprinting(false);
	}

	if (CarriedObjectComp->GetIsCarryingActor())
	{
		StopWeaponFire();

		CarriedObjectComp->Throw();
		return;
	}

	StartWeaponFire();
}


void ASCharacter::OnStopFire()
{
	StopWeaponFire();
}


void ASCharacter::StartWeaponFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		if (CurrentWeapon)
		{
			CurrentWeapon->StartFire();
		}
	}
}


void ASCharacter::StopWeaponFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
	}
}


void ASCharacter::OnSwapWeapon()
{
	if (CarriedObjectComp->GetIsCarryingActor())
	{
		CarriedObjectComp->Rotate(0.0f, 1.0f);
		return;
	}

	if (Inventory.Num() >= 2) // TODO: Check for weaponstate.
	{
		const int32 CurrentWeaponIndex = Inventory.IndexOfByKey(CurrentWeapon);
		ASWeapon* NextWeapon = Inventory[(CurrentWeaponIndex + 1) % Inventory.Num()];
		EquipWeapon(NextWeapon);
	}
}


void ASCharacter::OnPrevWeapon()
{
	if (CarriedObjectComp->GetIsCarryingActor())
	{
		CarriedObjectComp->Rotate(0.0f, -1.0f);
		return;
	}

	if (Inventory.Num() >= 2) // TODO: Check for weaponstate.
	{
		const int32 CurrentWeaponIndex = Inventory.IndexOfByKey(CurrentWeapon);
		ASWeapon* PrevWeapon = Inventory[(CurrentWeaponIndex - 1 + Inventory.Num()) % Inventory.Num()];
		EquipWeapon(PrevWeapon);
	}
}


void ASCharacter::DropWeapon()
{
	if (Role < ROLE_Authority)
	{
		ServerDropWeapon();
		return;
	}

	if (CurrentWeapon)
	{
		FVector CamLoc;
		FRotator CamRot;

		if (Controller == nullptr)
		{
			return;
		}		
		
		// Find a location to drop the item, slightly in front of the player.
		// Perform ray trace to check for blocking objects or walls and to make sure we don't drop any item through the level mesh 
		Controller->GetPlayerViewPoint(CamLoc, CamRot);
		FVector SpawnLocation;
		FRotator SpawnRotation = CamRot;

		const FVector Direction = CamRot.Vector();
		const FVector TraceStart = GetActorLocation();
		const FVector TraceEnd = GetActorLocation() + (Direction * DropWeaponMaxDistance);

		// Setup the trace params, we are only interested in finding a valid drop position 
		FCollisionQueryParams TraceParams;
		TraceParams.bTraceComplex = false;
		TraceParams.bReturnPhysicalMaterial = false;
		TraceParams.AddIgnoredActor(this);

		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldDynamic, TraceParams);

		// Find farthest valid spawn location 
		if (Hit.bBlockingHit)
		{
			// Slightly move away from impacted object 
			SpawnLocation = Hit.ImpactPoint + (Hit.ImpactNormal * 20);
		}
		else
		{
			SpawnLocation = TraceEnd;
		}

		// Spawn the "dropped" weapon 
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASWeaponPickup* NewWeaponPickup = GetWorld()->SpawnActor<ASWeaponPickup>(CurrentWeapon->WeaponPickupClass, SpawnLocation, FRotator::ZeroRotator, SpawnInfo);

		if (NewWeaponPickup)
		{
			// Apply torque to make it spin when dropped. 
			UStaticMeshComponent* MeshComp = NewWeaponPickup->GetMeshComponent();
			if (MeshComp)
			{
				MeshComp->SetSimulatePhysics(true);
				MeshComp->AddTorque(FVector(1, 1, 1) * 4000000);
			}
		}

		RemoveWeapon(CurrentWeapon, true);
	}
}


void ASCharacter::ServerDropWeapon_Implementation()
{
	DropWeapon();
}


bool ASCharacter::ServerDropWeapon_Validate()
{
	return true;
}


void ASCharacter::OnEquipPrimaryWeapon()
{
	if (CarriedObjectComp->GetIsCarryingActor())
	{
		CarriedObjectComp->Rotate(1.0f, 0.0f);
		return;
	}

	if (Inventory.Num() >= 1)
	{
		// Find first weapon that uses primary slot. 
		for (int32 i = 0; i < Inventory.Num(); i++)
		{
			ASWeapon* Weapon = Inventory[i];
			if (Weapon->GetStorageSlot() == EInventorySlot::Primary)
			{
				EquipWeapon(Weapon);
			}
		}
	}
}


void ASCharacter::OnEquipSecondaryWeapon()
{
	if (CarriedObjectComp->GetIsCarryingActor())
	{
		CarriedObjectComp->Rotate(-1.0f, 0.0f);
		return;
	}

	if (Inventory.Num() >= 2)
	{
		// Find first weapon that uses secondary slot. 
		for (int32 i = 0; i < Inventory.Num(); i++)
		{
			ASWeapon* Weapon = Inventory[i];
			if (Weapon->GetStorageSlot() == EInventorySlot::Secondary)
			{
				EquipWeapon(Weapon);
			}
		}
	}
}

bool ASCharacter::WeaponSlotAvailable(EInventorySlot CheckSlot)
{
	// Iterate all weapons to see if requested slot is occupied 
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		ASWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			if (Weapon->GetStorageSlot() == CheckSlot)
				return false;
		}
	}

	return true;

	// Special find function as alternative to looping the array and performing if statements 
	// the [=] prefix means "capture by value", other options include [] "capture nothing" and [&] "capture by reference"
	//return nullptr == Inventory.FindByPredicate([=](ASWeapon* W){ return W->GetStorageSlot() == CheckSlot; });
}


*/

void AFusionCharacter::Suicide()
{
	KilledBy(this);
}


void AFusionCharacter::KilledBy(class APawn* EventInstigator)
{
	if (Role == ROLE_Authority && !bIsDying)
	{
		AController* Killer = nullptr;
		if (EventInstigator != nullptr)
		{
			Killer = EventInstigator->Controller;
			LastHitBy = nullptr;
		}

		Die(Health, FDamageEvent(UDamageType::StaticClass()), Killer, nullptr);
	}
}

/*
void AFusionCharacter::SwapToNewWeaponMesh()
{
	if (PreviousWeapon)
	{
		PreviousWeapon->AttachMeshToPawn(PreviousWeapon->GetStorageSlot());
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->AttachMeshToPawn(EEquippedWeaponTypes::WeaponAttachPoint);
	}
}
*/


void AFusionCharacter::SetSprinting(bool NewSprinting)
{
	if (bWantsToSprint)
	{
		//StopWeaponFire();
	}

	Super::SetSprinting(NewSprinting);
}

void AFusionCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFusionCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

float AFusionCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	
	//Decrease the character's hp 

	//Health -= Damage;
	//if (Health <= 0) InitHealth();

	//Call the update text on the local client
	//OnRep_Health will be called in every other client so the character's text
	//will contain a text with the right values
	//UpdateCharText();

	return ActualDamage;
}

void AFusionCharacter::AttemptToFire()
{
	if (Ammo > 1)
	{
		//If we don't have authority, meaning that we're not the server
		//tell the server to spawn the bomb.
		//If we're the server, just spawn the bomb - we trust ourselves.
		if (Role < ROLE_Authority)
		{
			ServerOnFire();
		}
		else OnFire();

		
		//todo: this code will be removed in the next part // I NEED TO SET UP SOME OTHER FUNCTIONALITY HERE
		FDamageEvent DmgEvent;

		if (Role < ROLE_Authority)
		{
			ServerTakeDamage(25.f, DmgEvent, GetController(), this);
		}
		else TakeDamage(25.f, DmgEvent, GetController(), this);
	}
}

void AFusionCharacter::ServerOnFire_Implementation()
{
	OnFire();
}

bool AFusionCharacter::ServerOnFire_Validate()
{
	return true;
}


