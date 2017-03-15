// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"

#include "FusionHUD.h"

#include "FusionPlayerController.generated.h"


UENUM()
enum class EHUDMessage : uint8
{
	/* Weapons */
	Weapon_Picked_Up,

	/* Character */
	Character_Shields_Recharged,

	/* Gamemode */
	Game_CTF,
	Game_Slayer,
	Game_FFA,
	Game_Assault,

	/* No category specified */
	None,
};


/**
 * 
 */
UCLASS()
class FUSION_API AFusionPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	AFusionPlayerController(const FObjectInitializer& ObjectInitializer);
	
	/* Flag to respawn or start spectating upon death */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	bool bRespawnImmediately;

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSuicide();
	void ServerSuicide_Implementation();
	bool ServerSuicide_Validate();

	/* Respawn or start spectating after dying */
	virtual void UnFreeze() override;

	/** Handle for efficient management of ClientStartOnlineGame timer */
	FTimerHandle TimerHandle_ClientStartOnlineGame;

public:

	UFUNCTION(Reliable, Client)
	void ClientHUDStateChanged(EHUDState NewState);
	void ClientHUDStateChanged_Implementation(EHUDState NewState);

	/* Enum is remapped to localized text before sending it to the HUD */
	UFUNCTION(Reliable, Client)
	void ClientHUDMessage(EHUDMessage MessageID);
	void ClientHUDMessage_Implementation(EHUDMessage MessageID);

	FText GetText(EHUDMessage MsgID);

	/* Kill the current pawn */
	UFUNCTION(exec)
	virtual void Suicide();

	/* Start spectating. Should be called only on server */
	void StartSpectating();


	/** notify player about started match */
	UFUNCTION(reliable, client)
	void ClientGameStarted();
	void ClientGameStarted_Implementation();

	/** Starts the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientStartOnlineGame();
	void ClientStartOnlineGame_Implementation();

	/** Ends the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientEndOnlineGame();
	void ClientEndOnlineGame_Implementation();
	
	/** Notifies clients to send the end-of-round event */
	UFUNCTION(reliable, client)
	void ClientSendRoundEndEvent(bool bIsWinner, int32 ExpendedTimeInSeconds);
	void ClientSendRoundEndEvent_Implementation(bool bIsWinner, int32 ExpendedTimeInSeconds);


	/** check if gameplay related actions (movement, weapon usage, etc) are allowed right now */
	bool IsGameInputAllowed() const;

	/** Returns a pointer to the Fusion game hud. May return NULL. */
	class AFusionHUD* GetFusionHUD() const;

	/** Ends and/or destroys game session */
	void CleanupSessionOnReturnToMenu();

	/** Cleans up any resources necessary to return to main menu.  Does not modify GameInstance state. */
	virtual void HandleReturnToMainMenu();

	/** Show the in-game menu if it's not already showing */
	void ShowInGameMenu();

	/** is game menu currently active? */
	bool IsGameMenuVisible() const;

	virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel) override;

protected:

	/** true for the first frame after the game has ended */
	bool bGameEndedFrame = false;

	/** if set, gameplay related actions (movement, weapn usage, etc) are allowed */
	bool bAllowGameActions = true;

	// For tracking whether or not to send the end event
	bool bHasSentStartEvents = false;

};
