// @Maxpro 2017

#include "Fusion.h"
#include "OkErrorMessage_Widget.h"




void UOkErrorMessage_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	// set the passed in error text
	ErrorTextBlock->SetText(ErrorText);

	ErrorOkButton->OnClicked.AddDynamic(this, &UOkErrorMessage_Widget::OnClickedErrorOkButton);
}

// remove the error message when the player presses ok
void UOkErrorMessage_Widget::OnClickedErrorOkButton()
{
	RemoveFromParent();
}

// change the text whenever the error changes
void UOkErrorMessage_Widget::OnRep_ErrorText()
{
	ErrorTextBlock->SetText(ErrorText);
}

void UOkErrorMessage_Widget::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UOkErrorMessage_Widget, ErrorText);
}