// @Maxpro 2017

#include "Fusion.h"

#include "FusionGameViewportClient.h"
#include "FusionGameInstance.h"

#include "ConfirmationDialog_Widget.h"




void UConfirmationDialog_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	MessageTextBlock->SetText(DisplayMessage);

	//MessageOkButton->OnClicked.AddDynamic(this, &UConfirmationDialog_Widget::OnClickedOkButton);
	//MessageCancelButton->OnClicked.AddDynamic(this, &UConfirmationDialog_Widget::OnClickedCancelButton);
}

void UConfirmationDialog_Widget::FinishConstruct(TWeakObjectPtr<ULocalPlayer> InPlayerOwner, const FText& Message, EFusionDialogType::Type DialogTypee, TScriptDelegate<FWeakObjectPtr> OkButton, TScriptDelegate<FWeakObjectPtr> CancelButton)
{
	PlayerOwner = InPlayerOwner;

	DisplayMessage = Message;
	MessageOkButton->OnClicked.AddUnique(OkButton);
	MessageCancelButton->OnClicked.AddUnique(CancelButton);
	DialogType = DialogTypee;
}


// change the text whenever the error changes
void UConfirmationDialog_Widget::OnRep_DisplayMessage()
{
	MessageTextBlock->SetText(DisplayMessage);
}

void UConfirmationDialog_Widget::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UConfirmationDialog_Widget, DisplayMessage);
}

void UConfirmationDialog_Widget::ExecuteConfirm()
{
	if (MessageOkButton->OnClicked.IsBound())
	{
		if (DialogType == EFusionDialogType::ControllerDisconnected && PlayerOwner != nullptr)
		{
			PlayerOwner->SetControllerId(0);
		}

		MessageOkButton->OnClicked.Broadcast();
	}
}
bool UConfirmationDialog_Widget::NativeSupportsKeyboardFocus() const
{
	return true;
}

/*
FReply UConfirmationDialog_Widget::NativeOnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusReceived(MyGeometry, InFocusEvent);
	//return FReply::Handled().ReleaseMouseCapture().SetUserFocus(this), EFocusCause::SetDirectly, true);
}*/


FReply UConfirmationDialog_Widget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	Super::NativeOnKeyDown(MyGeometry, KeyEvent);

	const FKey Key = KeyEvent.GetKey();
	const int32 UserIndex = KeyEvent.GetUserIndex();

	// Filter input based on the type of this dialog
	switch (DialogType)
	{
	case EFusionDialogType::Generic:
	{
		// Ignore input from users that don't own this dialog
		if (PlayerOwner != nullptr && PlayerOwner->GetControllerId() != UserIndex)
		{
			return FReply::Unhandled();
		}
		break;
	}

	case EFusionDialogType::ControllerDisconnected:
	{
		// Only ignore input from controllers that are bound to local users
		if (PlayerOwner != nullptr && PlayerOwner->GetGameInstance() != nullptr)
		{
			if (PlayerOwner->GetGameInstance()->FindLocalPlayerFromControllerId(UserIndex))
			{
				return FReply::Unhandled();
			}
		}
		break;
	}
	}

	// For testing on PC
	if ((Key == EKeys::Enter || Key == EKeys::Gamepad_FaceButton_Bottom) && !KeyEvent.IsRepeat())
	{
		ExecuteConfirm();
	}
	else if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right)
	{
		if (MessageCancelButton->OnClicked.IsBound())
		{
			MessageCancelButton->OnClicked.Broadcast();
		}
	}

	return FReply::Unhandled();
}


void UConfirmationDialog_Widget::OnClickedOkButton()
{

}

void UConfirmationDialog_Widget::OnClickedCancelButton()
{

}

