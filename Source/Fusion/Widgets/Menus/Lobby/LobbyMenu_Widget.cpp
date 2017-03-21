// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionPlayerController_Lobby.h"
#include "FusionHUD.h"
#include "FusionGameInstance.h"
#include "FusionGameSession.h"

#include "ChatEntry_Widget.h"
#include "PlayerInfoEntry_Widget.h"
#include "InviteSteamFriend_Widget.h"

#include "Runtime/Engine/Classes/Kismet/KismetStringLibrary.h"
//#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetTextLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetArrayLibrary.h"

#include "LobbyMenu_Widget.h"



#define LOCTEXT_NAMESPACE "Fusion.HUD.Menu"


// only let the player change the settings or kick another player if he is the host
void ULobbyMenu_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	LobbyPlayerControllerRef = Cast<AFusionPlayerController_Lobby>(GetOwningPlayer());
	GameInstanceRef = Cast<UFusionGameInstance>(LobbyPlayerControllerRef->GetGameInstance());


	bool IsThisServer = UKismetSystemLibrary::IsServer(GetWorld());
	
	KickPlayerButton->SetIsEnabled(IsThisServer);
	SettingsButton->SetIsEnabled(IsThisServer);
	
	// only let players invite other players if steam is active
	InvitePlayerButton->SetIsEnabled(GameInstanceRef->IsOnlineSubsystemSteam());

	if (IsThisServer)
	{
		// if the server, show the start game button
		LobbyPlayerControllerRef->SetIsReadyState(IsThisServer);
	}
	else
	{
		// if not server show the ready button
		ReadyAndStartGameWidgetSwitcher_0->SetActiveWidgetIndex(1);

	}


	// TODO: Is SP the correct add here?
	ReceiveChatMessageCompleteEvent.AddUObject(this, &ULobbyMenu_Widget::OnReceiveChatMessageComplete);
	OnUpdatePlayerListEvent.AddUObject(this, &ULobbyMenu_Widget::OnUpdatePlayerList);
	GetSteamFriendRequestCompletedEvent.AddUObject(this, &ULobbyMenu_Widget::OnGetSteamFriendRequestCompleted);
	
	ChatboxEditableTextBox->OnTextCommitted.AddDynamic(this, &ULobbyMenu_Widget::OnTextCommittedChatBoxEditableTextbox);

	LeaveButton->OnClicked.AddDynamic(this, &ULobbyMenu_Widget::OnClickedLeaveButton);
	InvitePlayerButton->OnClicked.AddDynamic(this, &ULobbyMenu_Widget::OnClickedInvitePlayerButton);
	KickPlayerButton->OnClicked.AddDynamic(this, &ULobbyMenu_Widget::OnClickedKickPlayerButton);
	CloseInviteSteamFriendButton->OnClicked.AddDynamic(this, &ULobbyMenu_Widget::OnClickedCloseInviteSteamFriendButton);
	StartGameButton->OnClicked.AddDynamic(this, &ULobbyMenu_Widget::OnClickedStartGameButton);
	ReadyButton->OnClicked.AddDynamic(this, &ULobbyMenu_Widget::OnClickedReadyButton);
}

// creates a chat entry and adds it to the scrollbox
void ULobbyMenu_Widget::OnReceiveChatMessageComplete(FText ChatMessage)
{
	UChatEntry_Widget* EntryWidget = CreateWidget<UChatEntry_Widget>(LobbyPlayerControllerRef, LobbyPlayerControllerRef->GetFusionHUD()->ChatEntry_WidgetTemplate);
	ChatEntriesScrollBox->AddChild(EntryWidget);
	ChatEntriesScrollBox->ScrollToEnd();
}

// if the chat message is not empty send the chat message to the game mode to pordcast it to all players then clears the text box to prepare it for another entry
void ULobbyMenu_Widget::OnTextCommittedChatBoxEditableTextbox(const FText &Text, ETextCommit::Type Method)
{
	FText EmptyText = FText::FromString(TEXT(""));

	if (UKismetTextLibrary::NotEqual_TextText(Text, EmptyText) && Method == ETextCommit::OnEnter)
	{
		LobbyPlayerControllerRef->SendChatMessage(Text);
		ChatboxEditableTextBox->SetText(EmptyText);
	}
}

void ULobbyMenu_Widget::OnUpdatePlayerList(TArray<FLobbyPlayerInfo> PlayerInfoArray)
{
	PlayerListScrollBox->ClearChildren();
	KickButtons.Empty();

	
	int32 ArrayIndex = 0;
	for (const auto& LobbyPlayerInfo : PlayerInfoArray)
	{
		UPlayerInfoEntry_Widget* PlayerInfoEntry_Widget = CreateWidget<UPlayerInfoEntry_Widget>(LobbyPlayerControllerRef, LobbyPlayerControllerRef->GetFusionHUD()->PlayerInfoEntry_WidgetTemplate);
		PlayerInfoEntry_Widget->SetLobbyPlayerInfo(LobbyPlayerInfo);
		PlayerInfoEntry_Widget->SetPlayerIndex(ArrayIndex);

		PlayerListScrollBox->AddChild(PlayerInfoEntry_Widget);
		
		if (ArrayIndex == 0)
		{
			PlayerInfoEntry_Widget->KickButton->SetVisibility(ESlateVisibility::Hidden);
		}
		else
		{
			PlayerInfoEntry_Widget->KickButton->SetVisibility(KickingPlayers);
		}

		KickButtons.Add(PlayerInfoEntry_Widget->KickButton);

		ArrayIndex = ArrayIndex + 1;
	}

	bool bShouldEnable = LobbyPlayerControllerRef->CanGameStart();
	StartGameButton->SetIsEnabled(bShouldEnable);
}

// call the game instance to destroy the session and leave game if the player wants to leave
void ULobbyMenu_Widget::OnClickedLeaveButton()
{
	GameInstanceRef->DestroySessionAndLeaveGame();
}

// Inviting steam friends if the player is on steam and the game is not full
void ULobbyMenu_Widget::OnClickedInvitePlayerButton()
{
	bool bOutIsSessionFull;
	IsSessionFull(bOutIsSessionFull);

	if (!bOutIsSessionFull)
	{
		// clear all previous entries
		InviteSteamFreindsListScrollBox->ClearChildren();
		
		// Call the Game instance to get friend list
		GameInstanceRef->GetSteamFriendsList(LobbyPlayerControllerRef);
	}
	else
	{
		// don't let the player invite any more players if the room is full
		FText ErrorText = FText::FromString(TEXT("Room is full can't invite anymore players"));
		GameInstanceRef->ShowErrorMessage(ErrorText);
	}
}

// invert whatever kickingplayer is (default hidden) then set all the children (Player entries in player list) to be the same accordenly)
void ULobbyMenu_Widget::OnClickedKickPlayerButton() // Making a note to re-check this functionality later
{
	if (KickingPlayers == ESlateVisibility::Hidden)
	{
		KickingPlayers = ESlateVisibility::Visible;
	}
	else
	{
		KickingPlayers = ESlateVisibility::Hidden;
	}

	int32 LastIndex = KickButtons.Num() - 1;
	for (int Index = 1; Index < LastIndex; Index++)
	{
		KickButtons[Index]->SetVisibility(KickingPlayers);
	}
}

void ULobbyMenu_Widget::OnClickedCloseInviteSteamFriendButton()
{
	InviteSteamFriendListOverlay->SetVisibility(ESlateVisibility::Hidden);
}

// if all players are ready, start the game, if they are not, show an error message
void ULobbyMenu_Widget::OnClickedStartGameButton()
{
	if (LobbyPlayerControllerRef->CanGameStart())
	{
		LobbyPlayerControllerRef->StartGame();
	}
	else
	{
		FText ErrorMsg = FText::FromString(TEXT("Some Players Are not ready"));
		GameInstanceRef->ShowErrorMessage(ErrorMsg);
	}
}

// reverse the ready state whenver the button is clicked and show the new text
void ULobbyMenu_Widget::OnClickedReadyButton()
{
	if (bIsReadyState)
	{
		bIsReadyState = false;
		ReadyButtonTextBlock->SetText(FText::FromString(TEXT("Ready")));

	}
	else
	{
		bIsReadyState = true;
		ReadyButtonTextBlock->SetText(FText::FromString(TEXT("Not Ready")));
	}

	LobbyPlayerControllerRef->SetIsReadyState(bIsReadyState);
}

void ULobbyMenu_Widget::OnGetSteamFriendRequestCompleted(TArray<FSteamFriendInfo> FriendsList)
{
	for (const auto& Friend : FriendsList)
	{
		UInviteSteamFriend_Widget* InviteSteamFriend_Widget = CreateWidget<UInviteSteamFriend_Widget>(LobbyPlayerControllerRef, LobbyPlayerControllerRef->GetFusionHUD()->InviteSteamFriend_WidgetTemplate);
		InviteSteamFriend_Widget->SetSteamFriendInfo(Friend);
		InviteSteamFriend_Widget->SetLobbyMenuWidgetRef(this);
		InviteSteamFriend_Widget->SetGameInstanceRef(GameInstanceRef);

		// add the entry to the scrollbox
		InviteSteamFreindsListScrollBox->AddChild(InviteSteamFriend_Widget);
	}

	// show the invite list after the scrollbox is populated
	InviteSteamFriendListOverlay->SetVisibility(ESlateVisibility::Visible);
}

void ULobbyMenu_Widget::IsSessionFull(bool& bOutResult)
{
	bOutResult = PlayerListScrollBox->GetChildrenCount() >= GameInstanceRef->GetSessionMaxPlayers();
}

// This shit didnt work
FText ULobbyMenu_Widget::NumberOfPlayersBinding() const
{
	// Setting the Current/Max Number of Players
	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("CurrentNumberOfPlayers"), FText::AsNumber(PlayerListScrollBox->GetChildrenCount()));
	Arguments.Add(TEXT("MaxNumberOfPlayers"), FText::AsNumber(GameInstanceRef->GetSessionMaxPlayers()));
	return FText::Format(LOCTEXT("Fusion.HUD.Menu", "{CurrentNumberOfPlayers}/{MaxNumberOfPlayers}"), Arguments);
}



#undef LOCTEXT_NAMESPACE



