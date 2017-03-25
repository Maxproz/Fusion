// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"

#include "FusionHUD.h"

#include "FusionPlayerController.generated.h"





/**
 * 
 */
UCLASS(config=Game)
class FUSION_API AFusionPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	AFusionPlayerController(const FObjectInitializer& ObjectInitializer);
	
	/* Flag to respawn or start spectating upon death */
	//UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	//bool bRespawnImmediately;

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSuicide();
	void ServerSuicide_Implementation();
	bool ServerSuicide_Validate();

	/* Respawn or start spectating after dying */
	virtual void UnFreeze() override;


public:

	/* Kill the current pawn */
	UFUNCTION(exec)
	virtual void Suicide();

	/* Start spectating. Should be called only on server */
	void StartSpectating();

	UFUNCTION(Client, Reliable)
	void ClientShowInGameHUD();

	/** sets spectator location and rotation */
	UFUNCTION(reliable, client)
	void ClientSetSpectatorCamera(FVector CameraLocation, FRotator CameraRotation);

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

	/** notify player about finished match */
	virtual void ClientGameEnded_Implementation(class AActor* EndGameFocus, bool bIsWinner);

	/** used for input simulation from blueprint (for automatic perf tests) */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SimulateInputKey(FKey Key, bool bPressed = true);

	/** sends cheat message */
	UFUNCTION(reliable, server, WithValidation)
	void ServerCheat(const FString& Msg);

	/* Overriden Message implementation. */
	virtual void ClientTeamMessage_Implementation(APlayerState* SenderPlayerState, const FString& S, FName Type, float MsgLifeTime) override;

	/* Tell the HUD to toggle the chat window. */
	void ToggleChatWindow();

	/** Local function say a string */
	UFUNCTION(exec)
	virtual void Say(const FString& Msg);

	/** RPC for clients to talk to server */
	UFUNCTION(unreliable, server, WithValidation)
	void ServerSay(const FString& Msg);

	/** notify local client about deaths */
	void OnDeathMessage(class AFusionPlayerState* KillerPlayerState, class AFusionPlayerState* KilledPlayerState, const UDamageType* KillerDamageType);

	/** Hides scoreboard if currently diplayed */
	void OnConditionalCloseScoreboard();

	/** Toggles scoreboard */
	void OnToggleScoreboard();

	void OnToggleInGameMenu();

	/** shows scoreboard */
	void OnShowScoreboard();

	/** hides scoreboard */
	void OnHideScoreboard();

	/** set infinite ammo cheat */
	void SetInfiniteAmmo(bool bEnable);

	/** set infinite clip cheat */
	void SetInfiniteClip(bool bEnable);

	/** set health regen cheat */
	void SetHealthRegen(bool bEnable);

	/** set god mode cheat */
	UFUNCTION(exec)
	void SetGodMode(bool bEnable);

	/** get infinite ammo cheat */
	bool HasInfiniteAmmo() const;

	/** get infinite clip cheat */
	bool HasInfiniteClip() const;

	/** get health regen cheat */
	bool HasHealthRegen() const;

	/** get gode mode cheat */
	bool HasGodMode() const;


	/**
	* Called when the read achievements request from the server is complete
	*
	* @param PlayerId The player id who is responsible for this delegate being fired
	* @param bWasSuccessful true if the server responded successfully to the request
	*/
	void OnQueryAchievementsComplete(const FUniqueNetId& PlayerId, const bool bWasSuccessful);

	// Begin APlayerController interface

	/** handle weapon visibility */
	virtual void SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning) override;

	/** Returns true if movement input is ignored. Overridden to always allow spectators to move. */
	virtual bool IsMoveInputIgnored() const override;

	/** Returns true if look input is ignored. Overridden to always allow spectators to look around. */
	virtual bool IsLookInputIgnored() const override;

	/** initialize the input system from the player settings */
	virtual void InitInputSystem() override;

	virtual bool SetPause(bool bPause, FCanUnpause CanUnpauseDelegate = FCanUnpause()) override;

	// End APlayerController interface


	/**
	* Reads achievements to precache them before first use
	*/
	void QueryAchievements();

	/**
	* Writes a single achievement (unless another write is in progress).
	*
	* @param Id achievement id (string)
	* @param Percent number 1 to 100
	*/
	void UpdateAchievementProgress(const FString& Id, float Percent);

	/** Returns the persistent user record associated with this player, or null if there is't one. */
	class UFusionPersistentUser* GetPersistentUser() const;

	/** Informs that player fragged someone */
	void OnKill();


	/** Associate a new UPlayer with this PlayerController. */
	virtual void SetPlayer(UPlayer* Player);


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

	/** infinite ammo cheat */
	UPROPERTY(Transient, Replicated)
	bool bInfiniteAmmo = false;

	/** infinite clip cheat */
	UPROPERTY(Transient, Replicated)
	bool bInfiniteClip = false;

	/** health regen cheat */
	UPROPERTY(Transient, Replicated)
	bool bHealthRegen = false;

	/** god mode cheat */
	UPROPERTY(Transient, Replicated)
	bool bGodMode = false;

	/** stores pawn location at last player death, used where player scores a kill after they died **/
	FVector LastDeathLocation;

	/** shooter in-game menu */
	//TSharedPtr<class FFusionIngameMenu> ShooterIngameMenu;

	/** try to find spot for death cam */
	bool FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation);

	virtual void BeginDestroy() override;

	/** Achievements write object */
	FOnlineAchievementsWritePtr WriteObject;

	/** true for the first frame after the game has ended */
	bool bGameEndedFrame = false;

	/** if set, gameplay related actions (movement, weapn usage, etc) are allowed */
	bool bAllowGameActions = true;

	// For tracking whether or not to send the end event
	bool bHasSentStartEvents = false;



	//Begin AActor interface

	/** after all game elements are created */
	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	//End AActor interface

	//Begin AController interface

	/** transition to dead state, retries spawning later */
	virtual void FailedToSpawnPawn() override;

	/** update camera when pawn dies */
	virtual void PawnPendingDestroy(APawn* P) override;

	//End AController interface


	/** sets up input */
	virtual void SetupInputComponent() override;



	/**
	* Called from game info upon end of the game, used to transition to proper state.
	*
	* @param EndGameFocus Actor to set as the view target on end game
	* @param bIsWinner true if this controller is on winning team
	*/
	virtual void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) override;

	/** Return the client to the main menu gracefully.  ONLY sets GI state. */
	void ClientReturnToMainMenu_Implementation(const FString& ReturnReason) override;

	/** Updates achievements based on the PersistentUser stats at the end of a round */
	void UpdateAchievementsOnGameEnd();

	/** Updates leaderboard stats at the end of a round */
	void UpdateLeaderboardsOnGameEnd();

	/** Updates the save file at the end of a round */
	void UpdateSaveFileOnGameEnd(bool bIsWinner);

	// End APlayerController interface

	FName	ServerSayString;

	// Timer used for updating friends in the player tick.
	float FusionFriendUpdateTimer;


private:

	/** Handle for efficient management of ClientStartOnlineGame timer */
	FTimerHandle TimerHandle_ClientStartOnlineGame;
};


