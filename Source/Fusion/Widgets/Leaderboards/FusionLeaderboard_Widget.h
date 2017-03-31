// @Maxpro 2017

#pragma once

#include "Widgets/MasterWidget.h"

#include "Online/FusionLeaderboards.h"

#include "FusionLeaderboard_Widget.generated.h"


/** leaderboard row display information */
struct FLeaderboardRow
{
	/** player rank*/
	FString Rank;

	/** player name */
	FString PlayerName;

	/** player total kills to display */
	FString Kills;

	/** player total deaths to display */
	FString Deaths;

	/** Unique Id for the player at this rank */
	const TSharedPtr<const FUniqueNetId> PlayerId;

	/** Default Constructor */
	FLeaderboardRow(const FOnlineStatsRow& Row);
};


/**
 * 
 */
UCLASS()
class FUSION_API UFusionLeaderboard_Widget : public UMasterWidget
{
	GENERATED_BODY()
	
	
public:

	virtual void NativeConstruct() override;


protected:

	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* LeaderboardScrollList;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

public:

	UFUNCTION()
	void OnClickedCloseButton();


	/** creates single item widget, called for every list item */
	void MakeListViewWidget(TSharedPtr<FLeaderboardRow> Item, int32 RowIndex);

	/** Starts reading leaderboards for the game */
	void ReadStats();

	/** Called on a particular leaderboard read */
	void OnStatsRead(bool bWasSuccessful);

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UFusionLeaderboardRow_Widget> RowToAddTemplate;

	UPROPERTY(VisibleAnywhere)
	TArray<class UFusionLeaderboardRow_Widget*> RowWidgets;

	/** pointer to our owner PC */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

	class AFusionPlayerController_Menu* MPC;

	class AFusionHUD* PlayerHUD;

	TArray< TSharedRef<FOnlineFriend> > Friends;

	IOnlineSubsystem* OnlineSub;
	IOnlineFriendsPtr OnlineFriendsPtr;

protected:

	/** Removes the bound LeaderboardReadCompleteDelegate if possible*/
	void ClearOnLeaderboardReadCompleteDelegate();

	/** Returns true if a leaderboard read request is in progress or scheduled */
	bool IsLeaderboardReadInProgress();

	/** action bindings array */
	TArray< TSharedPtr<FLeaderboardRow> > StatRows;

	/** Leaderboard read object */
	FOnlineLeaderboardReadPtr ReadObject;

	/** Indicates that a stats read operation has been initiated */
	bool bReadingStats;

	/** Delegate called when a leaderboard has been successfully read */
	FOnLeaderboardReadCompleteDelegate LeaderboardReadCompleteDelegate;


	/** currently selected list item */
	TSharedPtr<FLeaderboardRow> SelectedItem;


	/** Handle to the registered LeaderboardReadComplete delegate */
	FDelegateHandle LeaderboardReadCompleteDelegateHandle;
};


