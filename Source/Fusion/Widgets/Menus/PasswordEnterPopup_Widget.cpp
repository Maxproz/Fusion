// @Maxpro 2017

#include "Fusion.h"

#include "FusionGameInstance.h"

#include "Runtime/Engine/Classes/Kismet/KismetStringLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetTextLibrary.h"

#include "PasswordEnterPopup_Widget.h"


void UPasswordEnterPopup_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	BackButton->OnClicked.AddDynamic(this, &UPasswordEnterPopup_Widget::OnClickedBackButton);
	JoinRoomButton->OnClicked.AddDynamic(this, &UPasswordEnterPopup_Widget::OnClickedJoinRoomButton);
	PasswordTextBox->OnTextChanged.AddDynamic(this, &UPasswordEnterPopup_Widget::OnTextChangedPasswordTextBox);
}

// remove the widget and return to server list
void UPasswordEnterPopup_Widget::OnClickedBackButton()
{
	RemoveFromParent();
}

// if the password matches let the player join the session through GameInstance
void UPasswordEnterPopup_Widget::OnClickedJoinRoomButton()
{
	FText PWTextBoxText = PasswordTextBox->GetText();
	FText PwAsText = UKismetTextLibrary::Conv_StringToText(Password);

	if (UKismetTextLibrary::EqualEqual_TextText(PWTextBoxText, PwAsText))
	{
		GameInstanceRef->JoinOnlineGame(SessionIndex);
	}
	else
	{
		// Display and Error Message if the password is not correct
		PasswordTextBox->SetError(FText::FromString(TEXT("Wrong Password")));
	}
}


void UPasswordEnterPopup_Widget::OnTextChangedPasswordTextBox(const FText &Text)
{
	// Clear Errors if there is any
	if (PasswordTextBox->HasError())
		PasswordTextBox->ClearError();
	
	// Check is the Password Textbox is Empty
	FText EmptyText = FText::FromString(TEXT(""));
	if (UKismetTextLibrary::NotEqual_TextText(Text, EmptyText))
	{
		// limit the password to the max lenght of the password
		FString OutLimitedPW;
		LimitTextBoxText(Text, MaxPasswordLength, OutLimitedPW);
		PasswordTextBox->SetText(UKismetTextLibrary::Conv_StringToText(OutLimitedPW));
	}
}

void UPasswordEnterPopup_Widget::LimitTextBoxText(const FText InText, const int32 MaxTextSize, FString& OutString)
{
	FString InFText = UKismetTextLibrary::Conv_TextToString(InText);
	OutString = UKismetStringLibrary::GetSubstring(InFText, 0, MaxTextSize);
}
