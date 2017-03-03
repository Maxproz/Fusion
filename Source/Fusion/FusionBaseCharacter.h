// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"

#include "Types/TakeHitInfo.h"

#include "FusionBaseCharacter.generated.h"

UCLASS(ABSTRACT)
class FUSION_API AFusionBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFusionBaseCharacter(const class FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* SoundTakeHit = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* SoundDeath = nullptr;

	/************************************************************************/
	/* Health / Shields                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Shields")
	FORCEINLINE float GetMaxShields() const { return GetClass()->GetDefaultObject<AFusionBaseCharacter>()->Shields; }

	UFUNCTION(BlueprintCallable, Category = "Shields")
	FORCEINLINE float GetShields() const { return Shields; }

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Shields")
	FORCEINLINE float GetLastTakeDamageTime() const { return LastTakeDamageTime; }


	/************************************************************************/
	/* Sprinting ~~ Maybe Remove Later                                      */
	/************************************************************************/

	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual bool IsSprinting() const;

	/* Client/local call to update sprint state  */
	virtual void SetSprinting(bool NewSprinting);

	float GetSprintingSpeedModifier() const;

	float GetCrouchingSpeedModifier() const;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float CrouchingSpeedModifier;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintingSpeedModifier = 2.f;

	/* Character wants to run, checked during Tick to see if allowed */
	UPROPERTY(Transient, Replicated)
	bool bWantsToSprint = false;

	/* Server side call to update actual sprint state */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprinting(bool NewSprinting);
	void ServerSetSprinting_Implementation(bool NewSprinting);
	bool ServerSetSprinting_Validate(bool NewSprinting);


	/************************************************************************/
	/* Damage & Death                                                       */
	/************************************************************************/

	/**
	* TakeDamage Server version. Call this instead of TakeDamage when you're a client
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	void ServerTakeDamage_Implementation(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	bool ServerTakeDamage_Validate(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

protected:

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Health")
	float Health = 100.f;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Shields")
	float Shields = 100.f;

	/* Take damage & handle death */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	virtual bool Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser);

	virtual void OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser);

	virtual void FellOutOfWorld(const class UDamageType& DmgType) override;

	void SetRagdollPhysics();

	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled);

	void ReplicateHit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled);

	/* Holds hit data to replicate hits and death to clients */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	FTakeHitInfo LastTakeHitInfo;

	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	bool bIsDying = false;

	// TODO: implement a timer way to detect..
	// "If the player has taken shields damage and has not been damaged in the last 6 seconds, start recharging the shields.

	const float ShieldRechargeTimer = 6.f;

	float LastTakeDamageTime = 0.f;

	// The player's Shield Recharge Rate
	float ShieldsRechargeRate = 3.33f;

	uint32 Recharge = 0;

	const uint32 StartRecharging = 6;

	void ForceShieldsCap();

	void RechargeShields();

	void ForceHealthCap();

	void RestoreHealth(float HealthRestored);
};

