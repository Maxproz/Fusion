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
	//HostFreeForAll();
	ULocalPlayer* const Player = GetOwningLocalPlayer();
	UFusionGameInstance* GameInstance = Cast<UFusionGameInstance>(Player->GetGameInstance());
	
	//GameInstance->StartOnlineGame();
	
	//GameInstance->BeginHostingQuickMatch();

	//GameInstance->HostGame(Player, "TDM", "/Game/Maps/Downfall?game=TDM?listen");

	HostTeamDeathMatch();
	
	//FString const StartURL = TEXT("/Game/Maps/Downfall?game=TDM?listen");
	//GetOwningPlayer()->ClientTravel(StartURL, ETravelType::TRAVEL_Relative);

	//MenuWidget->LockControls(true);
	
	//GameInstance->GetGameSession()->TravelToSession(Player->GetControllerId(), TEXT("/Game/Maps/Downfall?game=TDM?listen"));

}



void UMainMenuUI::OnClickedJoinButton()
{
	ULocalPlayer* const Player = GetOwningLocalPlayer();
	UFusionGameInstance* GameInstance = Cast<UFusionGameInstance>(Player->GetGameInstance());

	//GameInstance->JoinOnlineGame();
	BeginQuickMatchSearch();
}

void UMainMenuUI::OnClickedQuitButton()
{
	Quit();
	ULocalPlayer* const Player = GetOwningLocalPlayer();
	UFusionGameInstance* GameInstance = Cast<UFusionGameInstance>(Player->GetGameInstance());

	//GameInstance->FindOnlineGames();

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

#undef LOCTEXT_NAMESPACE

