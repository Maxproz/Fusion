// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameSession.h"
#include "FusionCharacter.h"
#include "FusionPlayerController.h"
#include "FusionPlayerController_Menu.h"
#include "FusionPlayerController_Lobby.h"
#include "FusionGameViewportClient.h"
#include "FusionGameState.h"
#include "FusionPlayerState.h"

#include "FusionHUD.h"

#include "Widgets/Menus/MainMenuUI.h"
#include "Widgets/Menus/ServerMenu_Widget.h"
#include "Widgets/Menus/Lobby/LobbyMenu_Widget.h"
#include "Widgets/Menus/OkErrorMessage_Widget.h"


#include "Widgets/Menus/FusionMessageMenu_Widget.h"

#include "Online/FusionOnlineSessionClient.h"

#include "FusionGameLoadingScreen.h"

#include "OnlineKeyValuePair.h"

#include "FusionGameInstance.h"



//we include the steam api here to be able to get the steam avatar
//refresh your visual studio files from editor after adding this to avoid weird redline errors
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX

#pragma push_macro("ARRAY_COUNT")
#undef ARRAY_COUNT

#include <steam/steam_api.h>

#pragma pop_macro("ARRAY_COUNT")

#endif


namespace FusionGameInstanceState
{
	const FName None = FName(TEXT("None"));
	const FName MainMenu = FName(TEXT("MainMenu"));
	const FName MessageMenu = FName(TEXT("MessageMenu"));
	const FName Lobby = FName(TEXT("Lobby"));
	const FName Playing = FName(TEXT("Playing"));
}


UFusionGameInstance::UFusionGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsOnline(true) // Default to online
	, bIsLicensed(true) // Default to licensed (should have been checked by OS on boot)
{
	CurrentState = FusionGameInstanceState::None;
	LanPlayerName = "Player";


	/**Bind Function for aceppting an invite*/
	OnSessionUserInviteAcceptedDelegate = FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UFusionGameInstance::OnSessionUserInviteAccepted);
}

void UFusionGameInstance::Init()
{
	Super::Init();

	IgnorePairingChangeForControllerId = -1;
	CurrentConnectionStatus = EOnlineServerConnectionStatus::Connected;


	// game requires the ability to ID users.
	const auto OnlineSub = IOnlineSubsystem::Get();
	check(OnlineSub);

	const auto IdentityInterface = OnlineSub->GetIdentityInterface();
	check(IdentityInterface.IsValid());

	const auto SessionInterface = OnlineSub->GetSessionInterface();
	check(SessionInterface.IsValid());


	if (OnlineSub)
	{
		// Get the Session Interface, so we can bind the accept delegate to it
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			//OnSessionInviteReceivedDelegateHandle = Sessions->AddOnSessionInviteReceivedDelegate_Handle(OnSessionInviteReceivedDelegate);
			
			//we bind the delagate for accepting an invite to the session interface so when you accept an invite, you can join the game.
			OnSessionUserInviteAcceptedDelegateHandle = Sessions->AddOnSessionUserInviteAcceptedDelegate_Handle(OnSessionUserInviteAcceptedDelegate);
		}
	}


	FCoreDelegates::OnSafeFrameChangedEvent.AddUObject(this, &UFusionGameInstance::HandleSafeFrameChanged);
	FCoreDelegates::ApplicationLicenseChange.AddUObject(this, &UFusionGameInstance::HandleAppLicenseUpdate);

	FCoreUObjectDelegates::PostLoadMap.AddUObject(this, &UFusionGameInstance::OnPostLoadMap);


	OnlineSub->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateUObject(this, &UFusionGameInstance::HandleNetworkConnectionStatusChanged));

	SessionInterface->AddOnSessionFailureDelegate_Handle(FOnSessionFailureDelegate::CreateUObject(this, &UFusionGameInstance::HandleSessionFailure));

	OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(this, &UFusionGameInstance::OnEndSessionComplete);
	
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UFusionGameInstance::OnDestroySessionComplete);


	OnFoundSessionsCompleteUMG().AddUObject(this, &UFusionGameInstance::OnFoundSessionsCompleteUMG);
	OnGetSteamFriendRequestCompleteUMG().AddUObject(this, &UFusionGameInstance::OnGetSteamFriendRequestCompleteUMG);
	OnShowErrorMessageUMG().AddUObject(this, &UFusionGameInstance::OnShowErrorMessageUMG);

	/** Bind the function for completing the friend list request*/
	FriendListReadCompleteDelegate = FOnReadFriendsListComplete::CreateUObject(this, &UFusionGameInstance::OnReadFriendsListCompleted);


	// Register delegate for ticker callback
	TickDelegate = FTickerDelegate::CreateUObject(this, &UFusionGameInstance::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);
}

void UFusionGameInstance::Shutdown()
{
	Super::Shutdown();

	// Unregister ticker delegate
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void UFusionGameInstance::HandleNetworkConnectionStatusChanged(EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus)
{
	CurrentConnectionStatus = ConnectionStatus;
}

void UFusionGameInstance::HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType)
{
	UE_LOG(LogOnlineGame, Warning, TEXT("UFusionGameInstance::HandleSessionFailure: %u"), (uint32)FailureType);

	const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection has been lost.");
	ShowMessageThenGotoState(ReturnReason,  FusionGameInstanceState::MainMenu);
}



void UFusionGameInstance::OnPostLoadMap()
{
	// Make sure we hide the loading screen when the level is done loading
	UFusionGameViewportClient* FusionViewport = Cast<UFusionGameViewportClient>(GetGameViewportClient());

	if (FusionViewport != NULL)
	{
		FusionViewport->HideLoadingScreen();
	}
}

void UFusionGameInstance::StartGameInstance()
{
	GotoInitialState();
}

FName UFusionGameInstance::GetInitialState()
{
	// On PC, go directly to the main menu
	return FusionGameInstanceState::MainMenu;
}

void UFusionGameInstance::GotoInitialState()
{
	GotoState(GetInitialState());
}

void UFusionGameInstance::ShowMessageThenGotoState(const FText& Message, const FName& NewState, const bool OverrideExisting, TWeakObjectPtr< ULocalPlayer > PlayerOwner)
{

	UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Message: %s, NewState: %s"), *Message.ToString(), *NewState.ToString());

	const bool bAlreadyAtMessageMenu = PendingState == FusionGameInstanceState::MessageMenu || CurrentState == FusionGameInstanceState::MessageMenu;
	const bool bAlreadyAtDestState = PendingState == NewState || CurrentState == NewState;

	// If we are already going to the message menu, don't override unless asked to
	if (bAlreadyAtMessageMenu && PendingMessage.NextState == NewState && !OverrideExisting)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (check 1)."));
		return;
	}

	// If we are already at the dest state, don't override unless asked
	if (bAlreadyAtDestState && !OverrideExisting)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (check 3)"));
		return;
	}

	PendingMessage.DisplayString = Message;
	PendingMessage.NextState = NewState;
	PendingMessage.PlayerOwner = PlayerOwner;

	if (CurrentState == FusionGameInstanceState::MessageMenu)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Forcing new message"));
		EndMessageMenuState();
		BeginMessageMenuState();
	}
	else
	{
		GotoState(FusionGameInstanceState::MessageMenu);
	}
}

void UFusionGameInstance::ShowLoadingScreen()
{
	// This can be confusing, so here is what is happening:
	//	For LoadMap, we use the IShooterGameLoadingScreenModule interface to show the load screen
	//  This is necessary since this is a blocking call, and our viewport loading screen won't get updated.
	//  We can't use IShooterGameLoadingScreenModule for seamless travel though
	//  In this case, we just add a widget to the viewport, and have it update on the main thread
	//  To simplify things, we just do both, and you can't tell, one will cover the other if they both show at the same time
	
	/* TODO: Need to figure out how this is implemented*/
	IFusionGameLoadingScreenModule* const LoadingScreenModule = FModuleManager::LoadModulePtr<IFusionGameLoadingScreenModule>("FusionGameLoadingScreen");
	if (LoadingScreenModule != nullptr)
	{
		LoadingScreenModule->StartInGameLoadingScreen();
	}

	UFusionGameViewportClient* FusionViewport = Cast<UFusionGameViewportClient>(GetGameViewportClient());

	if (FusionViewport != NULL)
	{
		FusionViewport->ShowLoadingScreen();
	}
}

bool UFusionGameInstance::LoadFrontEndMap(const FString& MapName)
{
	bool bSuccess = true;

	// if already loaded, do nothing
	UWorld* const World = GetWorld();
	if (World)
	{
		FString const CurrentMapName = *World->PersistentLevel->GetOutermost()->GetName();
		//if (MapName.Find(TEXT("Highrise")) != -1)
		if (CurrentMapName == MapName)
		{
			return bSuccess;
		}
	}

	FString Error;
	EBrowseReturnVal::Type BrowseRet = EBrowseReturnVal::Failure;
	FURL URL(
		*FString::Printf(TEXT("%s"), *MapName)
	);

	if (URL.Valid && !HasAnyFlags(RF_ClassDefaultObject)) //CastChecked<UEngine>() will fail if using Default__ShooterGameInstance, so make sure that we're not default
	{
		BrowseRet = GetEngine()->Browse(*WorldContext, URL, Error);

		// Handle failure.
		if (BrowseRet != EBrowseReturnVal::Success)
		{
			UE_LOG(LogLoad, Fatal, TEXT("%s"), *FString::Printf(TEXT("Failed to enter %s: %s. Please check the log for errors."), *MapName, *Error));
			bSuccess = false;
		}
	}
	return bSuccess;
}

AFusionGameSession* UFusionGameInstance::GetGameSession() const
{
	UWorld* const World = GetWorld();
	if (World)
	{
		AGameModeBase* const Game = World->GetAuthGameMode();
		if (Game)
		{
			return Cast<AFusionGameSession>(Game->GameSession);
		}
	}

	return nullptr;
}

void UFusionGameInstance::TravelLocalSessionFailure(UWorld *World, ETravelFailure::Type FailureType, const FString& ReasonString)
{
	AFusionPlayerController_Menu* const FirstPC = Cast<AFusionPlayerController_Menu>(UGameplayStatics::GetPlayerController(GetWorld(), 0));	
	// TODO: add lobby test here also
	if (FirstPC != nullptr)
	{
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join Session failed.");
		if (ReasonString.IsEmpty() == false)
		{
			ReturnReason = FText::Format(NSLOCTEXT("NetworkErrors", "JoinSessionFailedReasonFmt", "Join Session failed. {0}"), FText::FromString(ReasonString));
		}

		ShowMessageThenGoMain(ReturnReason);
	}
}

void UFusionGameInstance::ShowMessageThenGoMain(const FText& Message)
{
	ShowMessageThenGotoState(Message, FusionGameInstanceState::MainMenu);
}

void UFusionGameInstance::SetPendingInvite(const FFusionPendingInvite& InPendingInvite)
{
	PendingInvite = InPendingInvite;
}

void UFusionGameInstance::GotoState(FName NewState)
{
	UE_LOG(LogOnline, Log, TEXT("GotoState: NewState: %s"), *NewState.ToString());

	PendingState = NewState;
}

void UFusionGameInstance::MaybeChangeState()
{
	if ((PendingState != CurrentState) && (PendingState != FusionGameInstanceState::None))
	{
		FName const OldState = CurrentState;

		// end current state
		EndCurrentState(PendingState);

		// begin new state
		BeginNewState(PendingState, OldState);

		// clear pending change
		PendingState = FusionGameInstanceState::None;
	}
}

void UFusionGameInstance::EndCurrentState(FName NextState)
{
	// per-state custom ending code here
	if (CurrentState == FusionGameInstanceState::MainMenu)
	{
		EndMainMenuState();
	}
	else if (CurrentState == FusionGameInstanceState::Lobby)
	{
		EndLobbyState();
	}
	else if (CurrentState == FusionGameInstanceState::MessageMenu)
	{
		EndMessageMenuState();
	}
	else if (CurrentState == FusionGameInstanceState::Playing)
	{
		EndPlayingState();
	}

	CurrentState = FusionGameInstanceState::None;
}

void UFusionGameInstance::BeginNewState(FName NewState, FName PrevState)
{
	// per-state custom starting code here
	if (NewState == FusionGameInstanceState::MainMenu)
	{
		BeginMainMenuState();
	}
	else if (NewState == FusionGameInstanceState::Lobby)
	{
		BeginLobbyState();
	}
	else if (NewState == FusionGameInstanceState::MessageMenu)
	{
		BeginMessageMenuState();
	}
	else if (NewState == FusionGameInstanceState::Playing)
	{
		BeginPlayingState();
	}

	CurrentState = NewState;
}

void UFusionGameInstance::SetPresenceForLocalPlayers(const FVariantData& PresenceData)
{
	const auto Presence = Online::GetPresenceInterface();
	if (Presence.IsValid())
	{
		for (int i = 0; i < LocalPlayers.Num(); ++i)
		{
			const TSharedPtr<const FUniqueNetId> UserId = LocalPlayers[i]->GetPreferredUniqueNetId();

			if (UserId.IsValid())
			{
				FOnlineUserPresenceStatus PresenceStatus;
				PresenceStatus.Properties.Add(DefaultPresenceKey, PresenceData);

				Presence->SetPresence(*UserId, PresenceStatus);
			}
		}
	}
}

void UFusionGameInstance::BeginMainMenuState()
{
	// Make sure we're not showing the loadscreen
	UFusionGameViewportClient* FusionViewport = Cast<UFusionGameViewportClient>(GetGameViewportClient());

	if (FusionViewport != NULL)
	{
		FusionViewport->HideLoadingScreen();
	}

	SetIsOnline(false);


	// Set presence to menu state for the owning player
	SetPresenceForLocalPlayers(FVariantData(FString(TEXT("OnMenu"))));

	// load startup map
	LoadFrontEndMap(MainMenuMap);

	// player 0 gets to own the UI
	ULocalPlayer* const Player = GetFirstGamePlayer();


	MainMenuUI = Cast<AFusionPlayerController_Menu>(Player->GetPlayerController(GetWorld()))->GetFusionHUD()->GetMainMenuUIWidget();

	if (MainMenuUI.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Emerald, FString::Printf(TEXT("Showing The Valid Main Menu Widget")));
		MainMenuUI.Get()->ShowWidget();
	}


	#if !FUSION_CONSOLE_UI
	// The cached unique net ID is usually set on the welcome screen, but there isn't
	// one on PC/Mac, so do it here.
	if (Player != nullptr)
	{
		Player->SetControllerId(0);
		Player->SetCachedUniqueNetId(Player->GetUniqueNetIdFromCachedControllerId());
	}
	#endif

	RemoveNetworkFailureHandlers();
}

void UFusionGameInstance::EndMainMenuState()
{
	if (MainMenuUI.IsValid())
	{
		MainMenuUI.Get()->HideWidget();
		MainMenuUI = nullptr;
	}
}

void UFusionGameInstance::BeginMessageMenuState()
{
	if (PendingMessage.DisplayString.IsEmpty())
	{
		UE_LOG(LogOnlineGame, Warning, TEXT("UFusionGameInstance::BeginMessageMenuState: Display string is empty"));
		GotoInitialState();
		return;
	}

	// Make sure we're not showing the loadscreen
	UFusionGameViewportClient* FusionViewport = Cast<UFusionGameViewportClient>(GetGameViewportClient());

	if (FusionViewport != NULL)
	{
		FusionViewport->HideLoadingScreen();
	}

	// player 0 gets to own the UI
	ULocalPlayer* const Player = GetFirstGamePlayer();
	MessageMenuUI = Cast<AFusionPlayerController_Menu>(Player->GetPlayerController(GetWorld()))->GetFusionHUD()->GetMessageMenuWidget();

	if (MessageMenuUI.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Emerald, FString::Printf(TEXT("Showing The Message Menu Widget")));

		MessageMenuUI.Get()->DisplayMessage = PendingMessage.DisplayString;
		MessageMenuUI.Get()->OnRep_DisplayMessage();
		MessageMenuUI.Get()->ShowWidget();
	}

	PendingMessage.DisplayString = FText::GetEmpty();
}

void UFusionGameInstance::EndMessageMenuState()
{
	if (MessageMenuUI.IsValid())
	{
		MessageMenuUI.Get()->HideWidget();
		MessageMenuUI = nullptr;
	}

	GotoState(FusionGameInstanceState::MainMenu);
}

void UFusionGameInstance::BeginLobbyState()
{
	// Set presence for being in the lobby
	SetPresenceForLocalPlayers(FVariantData(FString(TEXT("Lobby"))));


	ULocalPlayer* const Player = GetFirstGamePlayer();
	AFusionPlayerController_Lobby* LPC = Cast<AFusionPlayerController_Lobby>(Player->GetPlayerController(GetWorld()));

	if (LPC)
	{
		AFusionHUD* FHUD = Cast<AFusionHUD>(LPC->GetFusionHUD());
		if (FHUD)
		{
			
			LobbyWidget = FHUD->GetLobbyMenuWidget();
			{
				if (LobbyWidget.IsValid())
				{
					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Emerald, FString::Printf(TEXT("Showing The Valid Lobby Widget")));
					LobbyWidget.Get()->ShowWidget();
				}
			}
			

			/*
			LobbyWidget = FHUD->GetLobbyMenuWidget()->TakeWidget();
			{
				if (LobbyWidget.IsValid())
				{
					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Emerald, FString::Printf(TEXT("Showing The Valid Lobby Widget")));
					LobbyWidget.Get()->SetVisibility(EVisibility::Visible);
				}
			}
			*/

		}
	}
}

void UFusionGameInstance::EndLobbyState()
{
	if (LobbyWidget.IsValid())
	{
		LobbyWidget.Get()->HideWidget();
		//LobbyWidget.Get()->SetVisibility(EVisibility::Hidden);
		LobbyWidget = nullptr;
	}

	
}

void UFusionGameInstance::BeginPlayingState()
{
	// Set presence for playing in a map
	SetPresenceForLocalPlayers(FVariantData(FString(TEXT("InGame"))));

	// Make sure viewport has focus
	FSlateApplication::Get().SetAllUserFocusToGameViewport(); // TODO: Figure out if I need this later on.
}

void UFusionGameInstance::EndPlayingState()
{
	// Clear the players' presence information
	SetPresenceForLocalPlayers(FVariantData(FString(TEXT("OnMenu"))));

	UWorld* const World = GetWorld();
	AFusionGameState* const GameState = World != NULL ? World->GetGameState<AFusionGameState>() : NULL;

	if (GameState)
	{
		// Send round end events for local players
		for (int i = 0; i < LocalPlayers.Num(); ++i)
		{
			auto FusionPC = Cast<AFusionPlayerController>(LocalPlayers[i]->PlayerController);
			if (FusionPC)
			{
				// Assuming you can't win if you quit early
				FusionPC->ClientSendRoundEndEvent(false, GameState->ElapsedTime);
			}
		}

		// Give the game state a chance to cleanup first
		GameState->RequestFinishAndExitToMainMenu();
	}
	else
	{
		// If there is no game state, make sure the session is in a good state
		CleanupSessionOnReturnToMenu();
	}
}


void UFusionGameInstance::LabelPlayerAsQuitter(ULocalPlayer* LocalPlayer) const
{
	AFusionPlayerState* const PlayerState = LocalPlayer && LocalPlayer->PlayerController ? Cast<AFusionPlayerState>(LocalPlayer->PlayerController->PlayerState) : nullptr;
	if (PlayerState)
	{
		PlayerState->SetQuitter(true);
	}
}

void UFusionGameInstance::RemoveNetworkFailureHandlers()
{
	// Remove the local session/travel failure bindings if they exist
	if (GEngine->OnTravelFailure().IsBoundToObject(this) == true)
	{
		GEngine->OnTravelFailure().Remove(TravelLocalSessionFailureDelegateHandle);
	}
}

void UFusionGameInstance::AddNetworkFailureHandlers()
{
	// Add network/travel error handlers (if they are not already there)
	if (GEngine->OnTravelFailure().IsBoundToObject(this) == false)
	{
		TravelLocalSessionFailureDelegateHandle = GEngine->OnTravelFailure().AddUObject(this, &UFusionGameInstance::TravelLocalSessionFailure);
	}
}

TSubclassOf<UOnlineSession> UFusionGameInstance::GetOnlineSessionClass()
{
	return UOnlineSessionClient::StaticClass();
}


void UFusionGameInstance::TestMessageMenu()
{
	const FText TestMessage = NSLOCTEXT("ProfileMessages", "TestMsg", "This is a test message");

	ULocalPlayer* const Player = GetFirstGamePlayer();
	TWeakObjectPtr<ULocalPlayer> CurrentPlayer = Player;

	ShowMessageThenGotoState(FText::FromString(TEXT("Hey Buddy")), FusionGameInstanceState::Playing, true, CurrentPlayer);
}

void UFusionGameInstance::TestConfirmDialog()
{

	UFusionGameViewportClient* FusionViewport = Cast<UFusionGameViewportClient>(GetGameViewportClient());

	ULocalPlayer* const Player = GetFirstGamePlayer();
	TWeakObjectPtr<ULocalPlayer> CurrentPlayer = Player;

	TScriptDelegate<FWeakObjectPtr> OnControllerReconnectDelegate;
	OnControllerReconnectDelegate.BindUFunction(this, FName("HideDialogMenuTestFunc")); // OnControllerReconnectConfirm

	TScriptDelegate<FWeakObjectPtr> EmptyFunctionDelegate;
	EmptyFunctionDelegate.BindUFunction(this, FName("EmptyFunction"));

	FusionViewport->ShowDialog(
		CurrentPlayer,
		//FText::Format(NSLOCTEXT("ProfileMessages", "PlayerReconnectControllerFmt", "Player {0}, please reconnect your controller."), FText::AsNumber(i + 1)),
		FText::FromString(TEXT("please reconnect your controller.")),
		EFusionDialogType::Generic,
		OnControllerReconnectDelegate,
		EmptyFunctionDelegate
	);
}

void UFusionGameInstance::HideDialogMenuTestFunc()
{
	UFusionGameViewportClient* FusionViewport = Cast<UFusionGameViewportClient>(GetGameViewportClient());
	if (FusionViewport)
	{
		FusionViewport->HideDialog();
	}
}


bool UFusionGameInstance::Tick(float DeltaSeconds)
{
	// Dedicated server doesn't need to worry about game state
	if (IsRunningDedicatedServer() == true)
	{
		return true;
	}


	MaybeChangeState();

	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("CurrentState: %s"), *CurrentState.ToString()));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("PendingState: %s"), *PendingState.ToString()));


	UFusionGameViewportClient* FusionViewport = Cast<UFusionGameViewportClient>(GetGameViewportClient());

	if (CurrentState != FusionGameInstanceState::MainMenu)
	{
		// If at any point we aren't licensed (but we are after MainMenu) bounce them back to the welcome screen
		if (!bIsLicensed && CurrentState != FusionGameInstanceState::None && !FusionViewport->IsShowingDialog())
		{
			const FText ReturnReason = NSLOCTEXT("ProfileMessages", "NeedLicense", "The signed in users do not have a license for this game. Please purchase Fusion from the Xbox Marketplace or sign in a user with a valid license.");
			const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

			ShowMessageThenGotoState(ReturnReason, FusionGameInstanceState::MainMenu);
		}

		
		TScriptDelegate<FWeakObjectPtr> OnControllerReconnectDelegate;
		OnControllerReconnectDelegate.BindUFunction(this, FName("HideDialogMenuTestFunc")); // OnControllerReconnectConfirm

		TScriptDelegate<FWeakObjectPtr> EmptyFunctionDelegate;
		EmptyFunctionDelegate.BindUFunction(this, FName("EmptyFunction"));

		// Show controller disconnected dialog if any local players have an invalid controller
		if (FusionViewport != NULL &&
			!FusionViewport->IsShowingDialog())
		{
			for (int i = 0; i < LocalPlayers.Num(); ++i)
			{
				if (LocalPlayers[i] && LocalPlayers[i]->GetControllerId() == -1)
				{
					FusionViewport->ShowDialog(
						LocalPlayers[i],
						//FText::Format(NSLOCTEXT("ProfileMessages", "PlayerReconnectControllerFmt", "Player {0}, please reconnect your controller."), FText::AsNumber(i + 1)),
						FText::FromString(TEXT("please reconnect your controller.")),
						EFusionDialogType::ControllerDisconnected,
						OnControllerReconnectDelegate,
						EmptyFunctionDelegate
					);
				}
			}
		}
	}

	return true;
}

void UFusionGameInstance::EmptyFunction()
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("There is no function bound to this button")));
}

bool UFusionGameInstance::HandleOpenCommand(const TCHAR* Cmd, FOutputDevice& Ar, UWorld* InWorld)
{
	bool const bOpenSuccessful = Super::HandleOpenCommand(Cmd, Ar, InWorld);
	if (bOpenSuccessful)
	{
		GotoState(FusionGameInstanceState::Playing);
	}

	return bOpenSuccessful;
}



void UFusionGameInstance::HandleAppLicenseUpdate()
{
	TSharedPtr<GenericApplication> GenericApplication = FSlateApplication::Get().GetPlatformApplication();
	bIsLicensed = GenericApplication->ApplicationLicenseValid();
}

void UFusionGameInstance::HandleSafeFrameChanged()
{
	UCanvas::UpdateAllCanvasSafeZoneData();
}


void UFusionGameInstance::SetIsOnline(bool bInIsOnline)
{
	bIsOnline = bInIsOnline;
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		for (int32 i = 0; i < LocalPlayers.Num(); ++i)
		{
			ULocalPlayer* LocalPlayer = LocalPlayers[i];

			TSharedPtr<const FUniqueNetId> PlayerId = LocalPlayer->GetPreferredUniqueNetId();
			if (PlayerId.IsValid())
			{
				OnlineSub->SetUsingMultiplayerFeatures(*PlayerId, bIsOnline);
			}
		}
	}
}


bool UFusionGameInstance::IsLocalPlayerOnline(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer == NULL)
	{
		return false;
	}
	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		const auto IdentityInterface = OnlineSub->GetIdentityInterface();
		if (IdentityInterface.IsValid())
		{
			auto UniqueId = LocalPlayer->GetCachedUniqueNetId();
			if (UniqueId.IsValid())
			{
				const auto LoginStatus = IdentityInterface->GetLoginStatus(*UniqueId);
				if (LoginStatus == ELoginStatus::LoggedIn)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool UFusionGameInstance::ValidatePlayerForOnlinePlay(ULocalPlayer* LocalPlayer)
{
	// Get the viewport
	UFusionGameViewportClient* FusionViewport = Cast<UFusionGameViewportClient>(GetGameViewportClient());

	if (!IsLocalPlayerOnline(LocalPlayer))
	{
		// Don't let them play online if they aren't online
		if (FusionViewport != NULL)
		{
			const FText Msg = NSLOCTEXT("NetworkFailures", "MustBeSignedIn", "You must be signed in to play online");

			TScriptDelegate<FWeakObjectPtr> OnConfirmGenericDelegate;
			OnConfirmGenericDelegate.BindUFunction(this, FName("OnConfirmGeneric"));

			TScriptDelegate<FWeakObjectPtr> EmptyFunctionDelegate;

			FusionViewport->ShowDialog(
				NULL,
				Msg,
				EFusionDialogType::Generic,
				OnConfirmGenericDelegate,
				OnConfirmGenericDelegate
			);
		}

		return false;
	}

	return true;
}

FReply UFusionGameInstance::OnConfirmGeneric()
{
	UFusionGameViewportClient* FusionViewport = Cast<UFusionGameViewportClient>(GetGameViewportClient());
	if (FusionViewport)
	{
		FusionViewport->HideDialog();
	}

	return FReply::Handled();
}

void UFusionGameInstance::StartOnlinePrivilegeTask(const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate& Delegate, EUserPrivileges::Type Privilege, TSharedPtr< const FUniqueNetId > UserId)
{
	/*
	WaitMessageWidget = SNew(SShooterWaitDialog)
		.MessageText(NSLOCTEXT("NetworkStatus", "CheckingPrivilegesWithServer", "Checking privileges with server.  Please wait..."));

		*/
	if (GEngine && GEngine->GameViewport)
	{
		UGameViewportClient* const GVC = GEngine->GameViewport;
		//GVC->AddViewportWidgetContent(WaitMessageWidget.ToSharedRef());
	}

	auto Identity = Online::GetIdentityInterface();
	if (Identity.IsValid() && UserId.IsValid())
	{
		Identity->GetUserPrivilege(*UserId, Privilege, Delegate);
	}
	else
	{
		// Can only get away with faking the UniqueNetId here because the delegates don't use it
		Delegate.ExecuteIfBound(FUniqueNetIdString(), Privilege, (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures);
	}
}

void UFusionGameInstance::CleanupOnlinePrivilegeTask()
{
	/* TODO: Need to implement this widget and functionality
	if (GEngine && GEngine->GameViewport && WaitMessageWidget.IsValid())
	{
		UGameViewportClient* const GVC = GEngine->GameViewport;
		GVC->RemoveViewportWidgetContent(WaitMessageWidget.ToSharedRef());
	}
	*/
}

void UFusionGameInstance::DisplayOnlinePrivilegeFailureDialogs(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
	// Show warning that the user cannot play due to age restrictions
	UFusionGameViewportClient* FusionViewport = Cast<UFusionGameViewportClient>(GetGameViewportClient());
	TWeakObjectPtr<ULocalPlayer> OwningPlayer;
	if (GEngine)
	{
		for (auto It = GEngine->GetLocalPlayerIterator(GetWorld()); It; ++It)
		{
			TSharedPtr<const FUniqueNetId> OtherId = (*It)->GetPreferredUniqueNetId();
			if (OtherId.IsValid())
			{
				if (UserId == (*OtherId))
				{
					OwningPlayer = *It;
				}
			}
		}
	}


	TScriptDelegate<FWeakObjectPtr> OnConfirmGenericDelegate;
	OnConfirmGenericDelegate.BindUFunction(this, FName("OnConfirmGeneric"));

	if (FusionViewport != NULL && OwningPlayer.IsValid())
	{
		if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::AccountTypeFailure) != 0)
		{
			IOnlineExternalUIPtr ExternalUI = Online::GetExternalUIInterface();
			if (ExternalUI.IsValid())
			{
				ExternalUI->ShowAccountUpgradeUI(UserId);
			}
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredSystemUpdate) != 0)
		{
			FusionViewport->ShowDialog(
				OwningPlayer.Get(),
				NSLOCTEXT("OnlinePrivilegeResult", "RequiredSystemUpdate", "A required system update is available.  Please upgrade to access online features."),
				EFusionDialogType::Generic,
				OnConfirmGenericDelegate,
				OnConfirmGenericDelegate

			);
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredPatchAvailable) != 0)
		{
			FusionViewport->ShowDialog(
				OwningPlayer.Get(),
				NSLOCTEXT("OnlinePrivilegeResult", "RequiredPatchAvailable", "A required game patch is available.  Please upgrade to access online features."),
				EFusionDialogType::Generic,
				OnConfirmGenericDelegate,
				OnConfirmGenericDelegate
			);
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::AgeRestrictionFailure) != 0)
		{
			FusionViewport->ShowDialog(
				OwningPlayer.Get(),
				NSLOCTEXT("OnlinePrivilegeResult", "AgeRestrictionFailure", "Cannot play due to age restrictions!"),
				EFusionDialogType::Generic,
				OnConfirmGenericDelegate,
				OnConfirmGenericDelegate
				
			);
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::UserNotFound) != 0)
		{
			FusionViewport->ShowDialog(
				OwningPlayer.Get(),
				NSLOCTEXT("OnlinePrivilegeResult", "UserNotFound", "Cannot play due invalid user!"),
				EFusionDialogType::Generic,
				OnConfirmGenericDelegate,
				OnConfirmGenericDelegate
			);
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::GenericFailure) != 0)
		{
			FusionViewport->ShowDialog(
				OwningPlayer.Get(),
				NSLOCTEXT("OnlinePrivilegeResult", "GenericFailure", "Cannot play online.  Check your network connection."),
				EFusionDialogType::Generic,
				OnConfirmGenericDelegate,
				OnConfirmGenericDelegate
				
			);
		}
	}
}





#define LOCTEXT_NAMESPACE "Fusion.HUD.Menu"

void UFusionGameInstance::StartOnlineGame(FString ServerName, int32 MaxNumPlayers, bool bIsLAN, bool bIsPresence, bool bIsPasswordProtected, FString SessionPassword)
{
	// Creating a local player where we can get the UserID from
	ULocalPlayer* const Player = GetFirstGamePlayer();

	// Call our custom HostSession function. GameSessionName is a GameInstance variable
	GetGameSession()->HostSession(Player->GetPreferredUniqueNetId(), GameSessionName, ServerName, bIsLAN, bIsPresence, MaxNumPlayers, bIsPasswordProtected, SessionPassword);

}


void UFusionGameInstance::FindOnlineGames(bool bIsLAN, bool bIsPresence)
{
	ULocalPlayer* const Player = GetFirstGamePlayer();

	GetGameSession()->FindSessions(Player->GetPreferredUniqueNetId(), GameSessionName, bIsLAN, bIsPresence);
}

void UFusionGameInstance::JoinOnlineGame(int32 SessionIndex)
{
	ULocalPlayer* const Player = GetFirstGamePlayer();

	FOnlineSessionSearchResult SearchResult;
	SearchResult = GetGameSession()->SessionSearch->SearchResults[SessionIndex];

	//GetGameSession()->MaxPlayersinSession = SearchResult.Session.SessionSettings.NumPublicConnections;
	MaxPlayersinSession = SearchResult.Session.SessionSettings.NumPublicConnections;

	GetGameSession()->JoinASession(Player->GetPreferredUniqueNetId(), GameSessionName, SearchResult);
}

void UFusionGameInstance::DestroySessionAndLeaveGame()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
			Sessions->DestroySession(GameSessionName);
		}
	}
}

void UFusionGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));
	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// Clear the Delegate
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

			//HostSettings = NULL;

			// If it was successful, we just load another level (could be a MainMenu!)
			if (bWasSuccessful)
			{
				// Was doing this inside "Handle Match Has Ended I think 
				UGameplayStatics::OpenLevel(GetWorld(), MainMenuMapp, true);
			}
		}
	}
}



bool UFusionGameInstance::IsOnlineSubsystemSteam() const
{
	//get the steam online subsystem
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get(FName("Steam"));

	if (OnlineSub)
		return true;
	else
		return false;
}

FString UFusionGameInstance::GetPlayerName() const
{
	//if steam is running, return an empty string
	if (IsOnlineSubsystemSteam())
		return "";
	//else retrun the saved player lan name
	else
		return LanPlayerName;
}

#undef LOCTEXT_NAMESPACE

// call the ServerMenu UMG and pass the array of Session Results when yhe session finding is over
void UFusionGameInstance::OnFoundSessionsCompleteUMG(const TArray<FCustomFusionSessionResult>& CustomSessionResults)
{
	// player 0 gets to own the UI
	ULocalPlayer* const Player = GetFirstGamePlayer();
	AFusionPlayerController_Menu* MPC = Cast<AFusionPlayerController_Menu>(Player->GetPlayerController(GetWorld()));

	UServerMenu_Widget* ServerWidget = MPC->GetFusionHUD()->GetServerMenuWidget();

	if (ServerWidget)
	{
		ServerWidget->OnSessionSearchCompleated().Broadcast(CustomSessionResults);
	}
}

// called from code when it finishes getting the friendlist and passes it to blueprint
void UFusionGameInstance::OnGetSteamFriendRequestCompleteUMG(const TArray<FSteamFriendInfo>& BPFriendsLists)
{
	// player 0 gets to own the UI
	ULocalPlayer* const Player = GetFirstGamePlayer();

	AFusionPlayerController_Lobby* LPC = Cast<AFusionPlayerController_Lobby>(Player->GetPlayerController(GetWorld()));

	ULobbyMenu_Widget* LobbyWidget = LPC->GetFusionHUD()->GetLobbyMenuWidget();

	if (LobbyWidget)
	{
		LobbyWidget->GetSteamFriendRequestCompleted().Broadcast(BPFriendsLists);
	}
	
}

void UFusionGameInstance::TestErrorMsg(FText Msg)
{
	OnShowErrorMessageUMG().Broadcast(Msg);
}

void UFusionGameInstance::OnShowErrorMessageUMG(const FText & ErrorMessage)
{
	ULocalPlayer* const Player = GetFirstGamePlayer();

	AFusionPlayerController_Lobby* LPC = Cast<AFusionPlayerController_Lobby>(Player->GetPlayerController(GetWorld()));
	if (LPC)
	{
		UOkErrorMessage_Widget* OkErrorMessage_Widget = LPC->GetFusionHUD()->GetErrorMessageWidget();
		if (!OkErrorMessage_Widget)
		{
			UOkErrorMessage_Widget* OkErrorMessage_Widget = CreateWidget<UOkErrorMessage_Widget>(LPC, LPC->GetFusionHUD()->UOkErrorMessage_WidgetTemplate);
			OkErrorMessage_Widget->AddToViewport(2);
		}

		// if the widget was created already, change the error text and display it
		OkErrorMessage_Widget->ErrorText = ErrorMessage;
		OkErrorMessage_Widget->ShowWidget();
	}


	AFusionPlayerController_Menu* MPC = Cast<AFusionPlayerController_Menu>(Player->GetPlayerController(GetWorld()));
	if (MPC)
	{
		UOkErrorMessage_Widget* OkErrorMessage_Widget = MPC->GetFusionHUD()->GetErrorMessageWidget();
		if (!OkErrorMessage_Widget)
		{
			UOkErrorMessage_Widget* OkErrorMessage_Widget = CreateWidget<UOkErrorMessage_Widget>(MPC, MPC->GetFusionHUD()->UOkErrorMessage_WidgetTemplate);
			OkErrorMessage_Widget->AddToViewport(2);
		}
		// if the widget was created already, change the error text and display it
		OkErrorMessage_Widget->ErrorText = ErrorMessage;
		OkErrorMessage_Widget->ShowWidget();
	}

}


// when reading friend list from the online subsystem is finished, get it and store it then call blueprint to show it on UMG
void UFusionGameInstance::OnReadFriendsListCompleted(int32 LocalUserNum, bool bWasSuccessful, const FString & ListName, const FString & ErrorString)
{
	if (bWasSuccessful)
	{
		//get the steam online subsystem
		IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get(FName("Steam"));

		//check if the online subsystem is valid
		if (OnlineSub)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, TEXT("Found Steam Online"));
			//get Friends interface
			IOnlineFriendsPtr FriendsInterface = OnlineSub->GetFriendsInterface();

			//if the Friends Interface is valid
			if (FriendsInterface.IsValid())
			{

				TArray< TSharedRef<FOnlineFriend> > FriendList;
				//get a list on all online players and store them in the FriendList
				FriendsInterface->GetFriendsList(LocalUserNum, ListName/*EFriendsLists::ToString((EFriendsLists::Default)),*/, FriendList);

				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, *FString::Printf(TEXT("Number of friends found is: %d"), FriendList.Num()));

				TArray<FSteamFriendInfo> BPFriendsList;
				//for each loop to convert the FOnlineFriend array into the cuteom BP struct
				for (TSharedRef<FOnlineFriend> Friend : FriendList)
				{
					//temp FSteamFriendInfo variable to add to the array
					FSteamFriendInfo TempSteamFriendInfo;
					//get the friend's User ID
					TempSteamFriendInfo.PlayerUniqueNetID.SetUniqueNetId(Friend->GetUserId());
					//get the friend's avatar as texture 2D and store it
					TempSteamFriendInfo.PlayerAvatar = GetSteamAvatar(TempSteamFriendInfo.PlayerUniqueNetID);
					//get the friend's display name
					TempSteamFriendInfo.PlayerName = Friend->GetDisplayName();
					//add the temp variable to the 
					BPFriendsList.Add(TempSteamFriendInfo);
				}

				//call blueprint to show the info on UMG
				OnGetSteamFriendRequestCompleteUMG().Broadcast(BPFriendsList);
			}
		}
	}
	else
		OnShowErrorMessageUMG().Broadcast(FText::FromString(TEXT("ErrorString")));
}

void UFusionGameInstance::GetSteamFriendsList(APlayerController *PlayerController)
{
	//check if the player controller is valid
	if (PlayerController)
	{

		//get the steam online subsystem
		IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get(FName("Steam"));

		//check if the online subsystem is valid
		if (OnlineSub)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, TEXT("Found Steam Online"));
			//get Friends interface
			IOnlineFriendsPtr FriendsInterface = OnlineSub->GetFriendsInterface();

			//if the Friends Interface is valid
			if (FriendsInterface.IsValid())
			{

				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Found friend interface"));
				// get the local player from the player controller
				ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->Player);

				//chaeck if the local player exists
				if (LocalPlayer)
				{
					//read the friend list from the online subsystem then call the delagate when completed
					FriendsInterface->ReadFriendsList(LocalPlayer->GetControllerId(), EFriendsLists::ToString((EFriendsLists::InGamePlayers)), FriendListReadCompleteDelegate);
				}
			}
		}
	}
}


UTexture2D* UFusionGameInstance::GetSteamAvatar(const FBPUniqueNetId UniqueNetId)
{
	
	if (UniqueNetId.IsValid())
	{
		uint32 Width = 0;
		uint32 Height = 0;

		//get the player iID
		uint64 id = *((uint64*)UniqueNetId.UniqueNetId->GetBytes());

		int Picture = 0;

		// get the Avatar ID using the player ID
		Picture = SteamFriends()->GetMediumFriendAvatar(id);
	
		//if the Avatar ID is not valid retrun null
		if (Picture == -1)
			return nullptr;

		//get the image size from steam
		SteamUtils()->GetImageSize(Picture, &Width, &Height);

		// if the image size is valid (most of this is from the Advanced Seesion Plugin as well, mordentral, THANK YOU!
		if (Width > 0 && Height > 0)
		{
			//Creating the buffer "oAvatarRGBA" and then filling it with the RGBA Stream from the Steam Avatar
			uint8 *oAvatarRGBA = new uint8[Width * Height * 4];


			//Filling the buffer with the RGBA Stream from the Steam Avatar and creating a UTextur2D to parse the RGBA Steam in
			SteamUtils()->GetImageRGBA(Picture, (uint8*)oAvatarRGBA, 4 * Height * Width * sizeof(char));

			UTexture2D* Avatar = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);

			// Switched to a Memcpy instead of byte by byte transer
			uint8* MipData = (uint8*)Avatar->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(MipData, (void*)oAvatarRGBA, Height * Width * 4);
			Avatar->PlatformData->Mips[0].BulkData.Unlock();

			// Original implementation was missing this!!
			// the hell man......
			// deallocating the memory used to get the avatar data
			delete[] oAvatarRGBA;

			//Setting some Parameters for the Texture and finally returning it
			Avatar->PlatformData->NumSlices = 1;
			Avatar->NeverStream = true;
			//Avatar->CompressionSettings = TC_EditorIcon;

			Avatar->UpdateResource();

			return Avatar;
		}

	}
	
	return nullptr;
}

void UFusionGameInstance::ShowErrorMessage(const FText & ErrorMessage)
{
	//Show the message on UMG 
	OnShowErrorMessageUMG().Broadcast(ErrorMessage);
}


void UFusionGameInstance::SendSessionInviteToFriend(APlayerController* InvitingPlayer, const FBPUniqueNetId & Friend)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(InvitingPlayer->GetLocalPlayer());
		if (LocalPlayer)
		{
			// Get SessionInterface from the OnlineSubsystem
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid() && LocalPlayer->GetPreferredUniqueNetId().IsValid() && Friend.IsValid())
			{
				Sessions->SendSessionInviteToFriend(*LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, *Friend.GetUniqueNetId());
			}
		}
	}
}

void UFusionGameInstance::OnSessionUserInviteAccepted(bool bWasSuccessful, int32 LocalUserNum, TSharedPtr<const FUniqueNetId> InvitingPlayer, const FOnlineSessionSearchResult & TheSessionInvitedTo)
{
	if (bWasSuccessful)
	{
		if (TheSessionInvitedTo.IsValid())
		{
			GetGameSession()->JoinASession(LocalUserNum, GameSessionName, TheSessionInvitedTo);
		}
	}
}

void UFusionGameInstance::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	
	UE_LOG(LogOnline, Log, TEXT("UFusionGameInstance::OnEndSessionComplete: Session=%s bWasSuccessful=%s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
			Sessions->ClearOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegateHandle);
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

		}
	}
	// continue
	CleanupSessionOnReturnToMenu();
}

// This cleans up my Online Session State, different from my Game instance state
void UFusionGameInstance::CleanupSessionOnReturnToMenu()
{
	bool bPendingOnlineOp = false;

	// end online game and then destroy it
	IOnlineSubsystem * OnlineSub = IOnlineSubsystem::Get();
	IOnlineSessionPtr Sessions = (OnlineSub != NULL) ? OnlineSub->GetSessionInterface() : NULL;

	if (Sessions.IsValid())
	{
		EOnlineSessionState::Type SessionState = Sessions->GetSessionState(GameSessionName);
		UE_LOG(LogOnline, Log, TEXT("Session %s is '%s'"), *GameSessionName.ToString(), EOnlineSessionState::ToString(SessionState));

		if (EOnlineSessionState::InProgress == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Ending session %s on return to main menu"), *GameSessionName.ToString());
			OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			Sessions->EndSession(GameSessionName);
			bPendingOnlineOp = true;
		}
		else if (EOnlineSessionState::Ending == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to end on return to main menu"), *GameSessionName.ToString());
			OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			bPendingOnlineOp = true;
		}
		else if (EOnlineSessionState::Ended == SessionState || EOnlineSessionState::Pending == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Destroying session %s on return to main menu"), *GameSessionName.ToString());
			OnDestroySessionCompleteDelegateHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			Sessions->DestroySession(GameSessionName);
			bPendingOnlineOp = true;
		}
		else if (EOnlineSessionState::Starting == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to start, and then we will end it to return to main menu"), *GameSessionName.ToString());
			OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			bPendingOnlineOp = true;
		}
	}

	if (!bPendingOnlineOp)
	{
		//GEngine->HandleDisconnect( GetWorld(), GetWorld()->GetNetDriver() );
	}
	
	
}

