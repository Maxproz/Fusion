// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"
#include "SessionInviteRecieved_Widget.h"




void USessionInviteRecieved_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	AcceptInviteButton->OnClicked.AddDynamic(this, &USessionInviteRecieved_Widget::OnClickedAcceptInviteButton);
	CancelinviteButton->OnClicked.AddDynamic(this, &USessionInviteRecieved_Widget::OnClickedCancelinviteButton);
}

// accept the invite
void USessionInviteRecieved_Widget::OnClickedAcceptInviteButton()
{
	// TODO:
}

// remove the widget when the player presses cancel
void USessionInviteRecieved_Widget::OnClickedCancelinviteButton()
{
	RemoveFromParent();
}

