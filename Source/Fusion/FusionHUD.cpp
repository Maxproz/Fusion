// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Fusion.h"
#include "FusionHUD.h"

#include "Widgets/Gameplay/InGameHUD.h"
#include "Widgets/Menus/MainMenuUI.h"

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
	//check(ActiveMainMenuUIWidget == nullptr, TEXT("MainMenuUI Widget Already Initialized"));
	
	//if (ActiveMainMenuUIWidget != nullptr) return; // maybe later, return here if its already initalized

	ActiveMainMenuUIWidget = CreateWidget<UMainMenuUI>(GetOwningPlayerController(), MainMenuUIWidget.LoadSynchronous());
	//check(ActiveMainMenuUIWidget != nullptr, TEXT("Unable to Create MainMenuUI Widget Widget"));

	ActiveMainMenuUIWidget->AddToViewport(0);

	ActiveMainMenuUIWidget->SetVisibility(ESlateVisibility::Hidden);

	/*
	ActiveMainMenuUIWidget->SetPositionInViewport(FVector2D(0.f, 0.f));
	ActiveMainMenuUIWidget->SetDesiredSizeInViewport(FVector2D(0.f, 174.f));
	ActiveMainMenuUIWidget->SetAlignmentInViewport(FVector2D(0.f, 1.f));
	ActiveMainMenuUIWidget->SetAnchorsInViewport(FAnchors(0.f, 1.f, 1.f, 1.f));
	*/

	// Creates all Children etc.
	//ActiveMainMenuUIWidget->OnAddedToViewport();
}

void AFusionHUD::CreateInGameHUDWidget()
{
	//check(ActiveInGameHUDWidget == nullptr, TEXT("InGameHUD Widget Already Initialized"));

	ActiveInGameHUDWidget = CreateWidget<UInGameHUD>(GetOwningPlayerController(), InGameHUDWidget.LoadSynchronous());
	//check(ActiveInGameHUDWidget != nullptr, TEXT("Unable to Create InGameHUD Widget"));

	ActiveInGameHUDWidget->AddToViewport(1);

	//ActiveInfoWidget->SetPositionInViewport(FVector2D(0.f, 0.f));
	ActiveInGameHUDWidget->SetVisibility(ESlateVisibility::Hidden);

}

void AFusionHUD::CreateGameWidgets()
{
	CreateInGameHUDWidget();
	CreateMainMenuUIWidget();
}

void AFusionHUD::RemoveGameWidgets()
{
	
}


void AFusionHUD::DrawHUD()
{
	Super::DrawHUD();

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