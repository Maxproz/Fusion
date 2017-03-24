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
#include "Runtime/Engine/Classes/Kismet/KismetTextLibrary.h"

#include "MainMenuUI.h"



#define LOCTEXT_NAMESPACE "Fusion.HUD.Menu"


void UMainMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	AFusionPlayerController_Menu* MPC = Cast<AFusionPlayerController_Menu>(GetOwningPlayer());
	PlayerHUDRef = MPC->GetFusionHUD();


	if (!MPC)
	{
		return;
	}

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

void UMainMenuUI::LimitTextBoxText(const FText InText, const int32 MaxTextSize, FString& OutString)
{
	FString InFText = UKismetTextLibrary::Conv_TextToString(InText);
	OutString = UKismetStringLibrary::GetSubstring(InFText, 0, MaxTextSize);
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
	PlayerHUDRef->ShowServerMenu();
	PlayerHUDRef->HideMainMenu();
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
	FString InText = UKismetTextLibrary::Conv_TextToString(Text);
	int32 TextLen = UKismetStringLibrary::Len(InText);

	// if the server name is empty or too short
	if (TextLen >= MinServerNameLength)
	{
		// keep the textbox from exceeding the max server name size
		FString OutLimitedServerName;
		LimitTextBoxText(Text, MaxServerNameLength, OutLimitedServerName);

		TheServerName = OutLimitedServerName;

		FText TheFTextServerName = UKismetTextLibrary::Conv_StringToText(TheServerName);
		
		RoomNameTextbox->SetText(TheFTextServerName);

		// if there is an error remove it then enable the host button since the Server Name is correct
		if (RoomNameTextbox->HasError())
		{
			RoomNameTextbox->ClearError();
		}

		StartHostingButton->SetIsEnabled(true);
	}
	else
	{
		// if the server name is not correct, display an error and disable the hosting button
		FText ErrorMsg = FText::FromString("Name empty or too short");
		RoomNameTextbox->SetError(ErrorMsg);

		StartHostingButton->SetIsEnabled(false);
	}

}

void UMainMenuUI::OnTextCommittedRoomNameTextbox(const FText &Text, ETextCommit::Type Method)
{
	// if the server name is empty or too short
	FString ServerNameToString = UKismetTextLibrary::Conv_TextToString(Text);
	int32 TextLen = UKismetStringLibrary::Len(ServerNameToString);

	if (!(TextLen >= MinServerNameLength))
	{
		// if a correct ServerName was Set
		if (UKismetStringLibrary::Len(TheServerName) > 0)
		{
			// if a correct server name was set , change the text box name to that server name
			RoomNameTextbox->SetText(UKismetTextLibrary::Conv_StringToText(TheServerName));
		}
		else
		{
			// if no correct ServerName was set, set the Textbox to the default name (Players name)'s Game
			FText OutDefaultGameName;
			GetADefaultGameName(OutDefaultGameName);
			RoomNameTextbox->SetText(OutDefaultGameName);
		}

		// set textbox error message
		FText ErrorMsg = FText::FromString(TEXT("Name empty or too short"));
		RoomNameTextbox->SetError(ErrorMsg);
	}
}

void UMainMenuUI::OnCheckStateChangedPasswordCheckBox(bool IsChecked)
{
	// Only Enable the Textbox if the Player Choose to protect the room with a password
	SetIsPasswordProtected(IsChecked);

	if (GetIsPasswordProtected())
	{
		PasswordTextBox->SetIsEnabled(true);
		PasswordTextBox->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		PasswordTextBox->SetIsEnabled(false);
		PasswordTextBox->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainMenuUI::OnTextChangedPasswordTextBox(const FText &Text)
{
	FText EmptyText = FText::FromString(TEXT(""));
	if (UKismetTextLibrary::NotEqual_TextText(Text, EmptyText))
	{
		// keep the textbox from exceeding the max server password size
		FString OutLimitedPasswordName;
		LimitTextBoxText(Text, MaxPasswordSizeServer, OutLimitedPasswordName);

		TheSessionPassword = OutLimitedPasswordName;

		FText TheFTextPasswordName = UKismetTextLibrary::Conv_StringToText(TheSessionPassword);

		PasswordTextBox->SetText(TheFTextPasswordName);

		// if there is an error remove it then enable the host button since the Server Name is correct
		if (RoomNameTextbox->HasError())
		{
			RoomNameTextbox->ClearError();
		}

		StartHostingButton->SetIsEnabled(true);

	}
	else
	{
		// set textbox error message
		FText ErrorMsg = FText::FromString("Password can't be empty");
		PasswordTextBox->SetError(ErrorMsg);

		StartHostingButton->SetIsEnabled(false);
	}
}

void UMainMenuUI::OnTextCommittedPasswordTextBox(const FText &Text, ETextCommit::Type Method)
{
	FText EmptyText = FText::FromString(TEXT(""));

	// if password is not empty
	if (UKismetTextLibrary::NotEqual_TextText(Text, EmptyText))
	{
		// if a correct password was Set
		if (UKismetStringLibrary::Len(TheSessionPassword) > 0)
		{
			// if a correct server name was set , change the text box name to that server name
			PasswordTextBox->SetText(UKismetTextLibrary::Conv_StringToText(TheSessionPassword));
		}
		else
		{
			// if no correct password was set, set the Textbox to the default password (12345)
			FText DefaultPassword = FText::FromString(TEXT("12345"));
			PasswordTextBox->SetText(DefaultPassword);
		}

		PasswordTextBox->SetError(FText::FromString(TEXT("Password can't be empty")));
	}
}

void UMainMenuUI::OnCheckStateChangedIsLanCheckBox(bool IsChecked)
{
	SetIsLan(IsChecked);
}


#undef LOCTEXT_NAMESPACE



