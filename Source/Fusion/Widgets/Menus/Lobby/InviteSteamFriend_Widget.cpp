// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "Runtime/Engine/Classes/Kismet/KismetTextLibrary.h"

#include "FusionGameInstance.h"

#include "LobbyMenu_Widget.h"

#include "FusionHUD.h"
#include "FusionPlayerController_Lobby.h"

#include "InviteSteamFriend_Widget.h"


void UInviteSteamFriend_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	SteamFriendAvatarImage->SetBrushFromTexture(SteamFriendInfo.PlayerAvatar);
	SteamFriendNameTextBlock->SetText(FText::FromString(SteamFriendInfo.PlayerName));

	SteamFriendInfoButton->OnClicked.AddDynamic(this, &UInviteSteamFriend_Widget::OnClickedSteamFriendInfoButton);

	AFusionPlayerController_Lobby* LPC = Cast<AFusionPlayerController_Lobby>(GetOwningPlayer());
	if (!LPC)
	{
		return;
	}

	AFusionHUD* FusionHUD = LPC->GetFusionHUD();
	ULobbyMenu_Widget* LobbyMenu_WidgetRef = FusionHUD->GetLobbyMenuWidget();

}

void UInviteSteamFriend_Widget::OnClickedSteamFriendInfoButton()
{

	FText ErrorMsg = FText::FromString(TEXT("Room is full can't invite anymore players"));
	
	// Make sure the session is not full before inviting any players
	bool bOutIsFull;
	LobbyMenu_WidgetRef->IsSessionFull(bOutIsFull);

	if (bOutIsFull)
	{
		GameInstanceRef->ShowErrorMessage(ErrorMsg);
	}
	else
	{
		GameInstanceRef->SendSessionInviteToFriend(GetOwningPlayer(), SteamFriendInfo.PlayerUniqueNetID);
	}

}
