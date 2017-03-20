// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/MasterWidget.h"

#include "Online/FusionGame_Lobby.h"

#include "PlayerInfoEntry_Widget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UPlayerInfoEntry_Widget : public UMasterWidget
{
	GENERATED_BODY()
	

public:

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnCheckStateChangedIdsReadyCheckBox(bool IsChecked);

	UFUNCTION()
	void OnClickedKickButton();

	void SetLobbyPlayerInfo(FLobbyPlayerInfo InPlayerLobbyInfo) { PlayerLobbyInfo = InPlayerLobbyInfo; }

	void SetPlayerIndex(int32 InPlayerIndex) { PlayerIndex = InPlayerIndex; }

	UPROPERTY(meta = (BindWidget))
	UCheckBox* IdsReadyCheckBox;

	UPROPERTY(meta = (BindWidget))
	UButton* KickButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameTextBlock;

protected:

	UPROPERTY(BlueprintReadWrite, Category = BlueprintBinding)
	FLobbyPlayerInfo PlayerLobbyInfo;

	int32 PlayerIndex;

};
