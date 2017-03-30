// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/MasterWidget.h"
#include "MainMenuUI.generated.h"


class UEditableTextBox;
class UCheckBox;
class UButton;
class UWidgetSwitcher;
class USpinBox;

/**
 * 
 */
UCLASS()
class FUSION_API UMainMenuUI : public UMasterWidget
{
	GENERATED_BODY()
	
	
public:


	/** pointer to our owner PC */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

	
	virtual void NativeConstruct() override;

	void DisplayLoadingScreen();

	void GetADefaultGameName(FText& OutName);

	void LimitTextBoxText(const FText InText, const int32 MaxTextSize, FString& OutString);

	void SetIsPasswordProtected(const bool bHasPassword) { bDoesServerHavePassword = bHasPassword; }

	FORCEINLINE bool GetIsPasswordProtected() const { return bDoesServerHavePassword; }

	void SetIsLan(const bool bIsLan) { bIsItLan = bIsLan; }

	FORCEINLINE bool GetIsLan() const { return bIsItLan; }

	/* function events bound to our button presses */
	UFUNCTION()
	void OnClickedBackFromHostingButton();

	UFUNCTION()
	void OnClickedFindGameButton();
	
	UFUNCTION()
	void OnClickedSettingsButton();

	UFUNCTION()
	void OnClickedHostGameButton();

	UFUNCTION()
	void OnClickedLeaderboardsButton();

	UFUNCTION()
	void OnClickedStartHostingButton();

	UFUNCTION()
	void OnClickedQuitButton();

	UFUNCTION()
	void OnValueChangedMaxPlayersSlider(float InValue);

	UFUNCTION()
	void OnTextChangedRoomNameTextbox(const FText &Text);

	UFUNCTION()
	void OnTextCommittedRoomNameTextbox(const FText &Text, ETextCommit::Type Method);

	UFUNCTION()
	void OnCheckStateChangedPasswordCheckBox(bool IsChecked);

	UFUNCTION()
	void OnTextChangedPasswordTextBox(const FText &Text);

	UFUNCTION()
	void OnTextCommittedPasswordTextBox(const FText &Text, ETextCommit::Type Method);

	UFUNCTION()
	void OnCheckStateChangedIsLanCheckBox(bool IsChecked);

	int32 MinServerNameLength;

	int32 MaxServerNameLength;

	int32 MaxPasswordSizeServer;

	class UFusionGameInstance* GameInstanceRef;

protected:
	
	UPROPERTY(meta = (BindWidget))
	UButton* BackFromHostingButton;

	UPROPERTY(meta = (BindWidget))
	UButton* FindGameButton;

	UPROPERTY(meta = (BindWidget))
	UButton* HostGameButton;

	UPROPERTY(meta = (BindWidget))
	UButton* LeaderboardsButton;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* IsLanCheckBox;
	
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* MainMenuWidgetSwitcher;

	UPROPERTY(meta = (BindWidget))
	USpinBox* MaxPlayersSlider;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* PasswordCheckBox;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PasswordTextBox;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* RoomNameTextbox;

	UPROPERTY(meta = (BindWidget))
	UButton* SettingButton;

	UPROPERTY(meta = (BindWidget))
	UButton* StartHostingButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;


	class AFusionHUD* PlayerHUDRef;

	FString LanPlayerCurrentName;

	bool bIsItLan;

	int32 MaxNumOfPlayers;

	FString TheServerName;



	bool bDoesServerHavePassword;

	FString TheSessionPassword;

	

	

};
