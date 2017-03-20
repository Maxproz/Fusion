// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameSession.h"
#include "FusionGameSession.generated.h"

struct FFusionGameSessionParams
{
	/** Name of session settings are stored with */
	FName SessionName;
	/** LAN Match */
	bool bIsLAN;
	/** Presence enabled session */
	bool bIsPresence;
	/** Id of player initiating lobby */
	TSharedPtr<const FUniqueNetId> UserId;
	/** Current search result choice to join */
	int32 BestSessionIdx;

	FFusionGameSessionParams()
		: SessionName(NAME_None)
		, bIsLAN(false)
		, bIsPresence(false)
		, BestSessionIdx(0)
	{
	}
};

/**
 * 
 */
UCLASS()
class FUSION_API AFusionGameSession : public AGameSession
{
	//GENERATED_BODY()
	
	
	GENERATED_UCLASS_BODY()

public:
	/** Delegate for destroying a session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	
protected:

	/** Delegate for creating a new session */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	/** Delegate after starting a session */
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	/** Delegate for searching for sessions */
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	/** Delegate after joining a session */
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Online")
	int32 MinServerNameLength = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Online")
	int32 MaxServerNameLength = 30;


	/** Transient properties of a session during game creation/matchmaking */
	FFusionGameSessionParams CurrentSessionParams;



	/**
	* Delegate fired when a session create request has completed
	*
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	* Delegate fired when a session start request has completed
	*
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);

	/**
	* Delegate fired when a session search query has completed
	*
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnFindSessionsComplete(bool bWasSuccessful);

	/**
	* Delegate fired when a session join request has completed
	*
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/**
	* Delegate fired when a destroying an online session has completed
	*
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);



	/*
	* Event triggered when a presence session is created
	*
	* @param SessionName name of session that was created
	* @param bWasSuccessful was the create successful
	*/
	DECLARE_EVENT_TwoParams(AFusionGameSession, FOnCreatePresenceSessionComplete, FName /*SessionName*/, bool /*bWasSuccessful*/);
	FOnCreatePresenceSessionComplete CreatePresenceSessionCompleteEvent;

	/*
	* Event triggered when a session is joined
	*
	* @param SessionName name of session that was joined
	* @param bWasSuccessful was the create successful
	*/
	//DECLARE_DELEGATE_RetVal_TwoParams(bool, FOnJoinSessionComplete, FName /*SessionName*/, bool /*bWasSuccessful*/);
	DECLARE_EVENT_OneParam(AFusionGameSession, FOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type /*Result*/);
	FOnJoinSessionComplete JoinSessionCompleteEvent;

	/*
	* Event triggered after session search completes
	*/
	//DECLARE_EVENT(AFusionGameSession, FOnFindSessionsComplete);
	DECLARE_EVENT_OneParam(AFusionGameSession, FOnFindSessionsComplete, bool /*bWasSuccessful*/);
	FOnFindSessionsComplete FindSessionsCompleteEvent;




public:

	/** Default number of players allowed in a game */
	static const int32 DEFAULT_NUM_PLAYERS = 8;
	
	//store the max number of players in a session whenever we create of join a session
	int32 MaxPlayersinSession;


	TSharedPtr<class FOnlineSessionSettings> SessionSettings;

	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	//store the chosen search result for later use
	FOnlineSessionSearchResult ChosenSearchResult;


	/**
	* gets the max number of players in the session
	* @return	max number of players in the session
	*/
	UFUNCTION()
	FORCEINLINE int32 GetSessionMaxPlayers() const { return MaxPlayersinSession; }


	bool HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, FString ServerName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers, bool bIsPasswordProtected, FString SessionPassword);


	void FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence);

	// New
	bool JoinASession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);
	bool JoinASession(int32 LocalUserNum, FName SessionName, const FOnlineSessionSearchResult& SearchResult);
	




	/** @return true if any online async work is in progress, false otherwise */
	bool IsBusy() const;

	/**
	* Get the search results found and the current search result being probed
	*
	* @param SearchResultIdx idx of current search result accessed
	* @param NumSearchResults number of total search results found in FindGame()
	*
	* @return State of search result query
	*/
	EOnlineAsyncTaskState::Type GetSearchResultStatus(int32& SearchResultIdx, int32& NumSearchResults);

	/**
	* Get the search results.
	*
	* @return Search results
	*/
	const TArray<FOnlineSessionSearchResult> & GetSearchResults() const;

	/** @return the delegate fired when creating a presence session */
	FOnCreatePresenceSessionComplete& OnCreatePresenceSessionComplete() { return CreatePresenceSessionCompleteEvent; }

	/** @return the delegate fired when joining a session */
	FOnJoinSessionComplete& OnJoinSessionComplete() { return JoinSessionCompleteEvent; }

	/** @return the delegate fired when search of session completes */
	FOnFindSessionsComplete& OnFindSessionsComplete() { return FindSessionsCompleteEvent; }


	/** Handles when the match has ended */
	virtual void HandleMatchHasEnded() override;


	/** Handles to various registered delegates */
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	
	UPROPERTY()
	FName LobbyMapName = "/Game/Maps/Lobby";

	
	UPROPERTY()
	FName MainMenuMap = "/Game/Maps/FusionEntry";

};

