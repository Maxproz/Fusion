// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionHUD.h"

#include "Widgets/Menus/MainMenuUI.h"

#include "FusionPlayerController_Menu.h"

AFusionPlayerController_Menu::AFusionPlayerController_Menu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// TODO: Maybe needs moved
	
}

void AFusionPlayerController_Menu::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		GetFusionHUD()->CreateGameWidgets();
		ClientShowMainMenu();

		bShowMouseCursor = true;
		ClientIgnoreLookInput(true);
		ClientIgnoreMoveInput(true);
	}
}

void AFusionPlayerController_Menu::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//FShooterStyle::Initialize();
}

AFusionHUD* AFusionPlayerController_Menu::GetFusionHUD() const
{
	return Cast<AFusionHUD>(GetHUD());
}

// This probably doesn't need to be "client" function since widgets only exist on the owning client anyway.
void AFusionPlayerController_Menu::ClientShowMainMenu_Implementation()
{
	GetFusionHUD()->ShowMainMenu();
	//Widget->SetVisibility(ESlateVisibility::Visible);
}
