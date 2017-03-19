// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionPlayerController_Lobby.h"
#include "NetworkLobbyPlayerState.h"

#include "FusionGame_Lobby.h"



void AFusionGame_Lobby::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AFusionPlayerController_Lobby* JoiningPlayer = Cast<AFusionPlayerController_Lobby>(NewPlayer);

	//if the joining player is a lobby player controller, add him to a list of connected Players
	if (JoiningPlayer)
		ConnectedPlayers.Add(JoiningPlayer);
}

void AFusionGame_Lobby::Logout(AController* ExitingPlayer)
{
	Super::Logout(ExitingPlayer);

	//update the ConnectedPlayers Array and the PlayerList in the lobby whenever a player leaves
	AFusionPlayerController_Lobby* LobbyPlayerController = Cast<AFusionPlayerController_Lobby>(ExitingPlayer);
	if (LobbyPlayerController)
	{
		ConnectedPlayers.Remove(LobbyPlayerController);
		UpdatePlayerList();
	}

}

void AFusionGame_Lobby::ProdcastChatMessage(const FText & ChatMessage)
{
	//call all the connected players and pass in the chat message
	for (AFusionPlayerController_Lobby* Player : ConnectedPlayers)
		Player->Client_ReceiveChatMessage(ChatMessage);
}

void AFusionGame_Lobby::KickPlayer(int32 PlayerIndex)
{
	//call the player to make him destroy his session and leave game
	ConnectedPlayers[PlayerIndex]->Client_GotKicked();
}

void AFusionGame_Lobby::UpdatePlayerList()
{
	//Epmty the PlayerInfo Array to re-populate it
	PlayerInfoArray.Empty();

	//get all the player info from all the connected players
	for (AFusionPlayerController_Lobby* Player : ConnectedPlayers)
	{
		//temporary LobbyPlayerInfo var to hold the player info
		FLobbyPlayerInfo TempLobbyPlayerInfo;

		ANetworkLobbyPlayerState* NetworkLobbyPlayerState = Cast<ANetworkLobbyPlayerState>(Player->PlayerState);
		if (NetworkLobbyPlayerState)
			TempLobbyPlayerInfo.bPlayerReadyState = NetworkLobbyPlayerState->bIsReady;
		else
			TempLobbyPlayerInfo.bPlayerReadyState = false;

		TempLobbyPlayerInfo.PlayerName = Player->PlayerState->PlayerName;
		PlayerInfoArray.Add(TempLobbyPlayerInfo);
	}

	//call all the players to make them update and pass in the player info array
	for (AFusionPlayerController_Lobby* Player : ConnectedPlayers)
		Player->Client_UpdatePlayerList(PlayerInfoArray);
}

void AFusionGame_Lobby::StartGameFromLobby()
{
	GetWorld()->ServerTravel(GameMapName);
}

bool AFusionGame_Lobby::IsAllPlayerReady() const
{
	for (AFusionPlayerController_Lobby* Player : ConnectedPlayers)
	{
		ANetworkLobbyPlayerState* NetworkLobbyPlayerState = Cast<ANetworkLobbyPlayerState>(Player->PlayerState);
		if (NetworkLobbyPlayerState)
			if (!NetworkLobbyPlayerState->bIsReady)
				return false;
	}
	return true;
}


void AFusionGame_Lobby::PlayerRequestUpdate()
{
	UpdatePlayerList();
}

