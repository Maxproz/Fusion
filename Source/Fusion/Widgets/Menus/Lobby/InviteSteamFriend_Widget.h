// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/MasterWidget.h"
#include "InviteSteamFriend_Widget.generated.h"


/**
 * 
 */
UCLASS()
class FUSION_API UInviteSteamFriend_Widget : public UMasterWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedSteamFriendInfoButton();

	void SetSteamFriendInfo(FSteamFriendInfo Info) { SteamFriendInfo = Info; }

	void SetLobbyMenuWidgetRef(class ULobbyMenu_Widget* InLobbyWidget) { LobbyMenu_WidgetRef = InLobbyWidget; }

	void SetGameInstanceRef(class UFusionGameInstance* InGameInstanceRef) { GameInstanceRef = InGameInstanceRef; }

protected:

	UPROPERTY(meta = (BindWidget))
	UImage* SteamFriendAvatarImage;

	UPROPERTY(meta = (BindWidget))
	UButton* SteamFriendInfoButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SteamFriendNameTextBlock;

	FSteamFriendInfo SteamFriendInfo;

	class ULobbyMenu_Widget* LobbyMenu_WidgetRef;

	class UFusionGameInstance* GameInstanceRef;

};