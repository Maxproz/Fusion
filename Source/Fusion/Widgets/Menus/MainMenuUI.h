// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/MasterWidget.h"
#include "MainMenuUI.generated.h"


struct FServerEntry
{
	FString ServerName;
	FString CurrentPlayers;
	FString MaxPlayers;
	FString GameType;
	FString MapName;
	FString Ping;
	int32 SearchResultsIndex;
};


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





	void UpdateSearchStatus();

	AFusionGameSession* GetGameSession() const;

	/** Starts searching for servers */
	void BeginServerSearch(bool bLANMatch, const FString& InMapFilterName);

	/** Called when server search is finished */
	void OnServerSearchFinished();

	/** fill/update server list, should be called before showing this control */
	void UpdateServerList();

	/** connect to chosen server */
	void ConnectToServer();


	virtual void NativeTick(const FGeometry & MyGeometry,float InDeltaTime) override;

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

	void HostTeamDeathMatch();

	void BeginQuickMatchSearch();

	void Quit();

	FDelegateHandle OnMatchmakingCompleteDelegateHandle;

	/** Settings and storage for quickmatch searching */
	TSharedPtr<FOnlineSessionSearch> QuickMatchSearchSettings;

	/** pointer to our owner PC */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;



	/** Whether last searched for LAN (so spacebar works) */
	bool bLANMatchSearch;

	/** Whether we're searching for servers */
	bool bSearchingForServers;

	/** action bindings array */
	TArray< TSharedPtr<FServerEntry> > ServerList;

	/** Map filter name to use during server searches */
	FString MapFilterName;

	/** currently selected list item */
	TSharedPtr<FServerEntry> SelectedItem;

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
