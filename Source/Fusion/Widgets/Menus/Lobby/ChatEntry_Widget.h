// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/MasterWidget.h"
#include "ChatEntry_Widget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UChatEntry_Widget : public UMasterWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	void SplitPlayerNameAndChatMessage(FText& PlayerName, FText& ChatMessage);

protected:
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameTextBlock;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ChatEntryTextblock;

	FText ChatEntryText;

};
