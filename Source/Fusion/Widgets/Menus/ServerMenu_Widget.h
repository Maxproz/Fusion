// @Maxpro 2017

#pragma once

#include "Widgets/MasterWidget.h"

#include "FusionGameInstance.h"

#include "ServerMenu_Widget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UServerMenu_Widget : public UMasterWidget
{
	GENERATED_BODY()
	
	
public:

	virtual void NativeConstruct() override;
	
	class UFusionGameInstance* GameInstanceRef;

	UFUNCTION()
	void OnTextChangedLanPlayerNameTextbox(const FText &Text);

	UFUNCTION()
	void OnTextCommittedLanPlayerNameTextbox(const FText &Text, ETextCommit::Type Method);
	
	UFUNCTION()
	void OnClickedIsLanButton();

	UFUNCTION()
	void OnClickedSearchButton();

	UFUNCTION()
	void OnClickedBackButton();

	UFUNCTION()
	void OnClickedBackButton2();

	UFUNCTION()
	void OnClickedRefreshButton();

protected:
	// creates a chat entry and adds it to the scrollbox
	DECLARE_EVENT_OneParam(ULobbyMenu_Widget, FOnSessionSearchCompleated, TArray<FCustomFusionSessionResult> /*Result*/);
	FOnSessionSearchCompleated SessionSearchCompleatedEvent;

	// Delegate fired whenwhen finishing a session search
	void OnSessionSearchCompleated(TArray<FCustomFusionSessionResult> Results);



	TArray<FCustomFusionSessionResult> CustomSessionResults;

	bool bIsLan;

	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BackButton2;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BackButtonText;

	UPROPERTY(meta = (BindWidget))
	UCircularThrobber* CircularThrobber_0;

	UPROPERTY(meta = (BindWidget))
	UButton* IsLanButton;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* IsLanButtonText;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* LanPlayerNameTextbox;

	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RefreshButtonText;

	UPROPERTY(meta = (BindWidget))
	UButton* SearchButton;
	
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ServerListScrollBox;

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* ServerListWidgetSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* SessionWidgetSwitcher;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextBlock_1;

public:

	/** @return the delegate fired when finishing a session search */
	FOnSessionSearchCompleated& OnSessionSearchCompleated() { return SessionSearchCompleatedEvent; }

};
