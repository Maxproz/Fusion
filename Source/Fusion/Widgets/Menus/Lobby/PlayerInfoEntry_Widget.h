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

protected:

	UPROPERTY(meta = (BindWidget))
	UCheckBox* IdsReadyCheckBox;
		
	UPROPERTY(meta = (BindWidget))
	UButton* KickButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameTextBlock;

	FLobbyPlayerInfo PlayerLobbyInfo;

	int32 PlayerIndex;

};
