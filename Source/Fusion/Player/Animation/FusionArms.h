// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "FusionArms.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UFusionArms : public UAnimInstance
{
	GENERATED_BODY()
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:

	/*Updates the above properties*/
	UFUNCTION(BlueprintCallable, Category = "UpdateAnimationProperties")
	void UpdateAnimationProperties();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	bool bIsSprinting;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	//bool bIsReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	bool bIsZooming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bools)
	bool bIsWalking;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Reference")
	class AFusionCharacter* Player;

public:

	UPROPERTY(EditAnywhere, Category = "Montages")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = "Montages")
	UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, Category = "Montages")
	UAnimMontage* ZoomFireMontage;

	

};
