// @Maxpro 2017

#include "Fusion.h"

#include "FusionGameInstance.h"
#include "FusionGameViewportClient.h"

#include "FusionMessageMenu_Widget.h"



void UFusionMessageMenu_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UFusionGameInstance>(GetOwningPlayer()->GetGameInstance());


	MessageTextBlock->SetText(DisplayMessage);

	MessageOkButton->OnClicked.AddDynamic(this, &UFusionMessageMenu_Widget::OnClickedOkButton);
	MessageCancelButton->OnClicked.AddDynamic(this, &UFusionMessageMenu_Widget::OnClickedCancelButton);
}

void UFusionMessageMenu_Widget::UpdateDisplayMessage(FText NewDisplayMessage)
{
	DisplayMessage = NewDisplayMessage;
}

// change the text whenever the error changes
void UFusionMessageMenu_Widget::OnRep_DisplayMessage()
{
	MessageTextBlock->SetText(DisplayMessage);
}

void UFusionMessageMenu_Widget::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFusionMessageMenu_Widget, DisplayMessage);
}


void UFusionMessageMenu_Widget::OnClickedOkButton()
{
	GameInstance->GotoState(FusionGameInstanceState::MainMenu);
}

void UFusionMessageMenu_Widget::OnClickedCancelButton()
{
	GameInstance->GotoState(FusionGameInstanceState::MainMenu);
}


