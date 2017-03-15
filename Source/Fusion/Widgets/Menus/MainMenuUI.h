// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/MasterWidget.h"
#include "MainMenuUI.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UMainMenuUI : public UMasterWidget
{
	GENERATED_BODY()
	
	
public:
	/** Add the menu to the gameviewport so it becomes visible */
	//void AddMenuToGameViewport();
	
	virtual void NativeConstruct() override;

	void ShowMainMenu();

	void HideMainMenu();

	void DisplayLoadingScreen();


	/* function events bound to our button presses */
	UFUNCTION()
	void OnClickedHostButton();

	UFUNCTION()
	void OnClickedJoinButton();

	UFUNCTION()
	void OnClickedQuitButton();


	/** Returns the player that owns the main menu. */
	//ULocalPlayer* GetPlayerOwner() const;


protected:
	
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;

	/** Delegate executed when matchmaking completes */
	FOnMatchmakingCompleteDelegate OnMatchmakingCompleteDelegate;

	void OnMatchmakingComplete(FName SessionName, bool bWasSuccessful);

	void HostGame(const FString& GameType);

	void HostFreeForAll();

	void BeginQuickMatchSearch();

	void Quit();

	FDelegateHandle OnMatchmakingCompleteDelegateHandle;

	/** Settings and storage for quickmatch searching */
	TSharedPtr<FOnlineSessionSearch> QuickMatchSearchSettings;


	/* TODO: Implement these.. they seem needed
	// Start the check for whether the owner of the menu has online privileges 
	void StartOnlinePrivilegeTask(const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate& Delegate);

	// Common cleanup code for any Privilege task delegate 
	void CleanupOnlinePrivilegeTask();

	// Delegate function executed after checking privileges for hosting an online game 
	void OnUserCanPlayOnlineHost(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	// Delegate function executed after checking privileges for joining an online game 
	void OnUserCanPlayOnlineJoin(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	// Delegate function executed after checking privileges for starting quick match 
	void OnUserCanPlayOnlineQuickMatch(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);
	*/
};
