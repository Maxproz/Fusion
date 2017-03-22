// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionPlayerController.h"

#include "FusionGameInstance.h"

#include "Online/FusionOnlineGameSettings.h"

#include "FusionGameSession.h"




namespace
{
	const FString CustomMatchKeyword("Custom");
}

AFusionGameSession::AFusionGameSession(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &AFusionGameSession::OnCreateSessionComplete);
	

		OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &AFusionGameSession::OnFindSessionsComplete);
		OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &AFusionGameSession::OnJoinSessionComplete);

		OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &AFusionGameSession::OnStartOnlineGameComplete);



	}
}

/**
* Ends a game session
*
*/


void AFusionGameSession::HandleMatchHasEnded()
{
	
	
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// tell the clients to end
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				AFusionPlayerController* PC = Cast<AFusionPlayerController>(*It);
				if (PC && !PC->IsLocalPlayerController())
				{
					PC->ClientEndOnlineGame();
				}
			}

			/*
			// server is handled here
			UE_LOG(LogOnlineGame, Log, TEXT("Ending session %s on server"), *GameSessionName.ToString());
			Sessions->EndSession(GameSessionName);*/
		}
	}
}


void AFusionGameSession::HandleMatchHasStarted()
{

	
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				AFusionPlayerController* PC = Cast<AFusionPlayerController>(*It);
				if (PC && !PC->IsLocalPlayerController())
				{
					PC->ClientStartOnlineGame();
				}
			}
			/*
			UE_LOG(LogOnlineGame, Log, TEXT("Starting session %s on server"), *GameSessionName.ToString());
			OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
			Sessions->StartSession(GameSessionName);
			*/
		}
	}
	
}



bool AFusionGameSession::IsBusy() const
{
	/*
	if (HostSettings.IsValid() || SearchSettings.IsValid())
	{
		return true;
	}*/

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			EOnlineSessionState::Type GameSessionState = Sessions->GetSessionState(GameSessionName);
			EOnlineSessionState::Type PartySessionState = Sessions->GetSessionState(PartySessionName);
			if (GameSessionState != EOnlineSessionState::NoSession || PartySessionState != EOnlineSessionState::NoSession)
			{
				return true;
			}
		}
	}

	return false;
}

EOnlineAsyncTaskState::Type AFusionGameSession::GetSearchResultStatus(int32& SearchResultIdx, int32& NumSearchResults)
{
	SearchResultIdx = 0;
	NumSearchResults = 0;

	if (SessionSearch.IsValid())
	{
		if (SessionSearch->SearchState == EOnlineAsyncTaskState::Done)
		{
			SearchResultIdx = CurrentSessionParams.BestSessionIdx;
			NumSearchResults = SessionSearch->SearchResults.Num();
		}
		return SessionSearch->SearchState;
	}

	return EOnlineAsyncTaskState::NotStarted;
}

/**
* Get the search results.
*
* @return Search results
*/
const TArray<FOnlineSessionSearchResult> & AFusionGameSession::GetSearchResults() const
{
	return SessionSearch->SearchResults;
};



































bool AFusionGameSession::HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, FString ServerName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers, bool bIsPasswordProtected, FString SessionPassword)
{
	// Get the Online Subsystem to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the Session Interface, so we can call the "CreateSession" function on it
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && UserId.IsValid())
		{
			/*
			Fill in all the Session Settings that we want to use.
			There are more with SessionSettings.Set(...);
			For example the Map or the GameMode/Type.
			*/
			SessionSettings = MakeShareable(new FOnlineSessionSettings());
			SessionSettings->bIsLANMatch = bIsLAN;
			SessionSettings->bUsesPresence = bIsPresence;
			SessionSettings->NumPublicConnections = MaxNumPlayers;
			
			Cast<UFusionGameInstance>(GetGameInstance())->MaxPlayersinSession = MaxNumPlayers;
			//MaxPlayersinSession = MaxNumPlayers;
			SessionSettings->NumPrivateConnections = 0;
			SessionSettings->bAllowInvites = true;
			SessionSettings->bAllowJoinInProgress = true;
			SessionSettings->bShouldAdvertise = true;
			SessionSettings->bAllowJoinViaPresence = true;
			SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
			//setting a value in the FOnlineSessionSetting 's settings array
			SessionSettings->Set(SETTING_MAPNAME, LobbyMapName.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);

			//Making a temporary FOnlineSessionSetting variable to hold the data we want to add to the FOnlineSessionSetting 's settings array
			FOnlineSessionSetting ExtraSessionSetting;
			ExtraSessionSetting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;

			//setting the temporary data to the ServerName we got from UMG
			ExtraSessionSetting.Data = ServerName;

			//adding the Server Name value in the FOnlineSessionSetting 's settings array using the key defined in header
			//the key can be any FNAME but we define it to avoid mistakes
			SessionSettings->Settings.Add(SETTING_SERVER_NAME, ExtraSessionSetting);

			//setting the temporary data to the bIsPasswordProtected we got from UMG
			ExtraSessionSetting.Data = bIsPasswordProtected;
			//adding the bIsPasswordProtected value in the FOnlineSessionSetting 's settings array using the key defined in header
			SessionSettings->Settings.Add(SETTING_SERVER_IS_PROTECTED, ExtraSessionSetting);


			//setting the temporary data to the Password we got from UMG
			ExtraSessionSetting.Data = SessionPassword;
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, *ExtraSessionSetting.Data.ToString());
			//adding the Password value in the FOnlineSessionSetting 's settings array using the key defined in header
			SessionSettings->Settings.Add(SETTING_SERVER_PROTECT_PASSWORD, ExtraSessionSetting);



			// Set the delegate to the Handle of the SessionInterface
			OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
			// Our delegate should get called when this is complete (doesn't need to be successful!)
			return Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));
	}
	return false;


	/*
	#if !UE_BUILD_SHIPPING
	else
	{
	// Hack workflow in development
	OnCreatePresenceSessionComplete().Broadcast(GameSessionName, true);
	return true;
	}
	#endif
	*/ // This was in the else* before.
}

void AFusionGameSession::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnCreateSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));
	// Get the OnlineSubsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the Session Interface to call the StartSession function
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// Clear the SessionComplete delegate handle, since we finished this call
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);

			if (bWasSuccessful)
			{
				// This was in HandleMatchHasStarted

				// Should I remove this since I have it inside of handle match has started?
				UE_LOG(LogOnlineGame, Log, TEXT("Starting session %s on server"), *GameSessionName.ToString());
				OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
				Sessions->StartSession(SessionName);
			}
		}
	}
	//OnCreatePresenceSessionComplete().Broadcast(SessionName, bWasSuccessful);
}



void AFusionGameSession::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnStartSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));
	// Get the Online Subsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the Session Interface to clear the Delegate
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// Clear the delegate, since we are done with this call
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}
	
	if (bWasSuccessful)
	{

		// If the start was successful, we can open a NewMap if we want. Make sure to use "listen" as a parameter!
		UGameplayStatics::OpenLevel(GetWorld(), LobbyMapName, true, "listen");
	}
}


void AFusionGameSession::FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence)
{
	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the SessionInterface from our OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && UserId.IsValid())
		{
			/*
			Fill in all the SearchSettings, like if we are searching for a LAN game and how many results we want to have!
			*/
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = bIsLAN;
			SessionSearch->MaxSearchResults = 20;
			SessionSearch->PingBucketSize = 50;

			// We only want to set this Query Setting if "bIsPresence" is true
			if (bIsPresence)
			{
				SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
			}
			TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();
			// Set the Delegate to the Delegate Handle of the FindSession function
			OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

			// Finally call the SessionInterface function. The Delegate gets called once this is finished
			Sessions->FindSessions(*UserId, SearchSettingsRef);
		}
	}
	else
	{
		// If something goes wrong, just call the Delegate Function directly with "false".
		OnFindSessionsComplete(false);
	}
}


void AFusionGameSession::OnFindSessionsComplete(bool bWasSuccessful)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OFindSessionsComplete bSuccess: %d"), bWasSuccessful));
	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get SessionInterface of the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// Clear the Delegate handle, since we finished this call
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
			// Just debugging the Number of Search results. Can be displayed in UMG or something later on
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num Search Results: %d"), SessionSearch->SearchResults.Num()));

			TArray<FCustomBlueprintSessionResult> CustomSessionResults;

			// If we have found at least 1 session, we just going to debug them. You could add them to a list of UMG Widgets, like it is done in the BP version!
			if (SessionSearch->SearchResults.Num() > 0)
			{
				//ULocalPlayer* const Player = GetFirstGamePlayer();



				// "SessionSearch->SearchResults" is an Array that contains all the information. You can access the Session in this and get a lot of information.
				// This can be customized later on with your own classes to add more information that can be set and displayed
				for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
				{

					//temporary Session result to hold our data for this loop
					FCustomBlueprintSessionResult TempCustomSeesionResult;

					//uncomment if you want the session name to always be the name of the owning player (Computer name on lan and Steam name online)
					TempCustomSeesionResult.SessionName = SessionSearch->SearchResults[SearchIdx].Session.OwningUserName;
					TempCustomSeesionResult.bIsLan = SessionSearch->SearchResults[SearchIdx].Session.SessionSettings.bIsLANMatch;
					TempCustomSeesionResult.CurrentNumberOfPlayers = SessionSearch->SearchResults[SearchIdx].Session.SessionSettings.NumPublicConnections - SessionSearch->SearchResults[SearchIdx].Session.NumOpenPublicConnections;
					TempCustomSeesionResult.MaxNumberOfPlayers = SessionSearch->SearchResults[SearchIdx].Session.SessionSettings.NumPublicConnections;
					TempCustomSeesionResult.Ping = SessionSearch->SearchResults[SearchIdx].PingInMs;

					// get the server name
					SessionSearch->SearchResults[SearchIdx].Session.SessionSettings.Get(SETTING_SERVER_NAME, TempCustomSeesionResult.SessionName);


					// get if the server is password protected
					SessionSearch->SearchResults[SearchIdx].Session.SessionSettings.Get(SETTING_SERVER_IS_PROTECTED, TempCustomSeesionResult.bIsPasswordProtected);


					// get the Password if the session is Password Protected
					if (TempCustomSeesionResult.bIsPasswordProtected)
						SessionSearch->SearchResults[SearchIdx].Session.SessionSettings.Get(SETTING_SERVER_PROTECT_PASSWORD, TempCustomSeesionResult.SessionPassword);

					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *TempCustomSeesionResult.SessionPassword);



					CustomSessionResults.Add(TempCustomSeesionResult);

				}
			}

			//call UMG to show sessions after the search ends
			UFusionGameInstance* FGI = Cast<UFusionGameInstance>(GetGameInstance());
			FGI->OnFoundSessionsCompleteUMG().Broadcast(CustomSessionResults);
		}
	}
}


bool AFusionGameSession::JoinASession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult)
{
	// Return bool
	bool bSuccessful = false;
	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && UserId.IsValid())
		{
			// Set the Handle again
			OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

			// Call the "JoinSession" Function with the passed "SearchResult". The "SessionSearch->SearchResults" can be used to get such a
			// "FOnlineSessionSearchResult" and pass it. Pretty straight forward!
			bSuccessful = Sessions->JoinSession(*UserId, SessionName, SearchResult);
		}
	}

	return bSuccessful;
}



bool AFusionGameSession::JoinASession(int32 LocalUserNum, FName SessionName, const FOnlineSessionSearchResult& SearchResult)
{
	// Return bool
	bool bSuccessful = false;
	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// Set the Handle again
			OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

			// Call the "JoinSession" Function with the passed "SearchResult". The "SessionSearch->SearchResults" can be used to get such a
			// "FOnlineSessionSearchResult" and pass it. Pretty straight forward!
			bSuccessful = Sessions->JoinSession(LocalUserNum, SessionName, SearchResult);
		}
	}

	return bSuccessful;
}


void AFusionGameSession::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnJoinSessionComplete %s, %d"), *SessionName.ToString(), static_cast<int32>(Result)));
	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// Clear the Delegate again
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
			// Get the first local PlayerController, so we can call "ClientTravel" to get to the Server Map
			// This is something the Blueprint Node "Join Session" does automatically!
			APlayerController * const PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0); // ewww
			// We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
			// String for us by giving him the SessionName and an empty String. We want to do this, because
			// Every OnlineSubsystem uses different TravelURLs
			FString TravelURL;
			if (PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL))
			{
				// Finally call the ClienTravel. If you want, you could print the TravelURL to see
				// how it really looks like
				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}



