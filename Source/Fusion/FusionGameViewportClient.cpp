// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"
#include "SSafeZone.h"
#include "SThrobber.h"

#include "FusionBaseCharacter.h"

#include "FusionHUD.h"
#include "FusionPlayerController_Menu.h"

#include "Widgets/Menus/ConfirmationDialog_Widget.h"

#include "Player/FusionLocalPlayer.h"

#include "FusionGameViewportClient.h"


UFusionGameViewportClient::UFusionGameViewportClient(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetSuppressTransitionMessage(true);
}


void UFusionGameViewportClient::ShowDialog(TWeakObjectPtr<ULocalPlayer> InPlayerOwner, const FText& Message, EFusionDialogType::Type DialogType, TScriptDelegate<FWeakObjectPtr> OnConfirm, TScriptDelegate<FWeakObjectPtr> OnCancel)
{
	UE_LOG(LogPlayerManagement, Log, TEXT("UFusionGameViewportClient::ShowDialog..."));
	
	AFusionPlayerController_Menu* MPC = Cast<AFusionPlayerController_Menu>(InPlayerOwner->GetPlayerController(GetWorld()));
	if (MPC)
	{
		CurrentPlayersHUD = Cast<AFusionHUD>(MPC->GetHUD());
		if (CurrentPlayersHUD)
		{
			if (DialogWidget)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Emerald, FString::Printf(TEXT("Cannt show dialog ~~~ Already showing a dialog box")));
				return;	// Already showing a dialog box
			}

			// Hide all existing widgets
			if (!LoadingScreenWidget.IsValid())
			{
				//HideExistingWidgets();
				CurrentPlayersHUD->HideMainMenu();
			}
			
			

			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Emerald, FString::Printf(TEXT("Creating A DialogWidget")));

			DialogWidget = CreateWidget<UConfirmationDialog_Widget>(MPC, CurrentPlayersHUD->ConfirmationDialog_WidgetTemplate);
			DialogWidget->AddToViewport(2);

			if (DialogWidget)
			{
				DialogWidget->FinishConstruct(InPlayerOwner, Message, DialogType, OnConfirm, OnCancel);
				DialogWidget->OnRep_DisplayMessage();
			}

			if (LoadingScreenWidget.IsValid())
			{
				// Can't show dialog while loading screen is visible
				DialogWidget->HideWidget();
			}
			else
			{

				DialogWidget->ShowWidget();
			}
		}
	}
}

void UFusionGameViewportClient::HideDialog()
{
	UE_LOG(LogPlayerManagement, Log, TEXT("UFusionGameViewportClient::HideDialog. DialogWidget: %p, OldFocusWidget: %p"), DialogWidget, OldFocusWidget.Get());

	if (DialogWidget)
	{
		// Destroy the dialog widget
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Emerald, FString::Printf(TEXT("Destroying A DialogWidget")));

		DialogWidget->HideWidget();
		DialogWidget = nullptr;

		if (!LoadingScreenWidget.IsValid())
		{
			//ShowExistingWidgets();
			CurrentPlayersHUD->ShowMainMenu();
		}
	}
}

void UFusionGameViewportClient::ShowLoadingScreen()
{

	if (LoadingScreenWidget.IsValid())
	{
		return;
	}

	
	if (DialogWidget)
	{
		// Hide the dialog widget (loading screen takes priority)
		DialogWidget->HideWidget();
	}
	else
	{
		// Hide all existing widgets
		HideExistingWidgets();
	}

	LoadingScreenWidget = SNew(SFusionLoadingScreen);

	AddViewportWidgetContent(LoadingScreenWidget.ToSharedRef());
	
}

void UFusionGameViewportClient::HideLoadingScreen()
{

	if (!LoadingScreenWidget.IsValid())
	{
		return;
	}

	RemoveViewportWidgetContent(LoadingScreenWidget.ToSharedRef());

	LoadingScreenWidget = NULL;

	// Show the dialog widget if we need to
	if (DialogWidget)
	{
		DialogWidget->ShowWidget();
	}
	else
	{
		ShowExistingWidgets();
	}
}



void UFusionGameViewportClient::NotifyPlayerAdded(int32 PlayerIndex, ULocalPlayer* AddedPlayer)
{
	Super::NotifyPlayerAdded(PlayerIndex, AddedPlayer);

	UFusionLocalPlayer* const FusionLP = Cast<UFusionLocalPlayer>(AddedPlayer);
	if (FusionLP)
	{
		FusionLP->LoadPersistentUser();
	}
}

void UFusionGameViewportClient::AddViewportWidgetContent(TSharedRef<class SWidget> ViewportContent, const int32 ZOrder)
{
	UE_LOG(LogTemp, Warning, TEXT("UFusionGameViewportClient::AddViewportWidgetContent: %p"), &ViewportContent.Get());

	//if ((DialogWidget.IsValid() || LoadingScreenWidget.IsValid()) && ViewportContent != DialogWidget && ViewportContent != LoadingScreenWidget)
	if (LoadingScreenWidget.IsValid() && ViewportContent != LoadingScreenWidget)
	{
		// Add to hidden list, and don't show until we hide the dialog widget
		HiddenViewportContentStack.AddUnique(ViewportContent);
		return;
	}

	if (ViewportContentStack.Contains(ViewportContent))
	{
		return;
	}

	ViewportContentStack.AddUnique(ViewportContent);

	Super::AddViewportWidgetContent(ViewportContent, 0);
}

void UFusionGameViewportClient::RemoveViewportWidgetContent(TSharedRef<class SWidget> ViewportContent)
{
	UE_LOG(LogTemp, Warning, TEXT("UFusionGameViewportClient::RemoveViewportWidgetContent: %p"), &ViewportContent.Get());

	ViewportContentStack.Remove(ViewportContent);
	HiddenViewportContentStack.Remove(ViewportContent);

	Super::RemoveViewportWidgetContent(ViewportContent);
}

void UFusionGameViewportClient::HideExistingWidgets()
{
	check(HiddenViewportContentStack.Num() == 0);

	TArray<TSharedRef<class SWidget>> CopyOfViewportContentStack = ViewportContentStack;

	for (int32 i = ViewportContentStack.Num() - 1; i >= 0; i--)
	{
		RemoveViewportWidgetContent(ViewportContentStack[i]);
	}

	HiddenViewportContentStack = CopyOfViewportContentStack;
}

void UFusionGameViewportClient::ShowExistingWidgets()
{
	// We shouldn't have any visible widgets at this point
	check(ViewportContentStack.Num() == 0);

	// Unhide all of the previously hidden widgets
	for (int32 i = 0; i < HiddenViewportContentStack.Num(); i++)
	{
		AddViewportWidgetContent(HiddenViewportContentStack[i]);
	}

	check(ViewportContentStack.Num() == HiddenViewportContentStack.Num());

	// Done with these
	HiddenViewportContentStack.Empty();
}


EFusionDialogType::Type UFusionGameViewportClient::GetDialogType() const
{
	return (DialogWidget) ? DialogWidget->DialogType : EFusionDialogType::None;
}

TWeakObjectPtr<ULocalPlayer> UFusionGameViewportClient::GetDialogOwner() const
{
	return (DialogWidget) ? DialogWidget->PlayerOwner : nullptr;
}

void UFusionGameViewportClient::Tick(float DeltaSeconds)
{
	/*
	if (DialogWidget.IsValid() && !LoadingScreenWidget.IsValid())
	{
		// Make sure the dialog widget always has focus
		if (FSlateApplication::Get().GetKeyboardFocusedWidget() != DialogWidget)
		{
			// Remember which widget had focus before we override it
			OldFocusWidget = FSlateApplication::Get().GetKeyboardFocusedWidget();

			// Force focus back to dialog
			FSlateApplication::Get().SetKeyboardFocus(DialogWidget, EFocusCause::SetDirectly);
		}
	}*/

}

void SFusionLoadingScreen::Construct(const FArguments& InArgs)
{
	static const FName LoadingScreenName(TEXT("/Game/Tracked/Images/LoadingScreenTest.LoadingScreenTest"));

	//since we are not using game styles here, just load one image
	LoadingScreenBrush = MakeShareable(new FFusionGameLoadingScreenBrush(LoadingScreenName, FVector2D(1920, 1080)));


	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SImage)
			.Image(LoadingScreenBrush.Get())
		]
	+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SSafeZone)
			.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Right)
		.Padding(10.0f)
		.IsTitleSafe(true)
		[
			SNew(SThrobber)
			.Visibility(this, &SFusionLoadingScreen::GetLoadIndicatorVisibility)
		]
		]
		];
}



/*






#if WITH_EDITOR
void UFusionGameViewportClient::DrawTransition(UCanvas* Canvas)
{
	if (GetOuterUEngine() != NULL)
	{
		TEnumAsByte<enum ETransitionType> Type = GetOuterUEngine()->TransitionType;
		switch (Type)
		{
		case TT_Connecting:
			DrawTransitionMessage(Canvas, NSLOCTEXT("GameViewportClient", "ConnectingMessage", "CONNECTING").ToString());
			break;
		case TT_WaitingToConnect:
			DrawTransitionMessage(Canvas, NSLOCTEXT("GameViewportClient", "Waitingtoconnect", "Waiting to connect...").ToString());
			break;
		}
	}
}
#endif //WITH_EDITOR

*/