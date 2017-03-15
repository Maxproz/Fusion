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

	void UpdateDynamicInfo(const float InDeltaTime);
	
	void ShowInGameHUD();

	void HideInGameHUD();

	void UpdatePlayerHealthBar(float Health);

	FORCEINLINE bool IsHealthBarValid() const { return HealthBar != nullptr; }

protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

};
