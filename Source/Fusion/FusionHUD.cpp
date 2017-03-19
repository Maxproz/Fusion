// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Fusion.h"
#include "FusionHUD.h"

#include "Widgets/Gameplay/InGameHUD.h"
#include "Widgets/Menus/MainMenuUI.h"
#include "Widgets/Menus/OkErrorMessage_Widget.h"
#include "Widgets/Menus/ServerMenu_Widget.h"
#include "Widgets/Menus/Lobby/LobbyMenu_Widget.h"

#include "FusionPlayerController_Menu.h"

#include "Widgets/Menus/Lobby/ChatEntry_Widget.h"
#include "Widgets/Menus/Lobby/PlayerInfoEntry_Widget.h"
#include "Widgets/Menus/Lobby/InviteSteamFriend_Widget.h"
#include "Widgets/Menus/PasswordEnterPopup_Widget.h"
#include "Widgets/Menus/ServerMenuStats_Widget.h"


#include "Engine/Canvas.h"
#include "TextureResource.h"

#include "CanvasItem.h"

AFusionHUD::AFusionHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshiarTexObj(TEXT("/Game/Tracked/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshiarTexObj.Object;
}

void AFusionHUD::CreateMainMenuUIWidget()
{
	if (ActiveMainMenuUIWidget != nullptr) return;
	ActiveMainMenuUIWidget = CreateWidget<UMainMenuUI>(GetOwningPlayerController(), MainMenuUIWidget.LoadSynchronous());

	ActiveMainMenuUIWidget->AddToViewport(0);
	ActiveMainMenuUIWidget->SetVisibility(ESlateVisibility::Hidden);
}

void AFusionHUD::CreateInGameHUDWidget()
{
	if (ActiveInGameHUDWidget != nullptr) return;
	ActiveInGameHUDWidget = CreateWidget<UInGameHUD>(GetOwningPlayerController(), InGameHUDWidget.LoadSynchronous());
	ActiveInGameHUDWidget->AddToViewport(0);
	ActiveInGameHUDWidget->SetVisibility(ESlateVisibility::Hidden);
}

void AFusionHUD::CreateServerMenuWidget()
{
	if (ActiveServerMenuWidget != nullptr) return;
	ActiveServerMenuWidget = CreateWidget<UServerMenu_Widget>(GetOwningPlayerController(), ServerMenuWidget.LoadSynchronous());
	ActiveServerMenuWidget->AddToViewport(0);
	ActiveServerMenuWidget->SetVisibility(ESlateVisibility::Hidden);
}

void AFusionHUD::CreateErrorMessageWidget()
{
	if (ActiveErrorMessageWidget != nullptr) return;
	ActiveErrorMessageWidget = CreateWidget<UOkErrorMessage_Widget>(GetOwningPlayerController(), ErrorMessageWidget.LoadSynchronous());
	ActiveErrorMessageWidget->AddToViewport(0);
	ActiveErrorMessageWidget->SetVisibility(ESlateVisibility::Hidden);
}

void AFusionHUD::CreateLobbyMenuWidget()
{
	if (ActiveLobbyMenuWidget != nullptr) return;
	ActiveLobbyMenuWidget = CreateWidget<ULobbyMenu_Widget>(GetOwningPlayerController(), LobbyMenuWidget.LoadSynchronous());
	ActiveLobbyMenuWidget->AddToViewport(0);
	ActiveLobbyMenuWidget->SetVisibility(ESlateVisibility::Hidden);
}

void AFusionHUD::CreateGameWidgets()
{
	CreateInGameHUDWidget();
	CreateMainMenuUIWidget();
	CreateServerMenuWidget();
	CreateErrorMessageWidget();
	CreateLobbyMenuWidget();
}


void AFusionHUD::RemoveGameWidgets()
{
	
}


void AFusionHUD::ShowMainMenu() { GetMainMenuUIWidget()->ShowWidget(); }
void AFusionHUD::HideMainMenu() { GetMainMenuUIWidget()->HideWidget(); }

void AFusionHUD::ShowServerMenu() { GetServerMenuWidget()->ShowWidget(); }
void AFusionHUD::HideServerMenu() { GetServerMenuWidget()->HideWidget(); }

void AFusionHUD::ShowLobbyMenu() { GetLobbyMenuWidget()->ShowWidget(); }
void AFusionHUD::HideLobbyMenu() { GetLobbyMenuWidget()->HideWidget(); }


void AFusionHUD::DrawHUD()
{
	Super::DrawHUD();

	AFusionPlayerController_Menu* PC = Cast<AFusionPlayerController_Menu>(GetOwningPlayerController());
	if (PC) return; // Don't draw crosshair inside of Main Menu

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X),
										   (Center.Y + 20.0f));

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );
}

void AFusionHUD::OnStateChanged_Implementation(EHUDState NewState)
{
	CurrentState = NewState;
}


EHUDState AFusionHUD::GetCurrentState()
{
	return CurrentState;
}

void AFusionHUD::SetMatchState(EHUDState NewState)
{
	CurrentState = NewState;
}