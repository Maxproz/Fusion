// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "FusionFullBody.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UFusionFullBody : public UAnimInstance
{
	GENERATED_BODY()
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:

	/*Updates the above properties*/
	UFUNCTION(BlueprintCallable, Category = "UpdateAnimationProperties")
	void UpdateAnimationProperties(float DeltaTime);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	bool bIsReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	bool bIsZooming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	bool bIsSprinting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	bool bIsWalking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	bool bIsJumping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	bool bIsCrouched = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	float AimPitch = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	float AimYaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	float Speed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	float Direction = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	float JumpTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Reference")
	class AFusionCharacter* Player = nullptr;

public:

	UPROPERTY(EditAnywhere, Category = "Montages")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = "Montages")
	UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, Category = "Montages")
	UAnimMontage* ZoomFireMontage;



};
