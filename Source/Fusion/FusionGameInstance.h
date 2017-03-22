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
	extern const FName PendingInvite;
	extern const FName WelcomeScreen;
	extern const FName MainMenu;
	extern const FName MessageMenu;
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

struct FFusionPlayTogetherInfo
{
	FFusionPlayTogetherInfo() : UserIndex(-1) {}
	FFusionPlayTogetherInfo(int32 InUserIndex, const TArray<TSharedPtr<const FUniqueNetId>>& InUserIdList)
		: UserIndex(InUserIndex)
	{
		UserIdList.Append(InUserIdList);
	}

	int32 UserIndex;
	TArray<TSharedPtr<const FUniqueNetId>> UserIdList;
};

//A custom struct to be able to access the Session results in blueprint
USTRUCT(BlueprintType)
struct FCustomBlueprintSessionResult
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

	bool HostGame(ULocalPlayer* LocalPlayer, const FString& GameType, const FString& InTravelURL);
	bool JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults);
	bool JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult);
	void SetPendingInvite(const FFusionPendingInvite& InPendingInvite);

	bool PlayDemo(ULocalPlayer* LocalPlayer, const FString& DemoName);

	/** Travel directly to the named session */
	void TravelToSession(const FName& SessionName);

	/** Begin a hosted quick match */
	void BeginHostingQuickMatch();

	/** Initiates the session searching */
	bool FindSessions(ULocalPlayer* PlayerOwner, bool bLANMatch);

	/** Sends the game to the specified state. */
	void GotoState(FName NewState);

	/** Obtains the initial welcome state, which can be different based on platform */
	FName GetInitialState();

	/** Sends the game to the initial startup/frontend state  */
	void GotoInitialState();

	/**
	* Creates the message menu, clears other menus and sets the KingState to Message.
	*
	* @param	Message				Main message body
	* @param	OKButtonString		String to use for 'OK' button
	* @param	CancelButtonString	String to use for 'Cancel' button
	* @param	NewState			Final state to go to when message is discarded
	*/
	void ShowMessageThenGotoState(const FText& Message, const FText& OKButtonString, const FText& CancelButtonString, const FName& NewState, const bool OverrideExisting = true, TWeakObjectPtr< ULocalPlayer > PlayerOwner = nullptr);

	void RemoveExistingLocalPlayer(ULocalPlayer* ExistingPlayer);

	void RemoveSplitScreenPlayers();

	TSharedPtr< const FUniqueNetId > GetUniqueNetIdFromControllerId(const int ControllerId);

	/** Returns true if the game is in online mode */
	bool GetIsOnline() const { return bIsOnline; }

	/** Sets the online mode of the game */
	void SetIsOnline(bool bInIsOnline);

	/** Sets the controller to ignore for pairing changes. Useful when we are showing external UI for manual profile switching. */
	void SetIgnorePairingChangeForControllerId(const int32 ControllerId);

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

	/** Called when we receive a Play Together system event on PS4 */
	void OnPlayTogetherEventReceived(const int32 UserIndex, const TArray<TSharedPtr<const FUniqueNetId>>& UserIdList);

	/** Resets Play Together PS4 system event info after it's been handled */
	void ResetPlayTogetherInfo() { PlayTogetherInfo = FFusionPlayTogetherInfo(); }

	
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
	DECLARE_EVENT_OneParam(AFusionGameInstance, FOnFoundSessionsCompleteUMG, const TArray<FCustomBlueprintSessionResult>&);
	FOnFoundSessionsCompleteUMG FoundSessionsCompleteUMGEvent;

	void OnFoundSessionsCompleteUMG(const TArray<FCustomBlueprintSessionResult>& CustomSessionResults);


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
	//TSharedPtr<FShooterMainMenu> MainMenuUI;
	TWeakObjectPtr<class UMainMenuUI> MainMenuUI;

	/** Message menu (Shown in the even of errors - unable to connect etc) */
	//TSharedPtr<FShooterMessageMenu> MessageMenuUI;

	/** Welcome menu UI (for consoles) */
	//TSharedPtr<FShooterWelcomeMenu> WelcomeMenuUI;

	/** Dialog widget to show non-interactive waiting messages for network timeouts and such. */
	//TSharedPtr<SShooterWaitDialog> WaitMessageWidget;

	/** Controller to ignore for pairing changes. -1 to skip ignore. */
	int32 IgnorePairingChangeForControllerId;

	/** Last connection status that was passed into the HandleNetworkConnectionStatusChanged hander */
	EOnlineServerConnectionStatus::Type	CurrentConnectionStatus;

	/** Delegate for callbacks to Tick */
	FTickerDelegate TickDelegate;

	/** Handle to various registered delegates */
	FDelegateHandle TickDelegateHandle;
	FDelegateHandle TravelLocalSessionFailureDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnSearchSessionsCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnEndSessionCompleteDelegateHandle;
	//FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	FDelegateHandle OnCreatePresenceSessionCompleteDelegateHandle;

	/** Play Together on PS4 system event info */
	FFusionPlayTogetherInfo PlayTogetherInfo;

	void HandleNetworkConnectionStatusChanged(EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus);

	void HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType);

	void OnPreLoadMap(const FString& MapName);
	void OnPostLoadMap();
	void OnPostDemoPlay();

	virtual void HandleDemoPlaybackFailure(EDemoPlayFailure::Type FailureType, const FString& ErrorString) override;

	/** Delegate function executed after checking privileges for starting quick match */
	void OnUserCanPlayInvite(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	/** Delegate function executed after checking privileges for Play Together on PS4 */
	void OnUserCanPlayTogether(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	/** Delegate for ending a session */
	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;

	void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	void MaybeChangeState();
	void EndCurrentState(FName NextState);
	void BeginNewState(FName NewState, FName PrevState);

	void BeginPendingInviteState();
	void BeginWelcomeScreenState();
	void BeginMainMenuState();
	void BeginMessageMenuState();
	void BeginPlayingState();

	void EndPendingInviteState();
	void EndWelcomeScreenState();
	void EndMainMenuState();
	void EndMessageMenuState();
	void EndPlayingState();

	void ShowLoadingScreen();
	void AddNetworkFailureHandlers();
	void RemoveNetworkFailureHandlers();

	/** Called when there is an error trying to travel to a local session */
	void TravelLocalSessionFailure(UWorld *World, ETravelFailure::Type FailureType, const FString& ErrorString);

	/** Callback which is intended to be called upon joining session */
	void OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);

	/** Callback which is intended to be called upon session creation */
	void OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful);

	/** Callback which is called after adding local users to a session */
	void OnRegisterLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);

	/** Called after all the local players are registered */
	void FinishSessionCreation(EOnJoinSessionCompleteResult::Type Result);

	/** Callback which is called after adding local users to a session we're joining */
	void OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);

	/** Called after all the local players are registered in a session we're joining */
	void FinishJoinSession(EOnJoinSessionCompleteResult::Type Result);

	/** Send all invites for the current game session if we've created it because Play Together on PS4 was initiated*/
	void SendPlayTogetherInvites();

	/**
	* Creates the message menu, clears other menus and sets the KingState to Message.
	*
	* @param	Message				Main message body
	* @param	OKButtonString		String to use for 'OK' button
	* @param	CancelButtonString	String to use for 'Cancel' button
	*/
	void ShowMessageThenGoMain(const FText& Message, const FText& OKButtonString, const FText& CancelButtonString);

	/** Callback which is intended to be called upon finding sessions */
	void OnSearchSessionsComplete(bool bWasSuccessful);

	bool LoadFrontEndMap(const FString& MapName);

	/** Sets a rich presence string for all local players. */
	void SetPresenceForLocalPlayers(const FVariantData& PresenceData);

	/** Travel directly to the named session */
	void InternalTravelToSession(const FName& SessionName);

	/** Show messaging and punt to welcome screen */
	void HandleSignInChangeMessaging();

	// OSS delegates to handle
	void HandleUserLoginChanged(int32 GameUserIndex, ELoginStatus::Type PreviousLoginStatus, ELoginStatus::Type LoginStatus, const FUniqueNetId& UserId);

	// Callback to pause the game when the OS has constrained our app.
	void HandleAppWillDeactivate();

	// Callback occurs when game being suspended
	void HandleAppSuspend();

	// Callback occurs when game resuming
	void HandleAppResume();

	// Callback to process game licensing change notifications.
	void HandleAppLicenseUpdate();

	// Callback to handle safe frame size changes.
	void HandleSafeFrameChanged();

	// Callback to handle controller connection changes.
	void HandleControllerConnectionChange(bool bIsConnection, int32 Unused, int32 GameUserIndex);

	// Callback to handle controller pairing changes.
	FReply OnPairingUsePreviousProfile();

	// Callback to handle controller pairing changes.
	FReply OnPairingUseNewProfile();

	// Callback to handle controller pairing changes.
	void HandleControllerPairingChanged(int GameUserIndex, const FUniqueNetId& PreviousUser, const FUniqueNetId& NewUser);

	// Handle confirming the controller disconnected dialog.
	FReply OnControllerReconnectConfirm();

protected:
	bool HandleOpenCommand(const TCHAR* Cmd, FOutputDevice& Ar, UWorld* InWorld);

};