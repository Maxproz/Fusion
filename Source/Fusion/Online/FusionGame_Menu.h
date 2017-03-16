// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameModeBase.h"
#include "FusionGame_Menu.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API AFusionGame_Menu : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	AFusionGame_Menu(const FObjectInitializer& ObjectInitializer);

	// Begin AGameModeBase interface
	/** skip it, menu doesn't require player start or pawn */
	virtual void RestartPlayer(class AController* NewPlayer) override;

	/** Returns game session class to use */
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;
	// End AGameModeBase interface

protected:

	/** Perform some final tasks before hosting/joining a session. Remove menus, set king state etc */
	void BeginSession();

	/** Display a loading screen */
	void ShowLoadingScreen();

};
