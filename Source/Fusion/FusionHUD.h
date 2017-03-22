// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "FusionHUD.generated.h"


class UInGameHUD;
class UMainMenuUI;
class UServerMenu_Widget;
class UOkErrorMessage_Widget;
class ULobbyMenu_Widget;


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

	// Used By the lobby widgets 
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UPlayerInfoEntry_Widget> PlayerInfoEntry_WidgetTemplate;
	UPROPERTY(EditDefaultsOnly, Category = "Chat Widgets")
	TSubclassOf<class UChatEntry_Widget> ChatEntry_WidgetTemplate;
	UPROPERTY(EditDefaultsOnly, Category = "Lobby Widgets")
	TSubclassOf<class UInviteSteamFriend_Widget> InviteSteamFriend_WidgetTemplate;
	UPROPERTY(EditDefaultsOnly, Category = "Lobby Widgets")
	TSubclassOf<class UPasswordEnterPopup_Widget> PasswordEnterPopup_WidgetTemplate;
	UPROPERTY(EditDefaultsOnly, Category = "Lobby Widgets")
	TSubclassOf<class UServerMenuStats_Widget> ServerMenuStats_WidgetTemplate;
	UPROPERTY(EditDefaultsOnly, Category = "Error Widgets")
	TSubclassOf<class UOkErrorMessage_Widget> UOkErrorMessage_WidgetTemplate;


	FORCEINLINE UInGameHUD* GetInGameHUDWidget() const { return ActiveInGameHUDWidget; }
	FORCEINLINE UMainMenuUI* GetMainMenuUIWidget() const { return ActiveMainMenuUIWidget; }
	FORCEINLINE UServerMenu_Widget* GetServerMenuWidget() const { return ActiveServerMenuWidget; }
	FORCEINLINE UOkErrorMessage_Widget* GetErrorMessageWidget() const { return ActiveErrorMessageWidget; }
	FORCEINLINE ULobbyMenu_Widget* GetLobbyMenuWidget() const { return ActiveLobbyMenuWidget; }

protected:
	//////////////////////////////////
	///// Game Widget Management /////
	//////////////////////////////////

	// Widget Classes
	UPROPERTY(EditDefaultsOnly, Category = "InGame Widgets")
	TAssetSubclassOf<UInGameHUD> InGameHUDWidget;
	UPROPERTY(EditDefaultsOnly, Category = "Menu Widgets")
	TAssetSubclassOf<UMainMenuUI> MainMenuUIWidget;
	UPROPERTY(EditDefaultsOnly, Category = "Menu Widgets")
	TAssetSubclassOf<UServerMenu_Widget> ServerMenuWidget;
	UPROPERTY(EditDefaultsOnly, Category = "Menu Widgets")
	TAssetSubclassOf<UOkErrorMessage_Widget> ErrorMessageWidget;
	UPROPERTY(EditDefaultsOnly, Category = "Lobby Widgets")
	TAssetSubclassOf<ULobbyMenu_Widget> LobbyMenuWidget;


	// Active Widgets
	UInGameHUD* ActiveInGameHUDWidget;
	UMainMenuUI* ActiveMainMenuUIWidget;
	UServerMenu_Widget* ActiveServerMenuWidget;
	UOkErrorMessage_Widget* ActiveErrorMessageWidget;
	ULobbyMenu_Widget* ActiveLobbyMenuWidget;


	void CreateInGameHUDWidget();
	void CreateMainMenuUIWidget();
	void CreateServerMenuWidget();
	void CreateErrorMessageWidget();
	void CreateLobbyMenuWidget();


public:

	// Functions
	void CreateGameWidgets();
	void RemoveGameWidgets();

	void ShowMainMenu(); 
	void HideMainMenu();

	void ShowServerMenu();
	void HideServerMenu();

	void ShowLobbyMenu();
	void HideLobbyMenu();




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

