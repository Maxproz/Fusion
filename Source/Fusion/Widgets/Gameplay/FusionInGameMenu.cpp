// @Maxpro 2017

#include "Fusion.h"
#include "FusionInGameMenu.h"
#include "Online.h"
#include "OnlineExternalUIInterface.h"
#include "FusionGameInstance.h"
#include "FusionPlayerController.h"
#include "FusionHUD.h"

#define LOCTEXT_NAMESPACE "Fusion.HUD.Menu"

void FFusionInGameMenu::Construct(ULocalPlayer* _PlayerOwner)
{
	PlayerOwner = _PlayerOwner;
	bIsGameMenuUp = false;

	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}

	//todo:  don't create ingame menus for remote players.
	const UFusionGameInstance* GameInstance = nullptr;
	if (PlayerOwner)
	{
		GameInstance = Cast<UFusionGameInstance>(PlayerOwner->GetGameInstance());

	}
	
	AFusionPlayerController* const PCOwner = PlayerOwner ? Cast<AFusionPlayerController>(PlayerOwner->PlayerController) : nullptr;

	if (!InGameMenuWidget.IsValid() && PCOwner)
	{
		SAssignNew(InGameMenuWidget, SFusionHUDMenu_Widget)
			.PlayerOwner(TWeakObjectPtr<ULocalPlayer>(PlayerOwner))
			.Cursor(EMouseCursor::Default)
			.IsGameMenu(true)
			._MenuHeaderBGTexture(PCOwner->MenuHeaderBGTexture)
			._MenuLeftBGTexture(PCOwner->MenuLeftBGTexture)
			._MenuRightBGTexture(PCOwner->MenuRightBGTexture);


		int32 const OwnerUserIndex = GetOwnerUserIndex();

		// setup the exit to main menu submenu.  We wanted a confirmation to avoid a potential TRC violation.
		// fixes TTP: 322267
		TSharedPtr<FFusionMenuItemData> MainMenuRoot = FFusionMenuItemData::CreateRoot();
		MainMenuItem = MenuHelper::AddMenuItem(MainMenuRoot, LOCTEXT("Main Menu", "MAIN MENU"));
		MenuHelper::AddMenuItemSP(MainMenuItem, LOCTEXT("No", "NO"), this, &FFusionInGameMenu::OnCancelExitToMain);
		MenuHelper::AddMenuItemSP(MainMenuItem, LOCTEXT("Yes", "YES"), this, &FFusionInGameMenu::OnConfirmExitToMain);

		FusionOptions = MakeShareable(new FFusionOptions());
		FusionOptions->Construct(PlayerOwner);
		FusionOptions->TellInputAboutKeybindings();
		FusionOptions->OnApplyChanges.BindSP(this, &FFusionInGameMenu::CloseSubMenu);

		MenuHelper::AddExistingMenuItem(RootMenuItem, FusionOptions->CheatsItem.ToSharedRef());
		MenuHelper::AddExistingMenuItem(RootMenuItem, FusionOptions->OptionsItem.ToSharedRef());
		
		/* // TODO: Not implemented
		if (GameInstance && GameInstance->GetIsOnline())
		{
#if !PLATFORM_XBOXONE
			ShooterFriends = MakeShareable(new FShooterFriends());
			ShooterFriends->Construct(PlayerOwner, OwnerUserIndex);
			ShooterFriends->TellInputAboutKeybindings();
			ShooterFriends->OnApplyChanges.BindSP(this, &FFusionInGameMenu::CloseSubMenu);

			MenuHelper::AddExistingMenuItem(RootMenuItem, ShooterFriends->FriendsItem.ToSharedRef());

			ShooterRecentlyMet = MakeShareable(new FShooterRecentlyMet());
			ShooterRecentlyMet->Construct(PlayerOwner, OwnerUserIndex);
			ShooterRecentlyMet->TellInputAboutKeybindings();
			ShooterRecentlyMet->OnApplyChanges.BindSP(this, &FFusionInGameMenu::CloseSubMenu);

			MenuHelper::AddExistingMenuItem(RootMenuItem, ShooterRecentlyMet->RecentlyMetItem.ToSharedRef());
#endif		

#if SHOOTER_CONSOLE_UI			
			TSharedPtr<FFusionMenuItemData> ShowInvitesItem = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Invite Players", "INVITE PLAYERS"));
			ShowInvitesItem->OnConfirmMenuItem.BindRaw(this, &FFusionInGameMenu::OnShowInviteUI);
#endif
		}*/

		if (FSlateApplication::Get().SupportsSystemHelp())
		{
			TSharedPtr<FFusionMenuItemData> HelpSubMenu = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Help", "HELP"));
			HelpSubMenu->OnConfirmMenuItem.BindStatic([]() { FSlateApplication::Get().ShowSystemHelp(); });
		}

		MenuHelper::AddExistingMenuItem(RootMenuItem, MainMenuItem.ToSharedRef());

#if !SHOOTER_CONSOLE_UI
		MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("Quit", "QUIT"), this, &FFusionInGameMenu::OnUIQuit);
#endif

		InGameMenuWidget->MainMenu = InGameMenuWidget->CurrentMenu = RootMenuItem->SubMenu;
		InGameMenuWidget->OnMenuHidden.BindSP(this, &FFusionInGameMenu::DetachGameMenu);
		InGameMenuWidget->OnToggleMenu.BindSP(this, &FFusionInGameMenu::ToggleGameMenu);
		InGameMenuWidget->OnGoBack.BindSP(this, &FFusionInGameMenu::OnMenuGoBack);
	}
}

void FFusionInGameMenu::CloseSubMenu()
{
	InGameMenuWidget->MenuGoBack();
}

void FFusionInGameMenu::OnMenuGoBack(MenuPtr Menu)
{
	// if we are going back from options menu
	if (FusionOptions.IsValid() && FusionOptions->OptionsItem->SubMenu == Menu)
	{
		FusionOptions->RevertChanges();
	}
}

bool FFusionInGameMenu::GetIsGameMenuUp() const
{
	return bIsGameMenuUp;
}

void FFusionInGameMenu::UpdateFriendsList()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineFriendsPtr OnlineFriendsPtr = OnlineSub->GetFriendsInterface();
		if (OnlineFriendsPtr.IsValid())
		{
			OnlineFriendsPtr->ReadFriendsList(GetOwnerUserIndex(), EFriendsLists::ToString(EFriendsLists::OnlinePlayers));
		}
	}
}

void FFusionInGameMenu::DetachGameMenu()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(GameMenuContainer.ToSharedRef());
	}
	bIsGameMenuUp = false;

	AFusionPlayerController* const PCOwner = PlayerOwner ? Cast<AFusionPlayerController>(PlayerOwner->PlayerController) : nullptr;
	if (PCOwner)
	{
		PCOwner->SetPause(false);

		// If the game is over enable the scoreboard
		AFusionHUD* const FusionHUD = PCOwner->GetFusionHUD();
		if ((FusionHUD != NULL) && (FusionHUD->IsMatchOver() == true) && (PCOwner->IsPrimaryPlayer() == true))
		{
			FusionHUD->ShowScoreboard(true, true);
		}
	}
}

void FFusionInGameMenu::ToggleGameMenu()
{
	//Update the owner in case the menu was opened by another controller
	//UpdateMenuOwner();

	if (!InGameMenuWidget.IsValid())
	{
		return;
	}

	// check for a valid user index.  could be invalid if the user signed out, in which case the 'please connect your control' ui should be up anyway.
	// in-game menu needs a valid userindex for many OSS calls.
	if (GetOwnerUserIndex() == -1)
	{
		UE_LOG(LogTemp, Log, TEXT("Trying to toggle in-game menu for invalid userid"));
		return;
	}

	if (bIsGameMenuUp && InGameMenuWidget->CurrentMenu != RootMenuItem->SubMenu)
	{
		InGameMenuWidget->MenuGoBack();
		return;
	}

	AFusionPlayerController* const PCOwner = PlayerOwner ? Cast<AFusionPlayerController>(PlayerOwner->PlayerController) : nullptr;
	if (!bIsGameMenuUp)
	{
		// Hide the scoreboard
		if (PCOwner)
		{
			AFusionHUD* const FusionHUD = PCOwner->GetFusionHUD();
			if (FusionHUD != NULL)
			{
				FusionHUD->ShowScoreboard(false);
			}
		}

		GEngine->GameViewport->AddViewportWidgetContent(
			SAssignNew(GameMenuContainer, SWeakWidget)
			.PossiblyNullContent(InGameMenuWidget.ToSharedRef())
		);

		int32 const OwnerUserIndex = GetOwnerUserIndex();
		if (FusionOptions.IsValid())
		{
			FusionOptions->UpdateOptions(); // Main place where settings are updated in the options menu
		}

		/*
		if (ShooterRecentlyMet.IsValid())
		{
			ShooterRecentlyMet->UpdateRecentlyMet(OwnerUserIndex);
		}*/

		InGameMenuWidget->BuildAndShowMenu();
		bIsGameMenuUp = true;

		if (PCOwner)
		{
			// Disable controls while paused
			PCOwner->SetCinematicMode(true, false, false, true, true);

			PCOwner->SetPause(true);
		}
	}
	else
	{
		//Start hiding animation
		InGameMenuWidget->HideMenu();
		if (PCOwner)
		{
			// Make sure viewport has focus
			FSlateApplication::Get().SetAllUserFocusToGameViewport();

			// Don't renable controls if the match is over
			AFusionHUD* const FusionHUD = PCOwner->GetFusionHUD();
			if ((FusionHUD != NULL) && (FusionHUD->IsMatchOver() == false))
			{
				PCOwner->SetCinematicMode(false, false, false, true, true);
			}
		}
	}
}

void FFusionInGameMenu::OnCancelExitToMain()
{
	CloseSubMenu();
}

void FFusionInGameMenu::OnConfirmExitToMain()
{
	UFusionGameInstance* const GameInstance = Cast<UFusionGameInstance>(PlayerOwner->GetGameInstance());
	if (GameInstance)
	{
		GameInstance->LabelPlayerAsQuitter(PlayerOwner);

		// tell game instance to go back to main menu state
		GameInstance->GotoState(FusionGameInstanceState::MainMenu);
	}
}

void FFusionInGameMenu::OnUIQuit()
{
	UFusionGameInstance* const GI = Cast<UFusionGameInstance>(PlayerOwner->GetGameInstance());
	if (GI)
	{
		GI->LabelPlayerAsQuitter(PlayerOwner);
	}
	
	InGameMenuWidget->LockControls(true);
	InGameMenuWidget->HideMenu();

	

	UWorld* const World = PlayerOwner ? PlayerOwner->GetWorld() : nullptr;
	if (World)
	{
		AFusionPlayerController* MyPC = Cast<AFusionPlayerController>(PlayerOwner->GetPlayerController(World));
		if (MyPC)
		{
			MenuHelper::PlaySoundAndCall(World, MyPC->ExitGameSound, GetOwnerUserIndex(), this, &FFusionInGameMenu::Quit); // TODO: Can I set this like so?
		}
	}
}

void FFusionInGameMenu::Quit()
{
	APlayerController* const PCOwner = PlayerOwner ? PlayerOwner->PlayerController : nullptr;
	if (PCOwner)
	{
		PCOwner->ConsoleCommand("quit");
	}
}

void FFusionInGameMenu::OnShowInviteUI()
{
	const auto ExternalUI = Online::GetExternalUIInterface();

	if (!ExternalUI.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnShowInviteUI: External UI interface is not supported on this platform."));
		return;
	}

	ExternalUI->ShowInviteUI(GetOwnerUserIndex());
}

int32 FFusionInGameMenu::GetOwnerUserIndex() const
{
	return PlayerOwner ? PlayerOwner->GetControllerId() : 0;
}


#undef LOCTEXT_NAMESPACE
