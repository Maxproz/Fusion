// @Maxpro 2017

#include "Fusion.h"

#include "FusionHUD.h"

#include "FusionPlayerController_Lobby.h"

#include "Widgets/Menus/PasswordEnterPopup_Widget.h"

#include "Runtime/Engine/Classes/Kismet/KismetStringLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetTextLibrary.h"

#include "ServerMenuStats_Widget.h"

#define LOCTEXT_NAMESPACE "Fusion.HUD.Menu"

void UServerMenuStats_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	// Do 7 actions in a row

	// NUmber of the Servers in the List
	NumberText->SetText(FText::AsNumber(Index + 1));

	// Setting The Server Name
	ServerNameTextBox->SetText(FText::FromString(CustomResult.SessionName));

	// Set The Network Whether Lan or Online
	FText OutLanText;
	IsLan(CustomResult.bIsLan, OutLanText);
	NetworkTextBox->SetText(OutLanText);

	// Setting the Current/Max Number of Players
	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("CurrentNumberOfPlayers"), FText::AsNumber(CustomResult.CurrentNumberOfPlayers));
	Arguments.Add(TEXT("MaxNumberOfPlayers"), FText::AsNumber(CustomResult.MaxNumberOfPlayers));
	FText TextToSet = FText::Format(LOCTEXT("Fusion.HUD.Menu", "{CurrentNumberOfPlayers}/{MaxNumberOfPlayers}"), Arguments);
	NumberOfPlayersTextbox->SetText(TextToSet);

	// Set The Ping in the Server Display (one time ststic till now)
	PingTextBox->SetText(FText::AsNumber(CustomResult.Ping));

	if (CustomResult.bIsPasswordProtected)
	{
		LockImage->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		LockImage->SetVisibility(ESlateVisibility::Hidden);
	}

	// Disable The Button if the Session is Full so no players can join
	if (CustomResult.CurrentNumberOfPlayers >= CustomResult.MaxNumberOfPlayers)
	{
		ServerBrowserITemButton->SetIsEnabled(false);
	}


	ServerBrowserITemButton->OnClicked.AddDynamic(this, &UServerMenuStats_Widget::OnClickedServerBrowserITemButton);
}

// Make the Player Join the Session Through GameInstance using the Index in the Array if the game is not password protected else show the password window popup
void UServerMenuStats_Widget::OnClickedServerBrowserITemButton()
{
	if (CustomResult.bIsPasswordProtected)
	{
		AFusionPlayerController_Lobby* LPC = Cast<AFusionPlayerController_Lobby>(GetOwningPlayer());
		if (LPC)
		{
			UPasswordEnterPopup_Widget* PasswordEnterPopup_Widget = CreateWidget<UPasswordEnterPopup_Widget>(LPC, LPC->GetFusionHUD()->PasswordEnterPopup_WidgetTemplate);
			PasswordEnterPopup_Widget->SetMaxPasswordLength(50);
			PasswordEnterPopup_Widget->SetPassword(CustomResult.SessionPassword);
			PasswordEnterPopup_Widget->SetGameInstanceRef(GameInstanceRef);

			PasswordEnterPopup_Widget->AddToViewport();
		}
	}
	else
	{
		GameInstanceRef->JoinOnlineGame(Index);
	}

}

void UServerMenuStats_Widget::IsLan(const bool bIsLanBook, FText& OutIsLanText)
{
	if (bIsLanBook)
	{
		OutIsLanText = FText::FromString(TEXT("Lan"));
	}
	else
	{
		OutIsLanText = FText::FromString(TEXT("Online"));
	}
}

void UServerMenuStats_Widget::SessionNameToText(const FString SessionString, FText& SessionText)
{
	FString LocalSessionString = SessionString;

	if (UKismetStringLibrary::Len(LocalSessionString) > MaxRoomNameLength)
	{
		FString SubStr = UKismetStringLibrary::GetSubstring(LocalSessionString, 0, MaxRoomNameLength);
		SessionText = FText::FromString(SubStr);
	}
	else
	{
		int32 LastIndex = MaxRoomNameLength - UKismetStringLibrary::Len(LocalSessionString); 
		for (int32 FirstIndex = 0; FirstIndex != LastIndex; FirstIndex++) // Is this right? I think it is.
		{
			LocalSessionString = UKismetStringLibrary::Concat_StrStr(LocalSessionString, " ");
		}

		SessionText = FText::FromString(LocalSessionString);
	}
}



#undef LOCTEXT_NAMESPACE

