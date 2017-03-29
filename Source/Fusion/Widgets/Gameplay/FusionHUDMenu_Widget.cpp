// @Maxpro 2017

#include "Fusion.h"

#include "Engine/Console.h"
#include "Widgets/Menus/FusionMenuItem.h"
#include "FusionGameInstance.h"
#include "Player/FusionLocalPlayer.h"
#include "Player/FusionGameUserSettings.h"


#include "FusionHUDMenu_Widget.h"


#define LOCTEXT_NAMESPACE "SFusionHUDMenu_Widget"

#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( FPaths::GameContentDir() / "UI"/ RelativePath + TEXT(".ttf"), __VA_ARGS__ )



#if PLATFORM_XBOXONE
#define PROFILE_SWAPPING	1
#endif


void SFusionHUDMenu_Widget::Construct(const FArguments& InArgs)
{
	
	MenuHeaderBG = new FSlateBrush();
	MenuHeaderBG->SetResourceObject(InArgs.__MenuHeaderBGTexture);
	MenuHeaderBG->ImageSize = FVector2D(286.000000f, 64.000000f);
	MenuHeaderBG->TintColor = FSlateColor(FLinearColor(FColor::White));
	MenuHeaderBG->DrawAs = ESlateBrushDrawType::Image;

	MenuHeaderLeft = new FSlateBrush();
	MenuHeaderLeft->SetResourceObject(InArgs.__MenuLeftBGTexture);
	MenuHeaderLeft->DrawAs = ESlateBrushDrawType::Box;
	MenuHeaderLeft->ImageSize = FVector2D(447.000000f, 208.000000f);
	MenuHeaderLeft->TintColor = FSlateColor(FLinearColor(FColor::White));
	MenuHeaderLeft->Margin = FMargin(0.150000f, 0.150215f, 0.075000f, 0.150215f);

	MenuHeaderRight = new FSlateBrush();
	MenuHeaderRight->SetResourceObject(InArgs.__MenuRightBGTexture);
	MenuHeaderRight->DrawAs = ESlateBrushDrawType::Box;
	MenuHeaderRight->ImageSize = FVector2D(447.f, 208.f);
	MenuHeaderRight->Margin = FMargin(0.150000f, 0.150215f, 0.075000f, 0.150215f);
	MenuHeaderRight->TintColor = FSlateColor(FLinearColor(FColor::White));


	// Fonts still need to be specified in code for now
	Style.Set("Fusion.MenuServerListTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 14))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.ScoreboardListTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 14))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.MenuProfileNameStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 18))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.MenuTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 20))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.MenuHeaderTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 26))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.WelcomeScreen.WelcomeTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 32))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.DefaultScoreboard.Row.HeaderTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 24))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FVector2D(0, 1))
	);

	Style.Set("Fusion.DefaultScoreboard.Row.StatTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 18))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FVector2D(0, 1))
	);

	Style.Set("Fusion.SplitScreenLobby.StartMatchTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 16))
		.SetColorAndOpacity(FLinearColor::Green)
		.SetShadowOffset(FVector2D(0, 1))
	);

	Style.Set("Fusion.DemoListCheckboxTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 12))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);


	bControlsLocked = false;
	bConsoleVisible = false;
	OutlineWidth = 20.0f;
	SelectedIndex = 0;
	PlayerOwner = InArgs._PlayerOwner;
	bGameMenu = InArgs._IsGameMenu;
	ControllerHideMenuKey = EKeys::Gamepad_Special_Right;
	Visibility.Bind(this, &SFusionHUDMenu_Widget::GetSlateVisibility);
	FLinearColor MenuTitleTextColor = FLinearColor(FColor(155, 164, 182));
	MenuHeaderHeight = 62.0f;
	MenuHeaderWidth = 287.0f;

	// Calculate the size of the profile box based on the string it'll contain (+ padding)
	const FText PlayerName = PlayerOwner.IsValid() ? FText::FromString(PlayerOwner->GetNickname()) : FText::GetEmpty();
	const FText ProfileSwap = NSLOCTEXT("Network", "PCSwapProfile", "Space Switch User");;
	const TSharedRef< FSlateFontMeasure > FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const FSlateFontInfo PlayerNameFontInfo = Style.GetWidgetStyle<FTextBlockStyle>("Fusion.MenuProfileNameStyle").Font;
	const FSlateFontInfo ProfileSwapFontInfo = Style.GetWidgetStyle<FTextBlockStyle>("Fusion.MenuServerListTextStyle").Font;
	MenuProfileWidth = FMath::Max(FontMeasure->Measure(PlayerName, PlayerNameFontInfo, 1.0f).X, FontMeasure->Measure(ProfileSwap.ToString(), ProfileSwapFontInfo, 1.0f).X) + 32.0f;

	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		.Padding(GetProfileSwapOffset())
		[
			SNew(SBox)
			.WidthOverride(MenuProfileWidth)
		[
			SNew(SImage)
			.Visibility(this, &SFusionHUDMenu_Widget::GetProfileSwapVisibility)
		.ColorAndOpacity(this, &SFusionHUDMenu_Widget::GetHeaderColor)
		.Image(MenuHeaderBG)
		]
		]
	+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		.Padding(GetProfileSwapOffset())
		[
			SNew(SVerticalBox)
			.Visibility(this, &SFusionHUDMenu_Widget::GetProfileSwapVisibility)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(16.0f, 10.0f, 16.0f, 1.0f)
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		[
			SNew(STextBlock)
			.TextStyle(Style, "Fusion.MenuProfileNameStyle")
		.ColorAndOpacity(MenuTitleTextColor)
		.Text(PlayerName)
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(16.0f, 1.0f, 16.0f, 10.0f)
		[
			SNew(STextBlock)
			.TextStyle(Style, "Fusion.MenuServerListTextStyle")
		.ColorAndOpacity(MenuTitleTextColor)
		.Text(ProfileSwap)
		]
		]
		]
	+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(TAttribute<FMargin>(this, &SFusionHUDMenu_Widget::GetMenuOffset))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			SNew(SBox)
			.WidthOverride(MenuHeaderWidth)
		.HeightOverride(MenuHeaderHeight)
		[
			SNew(SImage)
			.ColorAndOpacity(this, &SFusionHUDMenu_Widget::GetHeaderColor)
		.Image(MenuHeaderBG)
		]
		]
	+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			SNew(SBox)
			.WidthOverride(MenuHeaderWidth)
		.HeightOverride(MenuHeaderHeight)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(Style, "Fusion.MenuHeaderTextStyle")
		.ColorAndOpacity(MenuTitleTextColor)
		.Text(this, &SFusionHUDMenu_Widget::GetMenuTitle)
		]
		]
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
		.ColorAndOpacity(this, &SFusionHUDMenu_Widget::GetBottomColor)
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Left)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(TAttribute<FMargin>(this, &SFusionHUDMenu_Widget::GetLeftMenuOffset))
		[
			SNew(SBorder)
			.BorderImage(MenuHeaderLeft)
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
		.Padding(FMargin(OutlineWidth))
		.DesiredSizeScale(this, &SFusionHUDMenu_Widget::GetBottomScale)
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Left)
		[
			SAssignNew(LeftBox, SVerticalBox)
		]
		]
		]
	+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.Padding(TAttribute<FMargin>(this, &SFusionHUDMenu_Widget::GetSubMenuOffset))
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(MenuHeaderRight)
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
		.Padding(FMargin(OutlineWidth))
		.DesiredSizeScale(this, &SFusionHUDMenu_Widget::GetBottomScale)
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Left)
		[
			SAssignNew(RightBox, SVerticalBox)
		]
		]
		]
		]
		]
		]
		]
		];
}

EVisibility SFusionHUDMenu_Widget::GetSlateVisibility() const
{
	return bConsoleVisible ? EVisibility::HitTestInvisible : EVisibility::Visible;
}

FText SFusionHUDMenu_Widget::GetMenuTitle() const
{
	return CurrentMenuTitle;
}

FMargin SFusionHUDMenu_Widget::GetProfileSwapOffset() const
{
	return FMargin(0.0f, 50.0f, 50.0f, 0.0f);
}

bool SFusionHUDMenu_Widget::IsProfileSwapActive() const
{
#if PROFILE_SWAPPING
	// Dont' show if ingame or not on the main menu screen
	return !bGameMenu && MenuHistory.Num() == 0 ? true : false;
#else
	return false;
#endif
}

EVisibility SFusionHUDMenu_Widget::GetProfileSwapVisibility() const
{
	return IsProfileSwapActive() ? EVisibility::Visible : EVisibility::Collapsed;
}

bool SFusionHUDMenu_Widget::ProfileUISwap(const int ControllerIndex) const // TODO: Do I need this function?
{
	if (IsProfileSwapActive())
	{
		const FOnLoginUIClosedDelegate Delegate = FOnLoginUIClosedDelegate::CreateSP(this, &SFusionHUDMenu_Widget::HandleProfileUISwapClosed);
		if (ProfileSwapUI(ControllerIndex, false, &Delegate))
		{
			UFusionGameInstance* GameInstance = PlayerOwner.IsValid() ? Cast< UFusionGameInstance >(PlayerOwner->GetGameInstance()) : nullptr;

			if (GameInstance != nullptr)
			{
				//GameInstance->SetIgnorePairingChangeForControllerId(ControllerIndex);
			}
			return true;
		}
	}
	return false;
}

void SFusionHUDMenu_Widget::HandleProfileUISwapClosed(TSharedPtr<const FUniqueNetId> UniqueId, const int ControllerIndex) // TODO: Do I need this function?
{
	UFusionGameInstance * GameInstance = PlayerOwner.IsValid() ? Cast< UFusionGameInstance >(PlayerOwner->GetGameInstance()) : nullptr;

	if (GameInstance != nullptr)
	{
		//GameInstance->SetIgnorePairingChangeForControllerId(-1);
	}

	// If the id is null, the user backed out
	if (UniqueId.IsValid() && PlayerOwner.IsValid())
	{
		// If the id is the same, the user picked the existing profile
		// (use the cached unique net id, since we want to compare to the user that was selected at "press start"
		const TSharedPtr<const FUniqueNetId> OwnerId = PlayerOwner->GetCachedUniqueNetId();
		if (OwnerId.IsValid() && UniqueId.IsValid() && *OwnerId == *UniqueId)
		{
			return;
		}

		// Go back to the welcome screen.
		HideMenu();
	}

	UFusionLocalPlayer* LocalPlayer = Cast<UFusionLocalPlayer>(PlayerOwner.Get());
	LocalPlayer->LoadPersistentUser();
}

void SFusionHUDMenu_Widget::LockControls(bool bEnable)
{
	bControlsLocked = bEnable;
}

int32 SFusionHUDMenu_Widget::GetOwnerUserIndex()
{
	return PlayerOwner.IsValid() ? PlayerOwner->GetControllerId() : 0;
}

int32 SFusionHUDMenu_Widget::GetMenuLevel()
{
	return MenuHistory.Num();
}

void SFusionHUDMenu_Widget::BuildAndShowMenu()
{
	//grab the user settings
	UFusionGameUserSettings* UserSettings = CastChecked<UFusionGameUserSettings>(GEngine->GetGameUserSettings());
	ScreenRes = UserSettings->GetScreenResolution();

	//Build left menu panel
	bLeftMenuChanging = false;
	bGoingBack = false;
	BuildLeftPanel(bGoingBack);

	//sets up whole main menu animations and launches them
	SetupAnimations();

	// Set up right side and launch animation if there is any submenu
	if (CurrentMenu.Num() > 0 && CurrentMenu.IsValidIndex(SelectedIndex) && CurrentMenu[SelectedIndex]->bVisible)
	{
		NextMenu = CurrentMenu[SelectedIndex]->SubMenu;
		if (NextMenu.Num() > 0)
		{
			BuildRightPanel();
			bSubMenuChanging = true;
		}
	}

	bMenuHiding = false;
	//FSlateApplication::Get().PlaySound(Style->MenuEnterSound, GetOwnerUserIndex());
}

void SFusionHUDMenu_Widget::HideMenu()
{
	if (!bMenuHiding)
	{
		if (MenuWidgetAnimation.IsAtEnd())
		{
			MenuWidgetAnimation.PlayReverse(this->AsShared());
		}
		else
		{
			MenuWidgetAnimation.Reverse();
		}
		bMenuHiding = true;
	}
}


void SFusionHUDMenu_Widget::SetupAnimations()
{
	//Setup a curve
	const float StartDelay = 0.0f;
	const float SecondDelay = bGameMenu ? 0.f : 0.3f;
	const float AnimDuration = 0.5f;
	const float MenuChangeDuration = 0.2f;

	//always animate the menu from the same side of the screen; it looks silly when it disappears to one place and reappears from another
	AnimNumber = 1;

	MenuWidgetAnimation = FCurveSequence();
	SubMenuWidgetAnimation = FCurveSequence();
	SubMenuScrollOutCurve = SubMenuWidgetAnimation.AddCurve(0, MenuChangeDuration, ECurveEaseFunction::QuadInOut);

	MenuWidgetAnimation = FCurveSequence();
	LeftMenuWidgetAnimation = FCurveSequence();
	LeftMenuScrollOutCurve = LeftMenuWidgetAnimation.AddCurve(0, MenuChangeDuration, ECurveEaseFunction::QuadInOut);
	LeftMenuWidgetAnimation.Play(this->AsShared());

	//Define the fade in animation
	TopColorCurve = MenuWidgetAnimation.AddCurve(StartDelay, AnimDuration, ECurveEaseFunction::QuadInOut);

	//now, we want these to animate some time later

	//rolling out
	BottomScaleYCurve = MenuWidgetAnimation.AddCurve(StartDelay + SecondDelay, AnimDuration, ECurveEaseFunction::QuadInOut);
	//fading in
	BottomColorCurve = MenuWidgetAnimation.AddCurve(StartDelay + SecondDelay, AnimDuration, ECurveEaseFunction::QuadInOut);
	//moving from left side off screen
	ButtonsPosXCurve = MenuWidgetAnimation.AddCurve(StartDelay + SecondDelay, AnimDuration, ECurveEaseFunction::QuadInOut);
}

void SFusionHUDMenu_Widget::BuildLeftPanel(bool bInGoingBack)
{
	if (CurrentMenu.Num() == 0)
	{
		//do not build anything if we do not have any active menu
		return;
	}
	LeftBox->ClearChildren();
	int32 PreviousIndex = -1;
	if (bLeftMenuChanging)
	{
		if (bInGoingBack && MenuHistory.Num() > 0)
		{
			FFusionMenuInfo MenuInfo = MenuHistory.Pop();
			CurrentMenu = MenuInfo.Menu;
			CurrentMenuTitle = MenuInfo.MenuTitle;
			PreviousIndex = MenuInfo.SelectedIndex;
			if (CurrentMenu.Num() > 0 && CurrentMenu[PreviousIndex]->SubMenu.Num() > 0)
			{
				NextMenu = CurrentMenu[PreviousIndex]->SubMenu;
				bSubMenuChanging = true;
			}
		}
		else if (PendingLeftMenu.Num() > 0)
		{
			MenuHistory.Push(FFusionMenuInfo(CurrentMenu, SelectedIndex, CurrentMenuTitle));
			CurrentMenuTitle = CurrentMenu[SelectedIndex]->GetText();
			CurrentMenu = PendingLeftMenu;
		}
	}
	SelectedIndex = PreviousIndex;
	//Setup the buttons
	for (int32 i = 0; i < CurrentMenu.Num(); ++i)
	{
		if (CurrentMenu[i]->bVisible)
		{
			TSharedPtr<SWidget> TmpWidget;
			if (CurrentMenu[i]->MenuItemType == EFusionMenuItemType::Standard)
			{
				TmpWidget = SAssignNew(CurrentMenu[i]->Widget, SFusionMenuItem)
					.PlayerOwner(PlayerOwner)
					.OnClicked(this, &SFusionHUDMenu_Widget::ButtonClicked, i)
					.Text(CurrentMenu[i]->GetText())
					.bIsMultichoice(false);
			}
			else if (CurrentMenu[i]->MenuItemType == EFusionMenuItemType::MultiChoice)
			{
				TmpWidget = SAssignNew(CurrentMenu[i]->Widget, SFusionMenuItem)
					.PlayerOwner(PlayerOwner)
					.OnClicked(this, &SFusionHUDMenu_Widget::ButtonClicked, i)
					.Text(CurrentMenu[i]->GetText())
					.bIsMultichoice(true)
					.OnArrowPressed(this, &SFusionHUDMenu_Widget::ChangeOption)
					.OptionText(this, &SFusionHUDMenu_Widget::GetOptionText, CurrentMenu[i]);
				UpdateArrows(CurrentMenu[i]);
			}
			else if (CurrentMenu[i]->MenuItemType == EFusionMenuItemType::CustomWidget)
			{
				TmpWidget = CurrentMenu[i]->CustomWidget;
			}
			if (TmpWidget.IsValid())
			{
				//set first selection to first valid widget
				if (SelectedIndex == -1)
				{
					SelectedIndex = i;
				}
				LeftBox->AddSlot().HAlign(HAlign_Left).AutoHeight()
					[
						TmpWidget.ToSharedRef()
					];
			}
		}
	}


	TSharedPtr<FFusionMenuItemData> FirstMenuItem = CurrentMenu.IsValidIndex(SelectedIndex) ? CurrentMenu[SelectedIndex] : NULL;
	if (FirstMenuItem.IsValid() && FirstMenuItem->MenuItemType != EFusionMenuItemType::CustomWidget)
	{
		FirstMenuItem->Widget->SetMenuItemActive(true);
		FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
	}
}

FText SFusionHUDMenu_Widget::GetOptionText(TSharedPtr<FFusionMenuItemData> MenuItem) const
{
	FText Result = FText::GetEmpty();
	if (MenuItem->SelectedMultiChoice > -1 && MenuItem->SelectedMultiChoice < MenuItem->MultiChoice.Num())
	{
		Result = MenuItem->MultiChoice[MenuItem->SelectedMultiChoice];
	}
	return Result;
}

void SFusionHUDMenu_Widget::BuildRightPanel()
{
	RightBox->ClearChildren();

	if (NextMenu.Num() == 0) return;

	for (int32 i = 0; i < NextMenu.Num(); ++i)
	{
		if (NextMenu[i]->bVisible)
		{
			TSharedPtr<SFusionMenuItem> TmpButton;
			//Custom menu items are not supported in the right panel
			if (NextMenu[i]->MenuItemType == EFusionMenuItemType::Standard)
			{
				TmpButton = SAssignNew(NextMenu[i]->Widget, SFusionMenuItem)
					.PlayerOwner(PlayerOwner)
					.Text(NextMenu[i]->GetText())
					.InactiveTextAlpha(0.3f)
					.bIsMultichoice(false);
			}
			else if (NextMenu[i]->MenuItemType == EFusionMenuItemType::MultiChoice)
			{
				TmpButton = SAssignNew(NextMenu[i]->Widget, SFusionMenuItem)
					.PlayerOwner(PlayerOwner)
					.Text(NextMenu[i]->GetText())
					.InactiveTextAlpha(0.3f)
					.bIsMultichoice(true)
					.OptionText(this, &SFusionHUDMenu_Widget::GetOptionText, NextMenu[i]);
			}
			if (TmpButton.IsValid())
			{
				RightBox->AddSlot()
					.HAlign(HAlign_Center)
					.AutoHeight()
					[
						TmpButton.ToSharedRef()
					];
			}
		}
	}
}

void SFusionHUDMenu_Widget::UpdateArrows(TSharedPtr<FFusionMenuItemData> MenuItem)
{
	const int32 MinIndex = MenuItem->MinMultiChoiceIndex > -1 ? MenuItem->MinMultiChoiceIndex : 0;
	const int32 MaxIndex = MenuItem->MaxMultiChoiceIndex > -1 ? MenuItem->MaxMultiChoiceIndex : MenuItem->MultiChoice.Num() - 1;
	const int32 CurIndex = MenuItem->SelectedMultiChoice;
	if (CurIndex > MinIndex)
	{
		MenuItem->Widget->LeftArrowVisible = EVisibility::Visible;
	}
	else
	{
		MenuItem->Widget->LeftArrowVisible = EVisibility::Collapsed;
	}
	if (CurIndex < MaxIndex)
	{
		MenuItem->Widget->RightArrowVisible = EVisibility::Visible;
	}
	else
	{
		MenuItem->Widget->RightArrowVisible = EVisibility::Collapsed;
	}
}

void SFusionHUDMenu_Widget::EnterSubMenu()
{
	bLeftMenuChanging = true;
	bGoingBack = false;
	//FSlateApplication::Get().PlaySound(MenuStyle->MenuEnterSound, GetOwnerUserIndex());
}

void SFusionHUDMenu_Widget::MenuGoBack(bool bSilent)
{
	if (MenuHistory.Num() > 0)
	{
		if (!bSilent)
		{
			//FSlateApplication::Get().PlaySound(MenuStyle->MenuBackSound, GetOwnerUserIndex());
		}
		bLeftMenuChanging = true;
		bGoingBack = true;
		OnGoBack.ExecuteIfBound(CurrentMenu);
	}
	else if (bGameMenu) // only when it's in-game menu variant
	{
		if (!bSilent)
		{
			//FSlateApplication::Get().PlaySound(MenuStyle->MenuBackSound, GetOwnerUserIndex());
		}
		OnToggleMenu.ExecuteIfBound();
	}
	else
	{
#if SHOOTER_CONSOLE_UI
		// Go back to the welcome screen.
		HideMenu();
#endif
	}
}

void SFusionHUDMenu_Widget::ConfirmMenuItem()
{
	if (CurrentMenu[SelectedIndex]->OnConfirmMenuItem.IsBound())
	{
		CurrentMenu[SelectedIndex]->OnConfirmMenuItem.Execute();
	}
	else if (CurrentMenu[SelectedIndex]->SubMenu.Num() > 0)
	{
		EnterSubMenu();
	}
}

void SFusionHUDMenu_Widget::ControllerFacebuttonLeftPressed()
{
	if (CurrentMenu[SelectedIndex]->OnControllerFacebuttonLeftPressed.IsBound())
	{
		CurrentMenu[SelectedIndex]->OnControllerFacebuttonLeftPressed.Execute();
	}
}

void SFusionHUDMenu_Widget::ControllerUpInputPressed()
{
	if (CurrentMenu[SelectedIndex]->OnControllerUpInputPressed.IsBound())
	{
		CurrentMenu[SelectedIndex]->OnControllerUpInputPressed.Execute();
	}
}

void SFusionHUDMenu_Widget::ControllerDownInputPressed()
{
	if (CurrentMenu[SelectedIndex]->OnControllerDownInputPressed.IsBound())
	{
		CurrentMenu[SelectedIndex]->OnControllerDownInputPressed.Execute();
	}
}

void SFusionHUDMenu_Widget::ControllerFacebuttonDownPressed()
{
	if (CurrentMenu[SelectedIndex]->OnControllerFacebuttonDownPressed.IsBound())
	{
		CurrentMenu[SelectedIndex]->OnControllerFacebuttonDownPressed.Execute();
	}
}

void SFusionHUDMenu_Widget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	//Always tick the super
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	//ugly code seeing if the console is open
	UConsole* ViewportConsole = (GEngine != NULL && GEngine->GameViewport != NULL) ? GEngine->GameViewport->ViewportConsole : NULL;
	if (ViewportConsole != NULL && (ViewportConsole->ConsoleState == "Typing" || ViewportConsole->ConsoleState == "Open"))
	{
		if (!bConsoleVisible)
		{
			bConsoleVisible = true;
			FSlateApplication::Get().SetAllUserFocusToGameViewport();
		}
	}
	else
	{
		if (bConsoleVisible)
		{
			bConsoleVisible = false;
			FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
		}
	}

	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportFrame)
	{
		FViewport* Viewport = GEngine->GameViewport->ViewportFrame->GetViewport();
		if (Viewport)
		{
			ScreenRes = Viewport->GetSizeXY();
		}
	}

	if (MenuWidgetAnimation.IsAtStart() && !bMenuHiding)
	{
		//Start the menu widget animation, set keyboard focus
		FadeIn();
	}
	else if (MenuWidgetAnimation.IsAtStart() && bMenuHiding)
	{
		bMenuHiding = false;
		//Send event, so menu can be removed
		OnMenuHidden.ExecuteIfBound();
	}

	if (MenuWidgetAnimation.IsAtEnd())
	{
		if (bLeftMenuChanging)
		{
			if (LeftMenuWidgetAnimation.IsAtEnd())
			{
				PendingLeftMenu = NextMenu;
				if (NextMenu.Num() > 0
					&& NextMenu.Top()->SubMenu.Num() > 0)
				{
					NextMenu = NextMenu.Top()->SubMenu;
				}
				else
				{
					NextMenu.Reset();
				}
				bSubMenuChanging = true;

				LeftMenuWidgetAnimation.PlayReverse(this->AsShared());
			}
			if (!LeftMenuWidgetAnimation.IsPlaying())
			{
				if (CurrentMenu.Num() > 0)
				{
					BuildLeftPanel(bGoingBack);
					LeftMenuWidgetAnimation.Play(this->AsShared());
				}
				//Focus the custom widget
				if (CurrentMenu.Num() == 1 && CurrentMenu.Top()->MenuItemType == EFusionMenuItemType::CustomWidget)
				{
					FSlateApplication::Get().SetKeyboardFocus(CurrentMenu.Top()->CustomWidget);
				}
				bLeftMenuChanging = false;
				RightBox->ClearChildren();
			}
		}
		if (bSubMenuChanging)
		{
			if (SubMenuWidgetAnimation.IsAtEnd())
			{
				SubMenuWidgetAnimation.PlayReverse(this->AsShared());
			}
			if (!SubMenuWidgetAnimation.IsPlaying())
			{
				if (NextMenu.Num() > 0)
				{
					BuildRightPanel();
					SubMenuWidgetAnimation.Play(this->AsShared());
				}
				bSubMenuChanging = false;
			}
		}
	}
}

FMargin SFusionHUDMenu_Widget::GetMenuOffset() const
{
	const float WidgetWidth = LeftBox->GetDesiredSize().X + RightBox->GetDesiredSize().X;
	const float WidgetHeight = LeftBox->GetDesiredSize().Y + MenuHeaderHeight;
	const float OffsetX = (ScreenRes.X - WidgetWidth - OutlineWidth * 2) / 2;
	const float AnimProgress = ButtonsPosXCurve.GetLerp();
	FMargin Result;

	switch (AnimNumber)
	{
	case 0:
		Result = FMargin(OffsetX + ScreenRes.X - AnimProgress*ScreenRes.X, (ScreenRes.Y - WidgetHeight) / 2, 0, 0);
		break;
	case 1:
		Result = FMargin(OffsetX - ScreenRes.X + AnimProgress*ScreenRes.X, (ScreenRes.Y - WidgetHeight) / 2, 0, 0);
		break;
	case 2:
		Result = FMargin(OffsetX, (ScreenRes.Y - WidgetHeight) / 2 + ScreenRes.Y - AnimProgress*ScreenRes.Y, 0, 0);
		break;
	case 3:
		Result = FMargin(OffsetX, (ScreenRes.Y - WidgetHeight) / 2 + -ScreenRes.Y + AnimProgress*ScreenRes.Y, 0, 0);
		break;
	}
	return Result;
}

FMargin SFusionHUDMenu_Widget::GetLeftMenuOffset() const
{
	const float LeftBoxSizeX = LeftBox->GetDesiredSize().X + OutlineWidth * 2;
	return FMargin(0, 0, -LeftBoxSizeX + LeftMenuScrollOutCurve.GetLerp() * LeftBoxSizeX, 0);
}

FMargin SFusionHUDMenu_Widget::GetSubMenuOffset() const
{
	const float RightBoxSizeX = RightBox->GetDesiredSize().X + OutlineWidth * 2;
	return FMargin(0, 0, -RightBoxSizeX + SubMenuScrollOutCurve.GetLerp() * RightBoxSizeX, 0);
}


FVector2D SFusionHUDMenu_Widget::GetBottomScale() const
{
	return FVector2D(BottomScaleYCurve.GetLerp(), BottomScaleYCurve.GetLerp());
}

FLinearColor SFusionHUDMenu_Widget::GetBottomColor() const
{
	return FMath::Lerp(FLinearColor(1, 1, 1, 0), FLinearColor(1, 1, 1, 1), BottomColorCurve.GetLerp());
}

FLinearColor SFusionHUDMenu_Widget::GetTopColor() const
{
	return FMath::Lerp(FLinearColor(1, 1, 1, 0), FLinearColor(1, 1, 1, 1), TopColorCurve.GetLerp());
}

FSlateColor SFusionHUDMenu_Widget::GetHeaderColor() const
{
	return CurrentMenuTitle.IsEmpty() ? FLinearColor::Transparent : FLinearColor::White;
}

FReply SFusionHUDMenu_Widget::ButtonClicked(int32 ButtonIndex)
{
	if (bControlsLocked)
	{
		return FReply::Handled();
	}

	if (SelectedIndex != ButtonIndex)
	{
		TSharedPtr<SFusionMenuItem> MenuItem = CurrentMenu[SelectedIndex]->Widget;
		MenuItem->SetMenuItemActive(false);
		SelectedIndex = ButtonIndex;
		MenuItem = CurrentMenu[SelectedIndex]->Widget;
		MenuItem->SetMenuItemActive(true);
		NextMenu = CurrentMenu[SelectedIndex]->SubMenu;
		bSubMenuChanging = true;
		//FSlateApplication::Get().PlaySound(MenuStyle->MenuItemChangeSound, GetOwnerUserIndex());
	}
	else if (SelectedIndex == ButtonIndex)
	{
		ConfirmMenuItem();
	}

	return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::SetDirectly);
}

void SFusionHUDMenu_Widget::FadeIn()
{
	//Start the menu widget playing
	MenuWidgetAnimation.Play(this->AsShared());

	//Go into UI mode
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
}

FReply SFusionHUDMenu_Widget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	//If we clicked anywhere, jump to the end
	if (MenuWidgetAnimation.IsPlaying())
	{
		MenuWidgetAnimation.JumpToEnd();
	}

	//Set the keyboard focus 
	return FReply::Handled()
		.SetUserFocus(SharedThis(this), EFocusCause::SetDirectly);
}

void SFusionHUDMenu_Widget::ChangeOption(int32 MoveBy)
{
	TSharedPtr<FFusionMenuItemData> MenuItem = CurrentMenu[SelectedIndex];

	const int32 MinIndex = MenuItem->MinMultiChoiceIndex > -1 ? MenuItem->MinMultiChoiceIndex : 0;
	const int32 MaxIndex = MenuItem->MaxMultiChoiceIndex > -1 ? MenuItem->MaxMultiChoiceIndex : MenuItem->MultiChoice.Num() - 1;
	const int32 CurIndex = MenuItem->SelectedMultiChoice;

	if (MenuItem->MenuItemType == EFusionMenuItemType::MultiChoice)
	{
		if (CurIndex + MoveBy >= MinIndex && CurIndex + MoveBy <= MaxIndex)
		{
			MenuItem->SelectedMultiChoice += MoveBy;
			MenuItem->OnOptionChanged.ExecuteIfBound(MenuItem, MenuItem->SelectedMultiChoice);
			//FSlateApplication::Get().PlaySound(MenuStyle->OptionChangeSound, GetOwnerUserIndex());
		}
		UpdateArrows(MenuItem);
	}
}

int32 SFusionHUDMenu_Widget::GetNextValidIndex(int32 MoveBy)
{
	int32 Result = SelectedIndex;
	if (MoveBy != 0 && SelectedIndex + MoveBy > -1 && SelectedIndex + MoveBy < CurrentMenu.Num())
	{
		Result = SelectedIndex + MoveBy;
		//look for non-hidden menu item
		while (!CurrentMenu[Result]->Widget.IsValid())
		{
			MoveBy > 0 ? Result++ : Result--;
			//when moved outside of array, just return current selection
			if (!CurrentMenu.IsValidIndex(Result))
			{
				Result = SelectedIndex;
				break;
			}
		}
	}
	return Result;
}

FReply SFusionHUDMenu_Widget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	FReply Result = FReply::Unhandled();
	const int32 UserIndex = InKeyEvent.GetUserIndex();
	bool bEventUserCanInteract = GetOwnerUserIndex() == -1 || UserIndex == GetOwnerUserIndex();

	if (!bControlsLocked && bEventUserCanInteract)
	{
		const FKey Key = InKeyEvent.GetKey();
		if (Key == EKeys::Up || Key == EKeys::Gamepad_DPad_Up || Key == EKeys::Gamepad_LeftStick_Up)
		{
			ControllerUpInputPressed();
			int32 NextValidIndex = GetNextValidIndex(-1);
			if (NextValidIndex != SelectedIndex)
			{
				ButtonClicked(NextValidIndex);
			}
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Down || Key == EKeys::Gamepad_DPad_Down || Key == EKeys::Gamepad_LeftStick_Down)
		{
			ControllerDownInputPressed();
			int32 NextValidIndex = GetNextValidIndex(1);
			if (NextValidIndex != SelectedIndex)
			{
				ButtonClicked(NextValidIndex);
			}
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Left || Key == EKeys::Gamepad_DPad_Left || Key == EKeys::Gamepad_LeftStick_Left)
		{
			ChangeOption(-1);
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Right || Key == EKeys::Gamepad_DPad_Right || Key == EKeys::Gamepad_LeftStick_Right)
		{
			ChangeOption(1);
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Gamepad_FaceButton_Top)
		{
			ProfileUISwap(UserIndex);
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Enter)
		{
			ConfirmMenuItem();
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Gamepad_FaceButton_Bottom && !InKeyEvent.IsRepeat())
		{
			ControllerFacebuttonDownPressed();
			ConfirmMenuItem();
			Result = FReply::Handled();
		}
		else if ((Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Gamepad_Special_Left || Key == EKeys::Global_Back || Key == EKeys::Global_View) && !InKeyEvent.IsRepeat())
		{
			MenuGoBack();
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Gamepad_FaceButton_Left)
		{
			ControllerFacebuttonLeftPressed();
			Result = FReply::Handled();
		}
		else if ((Key == ControllerHideMenuKey || Key == EKeys::Global_Play || Key == EKeys::Global_Menu) && !InKeyEvent.IsRepeat())
		{
			OnToggleMenu.ExecuteIfBound();
			Result = FReply::Handled();
		}
	}
	return Result;
}

FReply SFusionHUDMenu_Widget::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	//Focus the custom widget
	if (CurrentMenu.Num() == 1 && CurrentMenu.Top()->MenuItemType == EFusionMenuItemType::CustomWidget)
	{
		return FReply::Handled().SetUserFocus(CurrentMenu.Top()->CustomWidget.ToSharedRef(), EFocusCause::SetDirectly);
	}

	return FReply::Handled().ReleaseMouseCapture().SetUserFocus(SharedThis(this), EFocusCause::SetDirectly, true);
}

// added 3/27/17
bool SFusionHUDMenu_Widget::ProfileSwapUI(const int ControllerIndex, bool bShowOnlineOnly, const FOnLoginUIClosedDelegate* Delegate) const
{
	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Show the profile UI.
		const auto ExternalUI = OnlineSub->GetExternalUIInterface();
		if (ExternalUI.IsValid())
		{
			// Create a dummy delegate, if one wasn't specified
			struct Local
			{
				static void DummyOnProfileSwapUIClosedDelegate(TSharedPtr<const FUniqueNetId> UniqueId, const int InControllerIndex)
				{
					// do nothing
				}
			};
			return ExternalUI->ShowLoginUI(ControllerIndex, bShowOnlineOnly, Delegate ? *Delegate : FOnLoginUIClosedDelegate::CreateStatic(&Local::DummyOnProfileSwapUIClosedDelegate));
		}
	}
	return false;
}


#undef TTF_FONT
#undef LOCTEXT_NAMESPACE
