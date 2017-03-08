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

	/** Starts the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientStartOnlineGame();

};
