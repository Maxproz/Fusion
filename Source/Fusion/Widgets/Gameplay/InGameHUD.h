// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/MasterWidget.h"
#include "InGameHUD.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UInGameHUD : public UMasterWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	void UpdateDynamicInfo(const float InDeltaTime, float Shields, float Health);
	
	void ShowInGameHUD();

	void HideInGameHUD();

	UFUNCTION(BlueprintPure, Category = Bindings)
	float UpdatePlayerHealthBar() const;

	UFUNCTION(BlueprintPure, Category = Bindings)
	float UpdatePlayerShieldBar() const;

	FORCEINLINE bool IsHealthBarValid() const { return HealthBar != nullptr; }

	FORCEINLINE bool IsShieldBarValid() const { return ShieldBar != nullptr; }

	UPROPERTY()
	class AFusionPlayerController* PlayerControllerRef;

protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

};
