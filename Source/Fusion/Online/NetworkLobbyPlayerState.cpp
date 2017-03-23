// @Maxpro 2017

#include "Fusion.h"

#include "FusionGameInstance.h"

#include "NetworkLobbyPlayerState.h"


ANetworkLobbyPlayerState::ANetworkLobbyPlayerState()
{
	bIsReady = false;
}

void ANetworkLobbyPlayerState::BeginPlay()
{

	Super::BeginPlay();

	//change the player name to the name in the game insance if it is on lan
	ChangePlayerName();
}


void ANetworkLobbyPlayerState::ChangePlayerName()
{
	//check if this has authority
	if (Role == ROLE_Authority)
	{
		//try to get the NetworkedGameInstance
		UFusionGameInstance* FusionGameInstance = Cast<UFusionGameInstance>(GetWorld()->GetGameInstance());

		//if the game instance is not null, get the player name from it
		if (FusionGameInstance)
		{
			FString ActualPlayerName = FusionGameInstance->GetPlayerName();
			//if the string is empty that means we are on steam so no need to change the name
			if (ActualPlayerName != "")
				//if we are on lan set the player name to the name we got from game instance
				SetPlayerName(ActualPlayerName);
		}

	}
	else //if the player doesn't have authority call the serverside to call this function again
		Server_ChangePlayerName();
}

void ANetworkLobbyPlayerState::Server_ChangePlayerName_Implementation()
{
	//call the change player name fucntion from server side
	ChangePlayerName();
}


void ANetworkLobbyPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANetworkLobbyPlayerState, bIsReady);
}