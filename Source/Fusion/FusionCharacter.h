// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "FusionBaseCharacter.h"

#include "Player/EquippedWeaponTypes.h"

#include "FusionCharacter.generated.h"


namespace InventoryTypes
{
	
#define MAX_INVENTORY_SIZE = 2;

}


class UInputComponent;

UCLASS(config=Game)
class AFusionCharacter : public AFusionBaseCharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	//UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	//class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	//UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	//class USceneComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	void StopAllAnimMontages();

public:
	AFusionCharacter(const class FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay();

	/* Called every frame */
	virtual void Tick(float DeltaSeconds) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PawnClientRestart() override;

	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	virtual void PossessedBy(class AController* InController) override;

	virtual void PostInitializeComponents() override;

	virtual void OnRep_PlayerState() override;

protected:



	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:


	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;


	/************************************************************************/
	/* MOVEMENT                                                       */
	/************************************************************************/
protected:

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float Rate);


	/** material instances for setting team color in mesh (3rd person view) */
	UPROPERTY(VisibleInstanceOnly, Transient)
	TArray<UMaterialInstanceDynamic*> MeshMIDs;

	UPROPERTY(VisibleInstanceOnly)
	UMaterialInstanceDynamic* Mesh1PMID;

	UPROPERTY(VisibleInstanceOnly, Transient)
	UMaterialInstanceDynamic* Mesh3rdPMID;

	

	/** handle mesh visibility and updates */
	void UpdatePawnMeshes();

	/** handle mesh colors on specified material instance */
	void UpdateTeamColors(UMaterialInstanceDynamic* UseMID);
	


public:

	/** Update the team color of all player meshes. */
	void UpdateTeamColorsAllMIDs();

	void Update3rdPersonMeshColor();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/* Client mapped to Input */
	void OnJump();

	/* Client mapped to Input */
	void OnStartSprinting();

	/* Client mapped to Input */
	void OnStopSprinting();

	virtual void SetSprinting(bool NewSprinting) override;

	/* Is character currently performing a jump action. Resets on landed.  */
	UPROPERTY(Transient, Replicated)
	bool bIsJumping;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsInitiatedJump() const;

	void SetIsJumping(bool NewJumping);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSetIsJumping(bool NewJumping);
	void ServerSetIsJumping_Implementation(bool NewJumping);
	bool ServerSetIsJumping_Validate(bool NewJumping);


	/* Client mapped to Input */
	void OnCrouchToggle();


	/************************************************************************/
	/* Object Interaction                                                   */
	/************************************************************************/


	/* Use the usable actor currently in focus, if any */
	UFUNCTION()
	virtual void Use();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUse();
	void ServerUse_Implementation();
	bool ServerUse_Validate();

	class AMasterPickupActor* GetPickupInView();

	/*Max distance to use/focus on actors. */
	UPROPERTY(EditDefaultsOnly, Category = "ObjectInteraction")
	float MaxUseDistance = 1000; 

	/* True only in first frame when focused on a new usable actor. */
	bool bHasNewFocus = true;

	class AMasterPickupActor* FocusedPickupActor = nullptr;



	/************************************************************************/
	/* Damage & Death                                                       */
	/************************************************************************/

	virtual void OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser) override;

	virtual void Suicide();

	virtual void KilledBy(class APawn* EventInstigator);


	/************************************************************************/
	/* Weapons & Inventory                                                  */
	/************************************************************************/

private:

	/* Attachpoint for Primary active weapon in hands */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName WeaponAttachPoint;

	/* Attachpoint for primary weapons */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName BackAttachPoint;

	bool bWantsToFire;

	/* Distance away from character when dropping inventory items. */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	float DropWeaponMaxDistance;

	void OnReload();

	/* Mapped to input */
	void OnStartFire();

	/* Mapped to input */
	void OnStopFire();

	/* Mapped to input */
	void OnNextWeapon();

	/* Mapped to input */
	void OnPrevWeapon();

	/* Mapped to input */ // TODO: Needs implemented
	void OnSwapWeapon();

	/* Mapped to input */
	void OnEquipPrimaryWeapon();

	/* Mapped to input */
	void OnEquipSecondaryWeapon();

	void StartWeaponFire();

	void StopWeaponFire();

	void DestroyInventory();

	/* Mapped to input. Drops current weapon */
	void DropWeapon();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerDropWeapon();
	void ServerDropWeapon_Implementation();
	bool ServerDropWeapon_Validate();
	
public:
	// TODO: Should this be replicated?

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool bIsZooming = false;

	bool bIsReloading = false;

	// Check if pawn is allowed to fire weapon 
	bool CanFire() const;

	bool CanReload() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool IsFiring() const;

	/* Check if the specified slot is available, limited to one item per type (primary, secondary) */
	bool WeaponSlotAvailable(EEquippedWeaponTypes CheckSlot);

	/* Return socket name for attachments (to match the socket in the character skeleton) */
	FName GetInventoryAttachPoint(EEquippedWeaponTypes WeaponSlot) const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	class AMasterWeapon* GetCurrentWeapon() const;

	/* OnRep functions can use a parameter to hold the previous value of the variable. Very useful when you need to handle UnEquip etc. */
	UFUNCTION()
	void OnRep_CurrentWeapon(class AMasterWeapon* LastWeapon);

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	class AMasterWeapon* CurrentWeapon;
	class AMasterWeapon* PreviousWeapon;

	// uses optional second parameter
	void SetCurrentWeapon(class AMasterWeapon* NewWeapon, class AMasterWeapon* LastWeapon = nullptr);

	/* It will just be 2 slots for the primary and secondary weapons for now. */
	UPROPERTY(Transient, Replicated)
	TArray<class AMasterWeapon*> Inventory;

	void EquipWeapon(class AMasterWeapon* Weapon);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerEquipWeapon(class AMasterWeapon* Weapon);
	void ServerEquipWeapon_Implementation(class AMasterWeapon* Weapon);
	bool ServerEquipWeapon_Validate(class AMasterWeapon* Weapon);

	void AddWeapon(class AMasterWeapon* Weapon);

	void RemoveWeapon(class AMasterWeapon* Weapon, bool bDestroy);

	/* Update the weapon mesh to the newly equipped weapon, this is triggered during an anim montage.
	NOTE: Requires an AnimNotify created in the Equip animation to tell us when to swap the meshes. */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SwapToNewWeaponMesh();

protected:
	
	/** Fires a projectile. */
	//void OnFire();

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }


private:
	/**
	* OnFire Server version. Call this instead of OnFire when you're a client
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerOnFire();
	void ServerOnFire_Implementation();
	bool ServerOnFire_Validate();
	
	UFUNCTION()
	void AttemptToFire();
	*/
	

	//float TimeSinceLastHit = 0.f;
	//MyPlayerState->ManaRegen();  // start the refresh tick

public:
	/** Applies damage to the character */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	FORCEINLINE bool IsFirstPerson() const { return IsAlive() && Controller && Controller->IsLocalPlayerController(); }
	
		
	

};

