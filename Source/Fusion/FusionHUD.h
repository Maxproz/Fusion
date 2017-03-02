// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "FusionHUD.generated.h"


UENUM(BlueprintType)
enum class EHUDState : uint8
{
	Playing,
	Spectating,
	MatchEnd
};

UCLASS()
class AFusionHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFusionHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	EHUDState GetCurrentState();

	/* Event hook to update HUD state (eg. to determine visibility of widgets) */
	UFUNCTION(BlueprintNativeEvent, Category = "HUDEvents")
	void OnStateChanged(EHUDState NewState);

	/* An event hook to call HUD text events to display in the HUD. Blueprint HUD class must implement how to deal with this event. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUDEvents")
	void MessageReceived(const FText& TextMessage);

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

	/* Current HUD state */
	EHUDState CurrentState;
	

};

