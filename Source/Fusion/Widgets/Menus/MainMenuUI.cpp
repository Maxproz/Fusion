// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameLoadingScreen.h"

#include "FusionGameInstance.h"
#include "FusionGameSession.h"

#include "Online/FusionOnlineGameSettings.h"

#include "MainMenuUI.h"



#define LOCTEXT_NAMESPACE "Fusion.HUD.Menu"


void UMainMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	HostButton->OnClicked.AddDynamic(this, &UMainMenuUI::OnClickedHostButton);
	JoinButton->OnClicked.AddDynamic(this, &UMainMenuUI::OnClickedJoinButton);
	QuitButton->OnClicked.AddDynamic(this, &UMainMenuUI::OnClickedQuitButton);

	PlayerOwner = GetOwningLocalPlayer();
	MapFilterName = "Any";
	bSearchingForServers = false;
	bLANMatchSearch = false;

	// Note: CreateSP wouldn't work with my current setup, so I had to change it to UObject. No idea what the consequences are.
	//OnMatchmakingCompleteDelegate = FOnMatchmakingCompleteDelegate::CreateUObject(this, &UMainMenuUI::OnMatchmakingComplete);
	OnMatchmakingCompleteDelegate = FOnMatchmakingCompleteDelegate::CreateUObject(this, &UMainMenuUI::OnMatchmakingComplete);
}

void UMainMenuUI::ShowMainMenu()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UMainMenuUI::HideMainMenu()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UMainMenuUI::DisplayLoadingScreen()
{
	IFusionGameLoadingScreenModule* LoadingScreenModule = FModuleManager::LoadModulePtr<IFusionGameLoadingScreenModule>("FusionGameLoadingScreen");
	if (LoadingScreenModule != NULL)
	{
		LoadingScreenModule->StartInGameLoadingScreen();
	}
}

void UMainMenuUI::OnClickedHostButton()
{

	ULocalPlayer* const Player = GetOwningLocalPlayer();
	UFusionGameInstance* GameInstance = Cast<UFusionGameInstance>(Player->GetGameInstance());


	HostTeamDeathMatch();

}



void UMainMenuUI::OnClickedJoinButton()
{
	ULocalPlayer* const Player = GetOwningLocalPlayer();
	UFusionGameInstance* GameInstance = Cast<UFusionGameInstance>(Player->GetGameInstance());

	ConnectToServer();

	//GameInstance->JoinOnlineGame();
	//BeginQuickMatchSearch();
}

void UMainMenuUI::OnClickedQuitButton()
{
	ULocalPlayer* const Player = GetOwningLocalPlayer();
	UFusionGameInstance* GameInstance = Cast<UFusionGameInstance>(Player->GetGameInstance());
	//Quit();
	//GameInstance->FindOnlineGames();


	BeginServerSearch(false, "Downfall");
}

void UMainMenuUI::HostGame(const FString& GameType)
{
	ULocalPlayer* const Player = GetOwningLocalPlayer();	
	UFusionGameInstance* GameInstance = Cast<UFusionGameInstance>(Player->GetGameInstance());

	//if (ensure(IsValid(GameInstance) && Player != NULL))
	if (Player != NULL)
	{
		bool bIsLanMatch = false;
		FString BotNames = FString(TEXT("Bots"));
		int32 AmountOfBots = 0;
		bool bIsRecordingDemo = false;
		FString MapName = TEXT("Downfall");

		//GameInstance->SetIsOnline(true);

		//FString const StartURL = TEXT("/Game/Maps/Downfall?game=TDM?listen");
		FString const StartURL = FString::Printf(TEXT("/Game/Maps/%s?game=%s%s%s?%s=%d%s"), *MapName, *GameType, GameInstance->GetIsOnline() ? TEXT("?listen") : TEXT(""), bIsLanMatch ? TEXT("?bIsLanMatch") : TEXT(""), *BotNames, AmountOfBots, bIsRecordingDemo ? TEXT("?DemoRec") : TEXT(""));
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("StartURL: %s"), *StartURL));
		
		// Game instance will handle success, failure and dialogs
		GameInstance->HostGame(Player, GameType, StartURL); // This doesn't work at all.

	}
}

/*
void UMainMenuUI::HostGame(const FString& GameType)
{
	if (ensure(GameInstance.IsValid()) && GetPlayerOwner() != NULL)
	{
		FString const StartURL = FString::Printf(TEXT("/Game/Maps/%s?game=%s%s%s?%s=%d%s"), *GetMapName(), *GameType, GameInstance->GetIsOnline() ? TEXT("?listen") : TEXT(""), bIsLanMatch ? TEXT("?bIsLanMatch") : TEXT(""), *AShooterGameMode::GetBotsCountOptionName(), BotsCountOpt, bIsRecordingDemo ? TEXT("?DemoRec") : TEXT(""));

		// Game instance will handle success, failure and dialogs
		GameInstance->HostGame(GetPlayerOwner(), GameType, StartURL);
	}
}*/

void UMainMenuUI::HostFreeForAll()
{
	HostGame(LOCTEXT("FFA", "FFA").ToString());
}

void UMainMenuUI::HostTeamDeathMatch()
{
	HostGame(LOCTEXT("TDM", "TDM").ToString());
}

// The find sessions function inside of the game instance is crashing me so I need to implement the function below 
//After I get it working here, I can begin moving it into a main menu widget.
void UMainMenuUI::BeginQuickMatchSearch()
{
	ULocalPlayer* const Player = GetOwningLocalPlayer();
	UFusionGameInstance* GameInstance = Cast<UFusionGameInstance>(Player->GetGameInstance());

	auto Sessions = Online::GetSessionInterface();
	if (!Sessions.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("Quick match is not supported: couldn't find online session interface."));
		return;
	}

	if (GetOwningLocalPlayer()->GetControllerId() == -1)
	{
		UE_LOG(LogOnline, Warning, TEXT("Quick match is not supported: Could not get controller id from player owner"));
		return;
	}

	QuickMatchSearchSettings = MakeShareable(new FFusionOnlineSearchSettings(false, true));
	QuickMatchSearchSettings->QuerySettings.Set(SEARCH_XBOX_LIVE_HOPPER_NAME, FString("TeamDeathmatch"), EOnlineComparisonOp::Equals);
	QuickMatchSearchSettings->QuerySettings.Set(SEARCH_XBOX_LIVE_SESSION_TEMPLATE_NAME, FString("MatchSession"), EOnlineComparisonOp::Equals);
	QuickMatchSearchSettings->TimeoutInSeconds = 120.0f;

	FFusionOnlineSessionSettings SessionSettings(false, true, 8);
	SessionSettings.Set(SETTING_GAMEMODE, FString("TDM"), EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"), EOnlineDataAdvertisementType::DontAdvertise);
	SessionSettings.Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineDataAdvertisementType::DontAdvertise);

	auto QuickMatchSearchSettingsRef = QuickMatchSearchSettings.ToSharedRef();

	//DisplayQuickmatchSearchingUI();

	Sessions->ClearOnMatchmakingCompleteDelegate_Handle(OnMatchmakingCompleteDelegateHandle);
	OnMatchmakingCompleteDelegateHandle = Sessions->AddOnMatchmakingCompleteDelegate_Handle(OnMatchmakingCompleteDelegate);

	// Perform matchmaking with all local players
	TArray<TSharedRef<const FUniqueNetId>> LocalPlayerIds;
	for (int i = 0; i < GameInstance->GetNumLocalPlayers(); ++i)
	{
		auto PlayerId = GameInstance->GetLocalPlayerByIndex(i)->GetPreferredUniqueNetId();
		if (PlayerId.IsValid())
		{
			LocalPlayerIds.Add(PlayerId.ToSharedRef());
		}
	}

	if (!Sessions->StartMatchmaking(LocalPlayerIds, GameSessionName, SessionSettings, QuickMatchSearchSettingsRef))
	{
		OnMatchmakingComplete(GameSessionName, false);

	}
}

void UMainMenuUI::OnMatchmakingComplete(FName SessionName, bool bWasSuccessful)
{
	ULocalPlayer* const Player = GetOwningLocalPlayer();
	UFusionGameInstance* GameInstance = Cast<UFusionGameInstance>(Player->GetGameInstance());


	auto SessionInterface = Online::GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("OnMatchmakingComplete: Couldn't find session interface."));
		return;
	}

	SessionInterface->ClearOnMatchmakingCompleteDelegate_Handle(OnMatchmakingCompleteDelegateHandle);

	/*
	if (bQuickmatchSearchRequestCanceled && bUsedInputToCancelQuickmatchSearch)
	{
		bQuickmatchSearchRequestCanceled = false;
		// Clean up the session in case we get this event after canceling
		auto Sessions = Online::GetSessionInterface();
		if (bWasSuccessful && Sessions.IsValid())
		{
			if (PlayerOwner.IsValid() && PlayerOwner->GetPreferredUniqueNetId().IsValid())
			{
				Sessions->DestroySession(GameSessionName);
			}
		}
		return;
	}

	*/


	/*
	if (bAnimateQuickmatchSearchingUI)
	{
		bAnimateQuickmatchSearchingUI = false;
		HelperQuickMatchSearchingUICancel(false);
		bUsedInputToCancelQuickmatchSearch = false;
	}
	else
	{
		return;
	}
	*/

	if (!bWasSuccessful)
	{
		UE_LOG(LogOnline, Warning, TEXT("Matchmaking was unsuccessful."));
		//DisplayQuickmatchFailureUI();
		return;
	}

	UE_LOG(LogOnline, Log, TEXT("Matchmaking successful! Session name is %s."), *SessionName.ToString());

	if (GetOwningLocalPlayer() == NULL)
	{
		UE_LOG(LogOnline, Warning, TEXT("OnMatchmakingComplete: No owner."));
		return;
	}

	auto MatchmadeSession = SessionInterface->GetNamedSession(SessionName);

	if (!MatchmadeSession)
	{
		UE_LOG(LogOnline, Warning, TEXT("OnMatchmakingComplete: No session."));
		return;
	}

	if (!MatchmadeSession->OwningUserId.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("OnMatchmakingComplete: No session owner/host."));
		return;
	}

	if (GEngine && GEngine->GameViewport)
	{
		//GEngine->GameViewport->RemoveViewportWidgetContent(QuickMatchSearchingWidgetContainer.ToSharedRef());

	}
	//bAnimateQuickmatchSearchingUI = false;

	UE_LOG(LogOnline, Log, TEXT("OnMatchmakingComplete: Session host is %d."), *MatchmadeSession->OwningUserId->ToString());

	if (GameInstance)
	{
		//MenuWidget->LockControls(true);

		auto Subsystem = IOnlineSubsystem::Get();
		if (Subsystem != nullptr && Subsystem->IsLocalPlayer(*MatchmadeSession->OwningUserId))
		{
			// This console is the host, start the map.
			GameInstance->BeginHostingQuickMatch();
		}
		else
		{
			// We are the client, join the host.
			GameInstance->TravelToSession(SessionName);
		}
	}
}

void UMainMenuUI::Quit()
{
	ULocalPlayer* const Player = GetOwningLocalPlayer();
	UFusionGameInstance* GameInstance = Cast<UFusionGameInstance>(Player->GetGameInstance());

	if (GameInstance)
	{
		UGameViewportClient* const Viewport = GameInstance->GetGameViewportClient();
		if (ensure(Viewport))
		{
			Viewport->ConsoleCommand("quit");
		}
	}
}

// Server List stuff

/** Updates current search status */
void UMainMenuUI::UpdateSearchStatus()
{
	check(bSearchingForServers); // should not be called otherwise

	bool bFinishSearch = true;
	AFusionGameSession* FusionSession = GetGameSession();
	if (FusionSession)
	{
		int32 CurrentSearchIdx, NumSearchResults;
		EOnlineAsyncTaskState::Type SearchState = FusionSession->GetSearchResultStatus(CurrentSearchIdx, NumSearchResults);

		UE_LOG(LogOnlineGame, Log, TEXT("FusionSession->GetSearchResultStatus: %s"), EOnlineAsyncTaskState::ToString(SearchState));

		switch (SearchState)
		{
		case EOnlineAsyncTaskState::InProgress:
			//StatusText = LOCTEXT("Searching", "SEARCHING...");
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("InProgress")));
			bFinishSearch = false;
			break;

		case EOnlineAsyncTaskState::Done:
			// copy the results
		{
			ServerList.Empty();
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Done")));
			const TArray<FOnlineSessionSearchResult> & SearchResults = FusionSession->GetSearchResults();
			
			check(SearchResults.Num() == NumSearchResults);
			if (NumSearchResults == 0)
			{
#if PLATFORM_PS4
				//StatusText = LOCTEXT("NoServersFound", "NO SERVERS FOUND, PRESS SQUARE TO TRY AGAIN");
#elif PLATFORM_XBOXONE
				//StatusText = LOCTEXT("NoServersFound", "NO SERVERS FOUND, PRESS X TO TRY AGAIN");
#else
				//StatusText = LOCTEXT("NoServersFound", "NO SERVERS FOUND, PRESS SPACE TO TRY AGAIN");
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("NO SERVERS FOUND")));
#endif
			}
			else
			{
#if PLATFORM_PS4
				//StatusText = LOCTEXT("ServersRefresh", "PRESS SQUARE TO REFRESH SERVER LIST");
#elif PLATFORM_XBOXONE
				//StatusText = LOCTEXT("ServersRefresh", "PRESS X TO REFRESH SERVER LIST");
#else
				//StatusText = LOCTEXT("ServersRefresh", "PRESS SPACE TO REFRESH SERVER LIST");
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("found servers..")));
#endif
			}

			for (int32 IdxResult = 0; IdxResult < NumSearchResults; ++IdxResult)
			{
				TSharedPtr<FServerEntry> NewServerEntry = MakeShareable(new FServerEntry());

				const FOnlineSessionSearchResult& Result = SearchResults[IdxResult];

				NewServerEntry->ServerName = Result.Session.OwningUserName;
				NewServerEntry->Ping = FString::FromInt(Result.PingInMs);
				NewServerEntry->CurrentPlayers = FString::FromInt(Result.Session.SessionSettings.NumPublicConnections
					+ Result.Session.SessionSettings.NumPrivateConnections
					- Result.Session.NumOpenPublicConnections
					- Result.Session.NumOpenPrivateConnections);
				NewServerEntry->MaxPlayers = FString::FromInt(Result.Session.SessionSettings.NumPublicConnections
					+ Result.Session.SessionSettings.NumPrivateConnections);
				NewServerEntry->SearchResultsIndex = IdxResult;

				Result.Session.SessionSettings.Get(SETTING_GAMEMODE, NewServerEntry->GameType);
				Result.Session.SessionSettings.Get(SETTING_MAPNAME, NewServerEntry->MapName);

				ServerList.Add(NewServerEntry);
			}
		}
		break;

		case EOnlineAsyncTaskState::Failed:
			// intended fall-through
		case EOnlineAsyncTaskState::NotStarted:
			//StatusText = FText::GetEmpty();
			// intended fall-through
		default:
			break;
		}
	}

	if (bFinishSearch)
	{
		OnServerSearchFinished();
	}
}

/**
* Get the current game session
*/
AFusionGameSession* UMainMenuUI::GetGameSession() const
{
	ULocalPlayer* const Player = GetOwningLocalPlayer();

	UFusionGameInstance* const GI = Cast<UFusionGameInstance>(Player->GetGameInstance());
	return GI ? GI->GetGameSession() : nullptr;
}

/*
void UMainMenuUI::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (bSearchingForServers)
	{
		UpdateSearchStatus();
	}
}*/

void UMainMenuUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bSearchingForServers)
	{
		UpdateSearchStatus();
	}
}

/** Starts searching for servers */
void UMainMenuUI::BeginServerSearch(bool bLANMatch, const FString& InMapFilterName)
{
	bLANMatchSearch = bLANMatch;
	MapFilterName = InMapFilterName;
	bSearchingForServers = true;
	ServerList.Empty();

	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Begin Server Search")));

	UFusionGameInstance* const GI = Cast<UFusionGameInstance>(PlayerOwner->GetGameInstance());
	if (GI)
	{
		GI->FindSessions(PlayerOwner.Get(), bLANMatchSearch);
	}
}

/** Called when server search is finished */
void UMainMenuUI::OnServerSearchFinished()
{
	bSearchingForServers = false;
	
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Server Search Finished")));

	UpdateServerList();
}

void UMainMenuUI::UpdateServerList()
{
	/** Only filter maps if a specific map is specified */
	if (MapFilterName != "Any")
	{
		for (int32 i = 0; i < ServerList.Num(); ++i)
		{
			/** Only filter maps if a specific map is specified */
			if (ServerList[i]->MapName != MapFilterName)
			{
				ServerList.RemoveAt(i);
			}
		}
	}


	//SelectedItem = ServerList[0];
	int32 SelectedItemIndex = ServerList.IndexOfByKey(SelectedItem);

	//ServerListWidget->RequestListRefresh();


	if (ServerList.Num() > 0)
	{
		
		//ServerListWidget->UpdateSelectionSet();
		//ServerListWidget->SetSelection(ServerList[SelectedItemIndex > -1 ? SelectedItemIndex : 0], ESelectInfo::OnNavigation);
	}

}

void UMainMenuUI::ConnectToServer()
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Attempting Server Connect")));
	//SelectedItem = ServerList[0];


	if (bSearchingForServers)
	{
		// unsafe
		return;
	}
#if WITH_EDITOR
	if (GIsEditor == true)
	{
		return;
	}
#endif
	if (SelectedItem.IsValid())
	{
		//int ServerToJoin = SelectedItem->SearchResultsIndex;
		
		int ServerToJoin = 0;

		
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveAllViewportWidgets();
		}

		UFusionGameInstance* const GI = Cast<UFusionGameInstance>(PlayerOwner->GetGameInstance());
		if (GI)
		{
			GI->JoinSession(PlayerOwner.Get(), ServerToJoin);
		}
	}
}

#undef LOCTEXT_NAMESPACE



