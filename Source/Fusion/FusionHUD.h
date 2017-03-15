// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "FusionHUD.generated.h"


class UInGameHUD;
class UMainMenuUI;

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

	
	FORCEINLINE UInGameHUD* GetInGameHUDWidget() const { return ActiveInGameHUDWidget; }
	FORCEINLINE UMainMenuUI* GetMainMenuUIWidget() const { return ActiveMainMenuUIWidget; }

protected:
	//////////////////////////////////
	///// Game Widget Management /////
	//////////////////////////////////

	// Widget Classes
	UPROPERTY(EditDefaultsOnly, Category = "InGame Widgets")
	TAssetSubclassOf<UInGameHUD> InGameHUDWidget;
	UPROPERTY(EditDefaultsOnly, Category = "Menu Widgets")
	TAssetSubclassOf<UMainMenuUI> MainMenuUIWidget;

	// Active Widgets
	UInGameHUD* ActiveInGameHUDWidget;
	UMainMenuUI* ActiveMainMenuUIWidget;

	void CreateInGameHUDWidget();
	void CreateMainMenuUIWidget();


public:

	// Functions
	void CreateGameWidgets();
	void RemoveGameWidgets();


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

	/**
	* Set state of current match.
	*
	* @param	NewState	The new match state.
	*/
	void SetMatchState(EHUDState NewState);

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

	/* Current HUD state */
	EHUDState CurrentState;
	

};

