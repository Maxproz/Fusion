// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameLoadingScreen.h"

#include "FusionGameInstance.h"
#include "FusionGameSession.h"

#include "FusionHUD.h"

#include "FusionPlayerController_Menu.h"

#include "Online/FusionOnlineGameSettings.h"

#include "Runtime/Engine/Classes/Kismet/KismetStringLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"


#include "MainMenuUI.h"



#define LOCTEXT_NAMESPACE "Fusion.HUD.Menu"


void UMainMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	BackFromHostingButton->OnClicked.AddDynamic(this, &UMainMenuUI::OnClickedBackFromHostingButton);
	FindGameButton->OnClicked.AddDynamic(this, &UMainMenuUI::OnClickedFindGameButton);
	HostGameButton->OnClicked.AddDynamic(this, &UMainMenuUI::OnClickedHostGameButton);
	StartHostingButton->OnClicked.AddDynamic(this, &UMainMenuUI::OnClickedStartHostingButton);
	QuitButton->OnClicked.AddDynamic(this, &UMainMenuUI::OnClickedQuitButton);
	MaxPlayersSlider->OnValueChanged.AddDynamic(this, &UMainMenuUI::OnValueChangedMaxPlayersSlider);
	RoomNameTextbox->OnTextChanged.AddDynamic(this, &UMainMenuUI::OnTextChangedRoomNameTextbox);
	RoomNameTextbox->OnTextCommitted.AddDynamic(this, &UMainMenuUI::OnTextCommittedRoomNameTextbox);
	PasswordTextBox->OnTextChanged.AddDynamic(this, &UMainMenuUI::OnTextChangedPasswordTextBox);
	PasswordTextBox->OnTextCommitted.AddDynamic(this, &UMainMenuUI::OnTextCommittedPasswordTextBox);
	PasswordCheckBox->OnCheckStateChanged.AddDynamic(this, &UMainMenuUI::OnCheckStateChangedPasswordCheckBox);
	IsLanCheckBox->OnCheckStateChanged.AddDynamic(this, &UMainMenuUI::OnCheckStateChangedIsLanCheckBox);
	

	PlayerOwner = GetOwningLocalPlayer();
	PlayerHUDRef = Cast<AFusionPlayerController_Menu>(PlayerOwner->GetPlayerController(GetWorld()))->GetFusionHUD();

}

void UMainMenuUI::ShowMainMenu()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UMainMenuUI::HideMainMenu()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UMainMenuUI::DisplayLoadingScreen()
{
	IFusionGameLoadingScreenModule* LoadingScreenModule = FModuleManager::LoadModulePtr<IFusionGameLoadingScreenModule>("FusionGameLoadingScreen");
	if (LoadingScreenModule != NULL)
	{
		LoadingScreenModule->StartInGameLoadingScreen();
	}
}

void UMainMenuUI::GetADefaultGameName(FText& OutName)
{
	FString OwningPlayerName = GetOwningPlayer()->PlayerState->GetName();
	int32 OwnerNameLen = UKismetStringLibrary::Len(OwningPlayerName);
	int32 Len = MaxServerNameLength - 7;

	if (OwnerNameLen <= Len)
	{
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("PlayerName"), FText::FromString(OwningPlayerName));
		OutName = FText::Format(LOCTEXT("Fusion.HUD.Menu", "{PlayerName}'s Game"), Arguments);
	}
	else
	{
		FString TextToText = UKismetStringLibrary::GetSubstring(OwningPlayerName, 0, Len);

		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("PlayerName"), FText::FromString(TextToText));
		OutName = FText::Format(LOCTEXT("Fusion.HUD.Menu", "{PlayerName}'s Game"), Arguments);
	}
}

// Make the Widget switch show the menu when back is clicked
void UMainMenuUI::OnClickedBackFromHostingButton()
{
	MainMenuWidgetSwitcher->SetActiveWidgetIndex(0);
}

// if the player clicked to find game, show the server menu and remove this widget from the viewport
void UMainMenuUI::OnClickedFindGameButton()
{
	MainMenuWidgetSwitcher->SetActiveWidgetIndex(0);
	//GameInstanceRef->ShowServerMenu();
	//PlayerHUDRef->GetMainMenuWidget()->HideMainMenu();
}

void UMainMenuUI::OnClickedHostGameButton()
{
	MainMenuWidgetSwitcher->SetActiveWidgetIndex(1);
	FText OutGeneratedGameName;
	GetADefaultGameName(OutGeneratedGameName);

	RoomNameTextbox->SetText(OutGeneratedGameName);
}

void UMainMenuUI::OnClickedStartHostingButton()
{
	GameInstanceRef->StartOnlineGame(TheServerName, MaxNumOfPlayers, bIsItLan, true, bDoesServerHavePassword, TheSessionPassword);
}

void UMainMenuUI::OnClickedQuitButton()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit);
}

void UMainMenuUI::OnValueChangedMaxPlayersSlider(float InValue)
{
	MaxNumOfPlayers = UKismetMathLibrary::FTrunc(InValue);
}

void UMainMenuUI::OnTextChangedRoomNameTextbox(const FText &Text)
{

}

void UMainMenuUI::OnTextCommittedRoomNameTextbox(const FText &Text, ETextCommit::Type Method)
{

}

void UMainMenuUI::OnCheckStateChangedPasswordCheckBox(bool IsChecked)
{

}

void UMainMenuUI::OnTextChangedPasswordTextBox(const FText &Text)
{

}

void UMainMenuUI::OnTextCommittedPasswordTextBox(const FText &Text, ETextCommit::Type Method)
{

}

void UMainMenuUI::OnCheckStateChangedIsLanCheckBox(bool IsChecked)
{

}


#undef LOCTEXT_NAMESPACE



