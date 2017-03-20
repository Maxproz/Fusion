// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/MasterWidget.h"
#include "LobbyMenu_Widget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API ULobbyMenu_Widget : public UMasterWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;


	UFUNCTION()
	void OnTextCommittedChatBoxEditableTextbox(const FText &Text, ETextCommit::Type Method);

	/* function events bound to our button presses */
	UFUNCTION()
	void OnClickedLeaveButton();

	UFUNCTION()
	void OnClickedInvitePlayerButton();

	UFUNCTION()
	void OnClickedKickPlayerButton();

	UFUNCTION()
	void OnClickedCloseInviteSteamFriendButton();

	UFUNCTION()
	void OnClickedStartGameButton();

	UFUNCTION()
	void OnClickedReadyButton();

	// This is currently being set after creating the game widgets inside of the LobbyPlayerController.
	void SetLobbyPlayerControllerRef(AFusionPlayerController_Lobby* InPlayerController) { LobbyPlayerControllerRef = InPlayerController; }

	// same as player controller
	void SetGameInstanceRef(class UFusionGameInstance* InGameInstance) { GameInstanceRef = InGameInstance; }

	/* Helper Functions */
	void IsSessionFull(bool& bOutResult);

	UFUNCTION(BlueprintPure, Category = BlueprintBindings)
	void NumberOfPlayersBinding(FText& OutReturnText);

protected:

	// creates a chat entry and adds it to the scrollbox
	DECLARE_EVENT_OneParam(ULobbyMenu_Widget, FOnUpdatePlayerList, TArray<FLobbyPlayerInfo> /*Result*/);
	FOnUpdatePlayerList OnUpdatePlayerListEvent;

	// Delegate fired when updating player list
	void OnUpdatePlayerList(TArray<FLobbyPlayerInfo> PlayerInfoArray);

	// creates a chat entry and adds it to the scrollbox
	DECLARE_EVENT_OneParam(ULobbyMenu_Widget, FOnReceiveChatMessageComplete, FText /*Result*/);
	FOnReceiveChatMessageComplete ReceiveChatMessageCompleteEvent;

	// Delegate fired when recieving a chat msg has completed
	void OnReceiveChatMessageComplete(FText ChatMessage);

	// creates a chat entry and adds it to the scrollbox
	DECLARE_EVENT_OneParam(ULobbyMenu_Widget, FGetSteamFriendRequestCompleted, TArray<FSteamFriendInfo> /*Result*/);
	FGetSteamFriendRequestCompleted GetSteamFriendRequestCompletedEvent;

	// Delegate fired when recieving a chat msg has completed
	void OnGetSteamFriendRequestCompleted(TArray<FSteamFriendInfo> FriendsList);


	
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ChatboxEditableTextBox;
	
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatEntriesScrollBox;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseInviteSteamFriendButton;

	UPROPERTY(meta = (BindWidget))
	UButton* InvitePlayerButton;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* InviteSteamFreindsListScrollBox;

	UPROPERTY(meta = (BindWidget))
	UOverlay* InviteSteamFriendListOverlay;

	UPROPERTY(meta = (BindWidget))
	UButton* KickPlayerButton;

	UPROPERTY(meta = (BindWidget))
	UButton* LeaveButton;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerListScrollBox;

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* ReadyAndStartGameWidgetSwitcher_0;

	UPROPERTY(meta = (BindWidget))
	UButton* ReadyButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReadyButtonTextBlock;

	UPROPERTY(meta = (BindWidget))
	UButton* SettingsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* StartGameButton;
	
	class AFusionPlayerController_Lobby* LobbyPlayerControllerRef;

	class UFusionGameInstance* GameInstanceRef;

	ESlateVisibility KickingPlayers;

	TArray<UButton*> KickButtons;

	bool bIsReadyState;

public:

	/** @return the delegate fired when receiving a chat msg */
	FOnReceiveChatMessageComplete& OnReceiveChatMessageComplete() { return ReceiveChatMessageCompleteEvent; }

	/** @return the delegate fired when updating player list */
	FOnUpdatePlayerList& OnUpdatePlayerList() { return OnUpdatePlayerListEvent; }

	/** @return the delegate fired when updating player list */
	FGetSteamFriendRequestCompleted& GetSteamFriendRequestCompleted() { return GetSteamFriendRequestCompletedEvent; }

};
