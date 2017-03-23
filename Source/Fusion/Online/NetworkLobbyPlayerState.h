// @Maxpro 2017

#pragma once

#include "GameFramework/PlayerState.h"
#include "NetworkLobbyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API ANetworkLobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
	

public:

	ANetworkLobbyPlayerState();

	virtual void BeginPlay() override;


	UPROPERTY(Replicated)
	bool bIsReady;

	/**
	*	Calls the game instace to get the lan player name if we are not on steam then sets it
	*/
	void ChangePlayerName();

	/**
	*	ChangePlayerName server side function if the caller is a client
	*/
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_ChangePlayerName();
	void Server_ChangePlayerName_Implementation();
	FORCEINLINE bool Server_ChangePlayerName_Validate() { return true; }


};