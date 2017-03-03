// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "FusionBaseCharacter.h"

#include "Player/EquippedWeaponTypes.h"

#include "FusionCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AFusionCharacter : public AFusionBaseCharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;


public:
	AFusionCharacter(const class FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay();

	/* Called every frame */
	virtual void Tick(float DeltaSeconds) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PawnClientRestart() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

protected:

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:


	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AFusionProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;


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

public:

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
	/* Damage & Death                                                       */
	/************************************************************************/

	virtual void OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser) override;

	virtual void Suicide();

	virtual void KilledBy(class APawn* EventInstigator);


	/************************************************************************/
	/* Weapons                                                              */
	/************************************************************************/
	
	// Check if pawn is allowed to fire weapon 
	bool CanFire() const;

	bool CanReload() const;
	

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool IsFiring() const;

	/* Attachpoint for Primary active weapon in hands */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName WeaponAttachPoint;

	/* Attachpoint for primary weapons */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName BackAttachPoint;

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


protected:
	
	/** Fires a projectile. */
	void OnFire();

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }


private:
	/**
	* OnFire Server version. Call this instead of OnFire when you're a client
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerOnFire();
	void ServerOnFire_Implementation();
	bool ServerOnFire_Validate();
	
	UFUNCTION()
	void AttemptToFire();

	void StopAllAnimMontages();

	float Ammo = 500;

	//float TimeSinceLastHit = 0.f;
	//MyPlayerState->ManaRegen();  // start the refresh tick

public:
	/** Applies damage to the character */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;




};

