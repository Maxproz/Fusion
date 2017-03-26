// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "OnlineIdentityInterface.h"
#include "OnlineSessionInterface.h"
#include "FusionGameInstance.generated.h"

#define SETTING_SERVER_NAME FName(TEXT("SERVERNAMEKEY"))
#define SETTING_SERVER_IS_PROTECTED FName(TEXT("SERVERSERVERISPASSWORDPROTECTEDKEY"))
#define SETTING_SERVER_PROTECT_PASSWORD FName(TEXT("SERVERPROTECTPASSWORDKEY"))

namespace FusionGameInstanceState
{
	extern const FName None;
	extern const FName MainMenu;
	extern const FName MessageMenu;
	extern const FName Lobby;
	extern const FName Playing;
}

/** This class holds the value of what message to display when we are in the "MessageMenu" state */
class FFusionPendingMessage
{
public:
	FText	DisplayString;				// This is the display message in the main message body
	FText	OKButtonString;				// This is the ok button text
	FText	CancelButtonString;			// If this is not empty, it will be the cancel button text
	FName	NextState;					// Final destination state once message is discarded

	TWeakObjectPtr< ULocalPlayer > PlayerOwner;		// Owner of dialog who will have focus (can be NULL)
};

class FFusionPendingInvite
{
public:
	FFusionPendingInvite() : ControllerId(-1), UserId(nullptr), bPrivilegesCheckedAndAllowed(false) {}

	int32							 ControllerId;
	TSharedPtr< const FUniqueNetId > UserId;
	FOnlineSessionSearchResult 		 InviteResult;
	bool							 bPrivilegesCheckedAndAllowed;
};



//A custom struct to be able to access the Session results in blueprint
USTRUCT(BlueprintType)
struct FCustomFusionSessionResult
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Blueprint Session Result")
	FString SessionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Blueprint Session Result")
	bool bIsLan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Blueprint Session Result")
	int32 CurrentNumberOfPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Blueprint Session Result")
	int32 MaxNumberOfPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Blueprint Session Result")
	bool bIsPasswordProtected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Blueprint Session Result")
	FString SessionPassword;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Blueprint Session Result")
	int32 Ping;
};


//stolen from Advanced Session Plugin cuz I can't find any other way to put FUniqueNetId in a struct
USTRUCT(BlueprintType)
struct FBPUniqueNetId
{
	GENERATED_USTRUCT_BODY()

private:
	bool bUseDirectPointer;


public:
	TSharedPtr<const FUniqueNetId> UniqueNetId;
	const FUniqueNetId * UniqueNetIdPtr;

	void SetUniqueNetId(const TSharedPtr<const FUniqueNetId> &ID)
	{
		bUseDirectPointer = false;
		UniqueNetIdPtr = nullptr;
		UniqueNetId = ID;
	}

	void SetUniqueNetId(const FUniqueNetId *ID)
	{
		bUseDirectPointer = true;
		UniqueNetIdPtr = ID;
	}

	bool IsValid() const
	{
		if (bUseDirectPointer && UniqueNetIdPtr != nullptr)
		{
			return true;
		}
		else if (UniqueNetId.IsValid())
		{
			return true;
		}
		else
			return false;

	}

	const FUniqueNetId* GetUniqueNetId() const
	{
		if (bUseDirectPointer && UniqueNetIdPtr != nullptr)
		{
			// No longer converting to non const as all functions now pass const UniqueNetIds
			return /*const_cast<FUniqueNetId*>*/(UniqueNetIdPtr);
		}
		else if (UniqueNetId.IsValid())
		{
			return UniqueNetId.Get();
		}
		else
			return nullptr;
	}

	FBPUniqueNetId()
	{
		bUseDirectPointer = false;
		UniqueNetIdPtr = nullptr;
	}
};

USTRUCT(BlueprintType)
struct FSteamFriendInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Friend Info")
	UTexture2D* PlayerAvatar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Friend Info")
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Friend Info")
	FBPUniqueNetId PlayerUniqueNetID;

};



/**
 * 
 */
UCLASS(config=Game)
class FUSION_API UFusionGameInstance : public UGameInstance
{
public:
	GENERATED_UCLASS_BODY()

public:
	//UFusionGameInstance(const FObjectInitializer& ObjectInitializer);

	bool Tick(float DeltaSeconds);

	class AFusionGameSession* GetGameSession() const;

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;

	void SetPendingInvite(const FFusionPendingInvite& InPendingInvite);





	/** Sends the game to the specified state. */
	void GotoState(FName NewState);

	/** Obtains the initial welcome state, which can be different based on platform */
	FName GetInitialState();

	/** Sends the game to the initial startup/frontend state  */
	void GotoInitialState();

	UFUNCTION()
	void EmptyFunction();

	UFUNCTION()
	void HideDialogMenuTestFunc();

	UFUNCTION(Exec)
	void TestConfirmDialog();

	UFUNCTION(Exec)
	void TestMessageMenu();

	/**
	* Creates the message menu, clears other menus and sets the KingState to Message.
	*
	* @param	Message				Main message body
	* @param	OKButtonString		String to use for 'OK' button
	* @param	CancelButtonString	String to use for 'Cancel' button
	* @param	NewState			Final state to go to when message is discarded
	*/
	void ShowMessageThenGotoState(const FText& Message, const FName& NewState, const bool OverrideExisting = true, TWeakObjectPtr< ULocalPlayer > PlayerOwner = nullptr);



	TSharedPtr< const FUniqueNetId > GetUniqueNetIdFromControllerId(const int ControllerId);

	/** Returns true if the game is in online mode */
	bool GetIsOnline() const { return bIsOnline; }

	/** Sets the online mode of the game */
	void SetIsOnline(bool bInIsOnline);

	


	/** Returns true if the passed in local player is signed in and online */
	bool IsLocalPlayerOnline(ULocalPlayer* LocalPlayer);

	/** Returns true if owning player is online. Displays proper messaging if the user can't play */
	bool ValidatePlayerForOnlinePlay(ULocalPlayer* LocalPlayer);

	/** Shuts down the session, and frees any net driver */
	void CleanupSessionOnReturnToMenu();

	/** Flag the local player when they quit the game */
	void LabelPlayerAsQuitter(ULocalPlayer* LocalPlayer) const;

	// Generic confirmation handling (just hide the dialog)
	FReply OnConfirmGeneric();
	bool HasLicense() const { return bIsLicensed; }

	/** Start task to get user privileges. */
	void StartOnlinePrivilegeTask(const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate& Delegate, EUserPrivileges::Type Privilege, TSharedPtr< const FUniqueNetId > UserId);

	/** Common cleanup code for any Privilege task delegate */
	void CleanupOnlinePrivilegeTask();

	/** Show approved dialogs for various privileges failures */
	void DisplayOnlinePrivilegeFailureDialogs(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	/** @return OnlineSession class to use for this player */
	TSubclassOf<class UOnlineSession> GetOnlineSessionClass() override;


	
	// Creating a Session
	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void StartOnlineGame(FString ServerName, int32 MaxNumPlayers, bool bIsLAN, bool bIsPresence, bool bIsPasswordProtected, FString SessionPassword);

	// Searching and Finding a Session
	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void FindOnlineGames(bool bIsLAN, bool bIsPresence);

	// Joining a Session
	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void JoinOnlineGame(int32 SessionIndex);

	// Destroying a Session
	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void DestroySessionAndLeaveGame();
	
	/**
	* called from the player state to get the player name
	* @retrun		returns empty string if the player is on steam and retruns the LanPlayerName if he is on Lan
	*/
	FString GetPlayerName() const;

	bool IsOnlineSubsystemSteam() const;

	//Lan player name to not use the Computer Name
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lan")
	FString LanPlayerName;


	UFUNCTION(BlueprintCallable, Category = "Network|Friends")
	void SendSessionInviteToFriend(APlayerController* InvitingPlayer, const FBPUniqueNetId & Friend);


	/**
	*	called to show an error message in UMG
	*	@Param	ErrorMessage The Error meaage you want to show
	*/
	UFUNCTION(BlueprintCallable, Category = "Network|Errors")
	void ShowErrorMessage(const FText & ErrorMessage);


	/**
	* gets the steam avatar of a player based on his UniqueNetId
	* @Param		UniqueNetId		the UniqueNetId of the player you want to get his avatar
	*/
	UTexture2D* GetSteamAvatar(const FBPUniqueNetId UniqueNetId);

	/**
	* called to get the list of steam friends a player has
	* @Param		PlayerController		the player controller of the player asking for the friend list
	* @Param		FriendsList				list of friends' info in  bluepritn wrapper structure
	*/
	UFUNCTION(BlueprintCallable, Category = "Network|Friends")
	void GetSteamFriendsList(APlayerController *PlayerController);


	/** Delegate for reading the friendlist*/
	FOnReadFriendsListComplete FriendListReadCompleteDelegate;

	/**
	* Delegate fired when the friend list request has been processed
	* @param	LocalUserNum		The local user id (UniqueNetId) of the requesting player
	* @param	bWasSuccessful		true if the async action completed without error, false if there was an error
	* @param	ListName			the friend list name
	* @param	ErrorString			if there is any errors
	*/
	void OnReadFriendsListCompleted(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorString);


	/**
	* Delegate function fired when a session invite is accepted to join the session
	* @param    bWasSuccessful true if the async action completed without error, false if there was an error
	* @param	LocalUserNum	The Local user Number of the player who recived the invite
	* @param	InvitingPlayer	The inviting player
	* @param	TheSessionInvitedTo		the session invited to
	*/
	void OnSessionUserInviteAccepted(bool bWasSuccessful, int32 LocalUserNum, TSharedPtr<const FUniqueNetId> InvitingPlayer, const FOnlineSessionSearchResult& TheSessionInvitedTo);

	/** Handle to registered delegate for accepting an invite */
	FDelegateHandle OnSessionUserInviteAcceptedDelegateHandle;
	/** Delegate for accepting invite */
	FOnSessionUserInviteAcceptedDelegate OnSessionUserInviteAcceptedDelegate;
protected:
	/**
	*	called when the session search is complete to show the results in UMG
	*/
	DECLARE_EVENT_OneParam(AFusionGameInstance, FOnFoundSessionsCompleteUMG, const TArray<FCustomFusionSessionResult>&);
	FOnFoundSessionsCompleteUMG FoundSessionsCompleteUMGEvent;

	void OnFoundSessionsCompleteUMG(const TArray<FCustomFusionSessionResult>& CustomSessionResults);


	/**
	* Called from the delegate when getting the friend list request in completed
	*/
	DECLARE_EVENT_OneParam(AFusionGameInstance, FOnGetSteamFriendRequestCompleteUMG, const TArray<FSteamFriendInfo>&);
	FOnGetSteamFriendRequestCompleteUMG OnGetSteamFriendRequestCompleteUMGEvent;

	void OnGetSteamFriendRequestCompleteUMG(const TArray<FSteamFriendInfo>& BPFriendsLists);



	/**
	* Called to show an error message in UMG
	*/
	DECLARE_EVENT_OneParam(AFusionGameInstance, FOnShowErrorMessageUMG, const FText&);
	FOnShowErrorMessageUMG OnShowErrorMessageUMGEvent;

	void OnShowErrorMessageUMG(const FText & ErrorMessage);

public:
	
	FOnFoundSessionsCompleteUMG& OnFoundSessionsCompleteUMG() { return FoundSessionsCompleteUMGEvent; }
	
	FOnGetSteamFriendRequestCompleteUMG& OnGetSteamFriendRequestCompleteUMG() { return OnGetSteamFriendRequestCompleteUMGEvent; }

	FOnShowErrorMessageUMG& OnShowErrorMessageUMG() { return OnShowErrorMessageUMGEvent; }

	UFUNCTION(Exec)
	void TestErrorMsg(FText Msg);


	/** Delegate for destroying a session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	/**
	* Delegate fired when a destroying an online session has completed
	*
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	FDelegateHandle OnDestroySessionCompleteDelegateHandle;

	/**
	* gets the max number of players in the session
	* @return	max number of players in the session
	*/

	UFUNCTION()
	FORCEINLINE int32 GetSessionMaxPlayers() const { return MaxPlayersinSession; }

	//store the max number of players in a session whenever we create of join a session
	int32 MaxPlayersinSession;

private:



	UPROPERTY(config)
	FString WelcomeScreenMap;

	UPROPERTY(config)
	FString MainMenuMap;
	
	UPROPERTY()
	FName MainMenuMapp = "/Game/Maps/FusionEntry";


	FName CurrentState;
	FName PendingState;

	FFusionPendingMessage PendingMessage;

	FFusionPendingInvite PendingInvite;

	/** URL to travel to after pending network operations */
	FString TravelURL;

	/** Whether the match is online or not */
	bool bIsOnline;

	/** If true, enable splitscreen when map starts loading */
	bool bPendingEnableSplitscreen;

	/** Whether the user has an active license to play the game */
	bool bIsLicensed;

	/** Main menu UI */
	
	TWeakObjectPtr<class UMainMenuUI> MainMenuUI;

	/** Message menu (Shown in the even of errors - unable to connect etc) */
	TWeakObjectPtr<class UFusionMessageMenu_Widget> MessageMenuUI;


	//TSharedPtr<class SWidget> LobbyWidget;
	TWeakObjectPtr<class ULobbyMenu_Widget> LobbyWidget;


	/** Controller to ignore for pairing changes. -1 to skip ignore. */
	int32 IgnorePairingChangeForControllerId;

	/** Last connection status that was passed into the HandleNetworkConnectionStatusChanged hander */
	EOnlineServerConnectionStatus::Type	CurrentConnectionStatus;

	/** Delegate for callbacks to Tick */
	FTickerDelegate TickDelegate;

	/** Handle to various registered delegates */
	FDelegateHandle TickDelegateHandle;
	FDelegateHandle TravelLocalSessionFailureDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnEndSessionCompleteDelegateHandle;
	FDelegateHandle OnCreatePresenceSessionCompleteDelegateHandle;


	void HandleNetworkConnectionStatusChanged(EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus);

	void HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType);

	void OnPostLoadMap();



	/** Delegate for ending a session */
	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;

	void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	void MaybeChangeState();
	void EndCurrentState(FName NextState);
	void BeginNewState(FName NewState, FName PrevState);


	void BeginMainMenuState();
	void BeginMessageMenuState();
	void BeginLobbyState();
	void BeginPlayingState();

	void EndMainMenuState();
	void EndMessageMenuState();
	void EndLobbyState();
	void EndPlayingState();

	void ShowLoadingScreen();
	void AddNetworkFailureHandlers();
	void RemoveNetworkFailureHandlers();

	/** Called when there is an error trying to travel to a local session */
	void TravelLocalSessionFailure(UWorld *World, ETravelFailure::Type FailureType, const FString& ErrorString);


	/**
	* Creates the message menu, clears other menus and sets the KingState to Message.
	*
	* @param	Message				Main message body
	*/
	void ShowMessageThenGoMain(const FText& Message);


	bool LoadFrontEndMap(const FString& MapName);

	/** Sets a rich presence string for all local players. */
	void SetPresenceForLocalPlayers(const FVariantData& PresenceData);


	// Callback to process game licensing change notifications.
	void HandleAppLicenseUpdate();

	// Callback to handle safe frame size changes.
	void HandleSafeFrameChanged();

protected:
	bool HandleOpenCommand(const TCHAR* Cmd, FOutputDevice& Ar, UWorld* InWorld);

};