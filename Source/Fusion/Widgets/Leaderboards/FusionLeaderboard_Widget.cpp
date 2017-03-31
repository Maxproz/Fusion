// @Maxpro 2017

#include "Fusion.h"

#include "FusionPlayerController_Menu.h"

#include "FusionHUD.h"

#include "FusionLeaderboardRow_Widget.h"

#include "FusionLeaderboard_Widget.h"




FLeaderboardRow::FLeaderboardRow(const FOnlineStatsRow& Row)
	: Rank(FString::FromInt(Row.Rank))
	, PlayerName(Row.NickName)
	, PlayerId(Row.PlayerId)
{
	if (const FVariantData* KillData = Row.Columns.Find(LEADERBOARD_STAT_KILLS))
	{
		int32 Val;
		KillData->GetValue(Val);
		Kills = FString::FromInt(Val);
	}

	if (const FVariantData* DeathData = Row.Columns.Find(LEADERBOARD_STAT_DEATHS))
	{
		int32 Val;
		DeathData->GetValue(Val);
		Deaths = FString::FromInt(Val);
	}
}

void UFusionLeaderboard_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	MPC = Cast<AFusionPlayerController_Menu>(GetOwningPlayer());
	if (MPC)
	{
		PlayerHUD = MPC->GetFusionHUD();
	}

	PlayerOwner = MPC->GetLocalPlayer();

	bReadingStats = false;
	LeaderboardReadCompleteDelegate = FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &UFusionLeaderboard_Widget::OnStatsRead);

	CloseButton->OnClicked.AddDynamic(this, &UFusionLeaderboard_Widget::OnClickedCloseButton);

	/*
	OnlineSub = IOnlineSubsystem::Get();
	OnlineFriendsPtr = OnlineSub->GetFriendsInterface();
	if (OnlineFriendsPtr.IsValid())
	{
		OnlineFriendsPtr->ReadFriendsList(0, EFriendsLists::ToString(EFriendsLists::OnlinePlayers)); //init read of the friends list with the current user
	}
	*/

}


/** Starts reading leaderboards for the game */
void UFusionLeaderboard_Widget::ReadStats()
{
	StatRows.Reset();
	RowWidgets.Empty();
	LeaderboardScrollList->ClearChildren();


	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineLeaderboardsPtr Leaderboards = OnlineSub->GetLeaderboardsInterface();
		if (Leaderboards.IsValid())
		{
			// We are about to read the stats. The delegate will set this to false once the read is complete.
			LeaderboardReadCompleteDelegateHandle = Leaderboards->AddOnLeaderboardReadCompleteDelegate_Handle(LeaderboardReadCompleteDelegate);
			bReadingStats = true;

			// There's no reason to request leaderboard requests while one is in progress, so only do it if there isn't one active.
			if (!IsLeaderboardReadInProgress())
			{
			
				ReadObject = MakeShareable(new FFusionAllTimeMatchResultsRead());
				FOnlineLeaderboardReadRef ReadObjectRef = ReadObject.ToSharedRef();
				bReadingStats = Leaderboards->ReadLeaderboardsForFriends(0, ReadObjectRef);
			}
		}
		else
		{
			// TODO: message the user?
		}
	}
}

/** Called on a particular leaderboard read */
void UFusionLeaderboard_Widget::OnStatsRead(bool bWasSuccessful)
{
	// It's possible for another read request to happen while another one is ongoing (such as when the player leaves the menu and
	// re-enters quickly). We only want to process a leaderboard read if our read object is done.
	if (!IsLeaderboardReadInProgress())
	{
		ClearOnLeaderboardReadCompleteDelegate();

		if (bWasSuccessful)
		{
			for (int Idx = 0; Idx < ReadObject->Rows.Num(); ++Idx)
			{
				TSharedPtr<FLeaderboardRow> NewLeaderboardRow = MakeShareable(new FLeaderboardRow(ReadObject->Rows[Idx]));
				StatRows.Add(NewLeaderboardRow);
				MakeListViewWidget(NewLeaderboardRow, Idx);
			}

			//LeaderboardScrollList->List
			//RowListWidget->RequestListRefresh();
		}

		bReadingStats = false;
	}
}


void UFusionLeaderboard_Widget::OnClickedCloseButton()
{
	PlayerHUD->HideLeaderboards();
	PlayerHUD->ShowMainMenu();
}

void UFusionLeaderboard_Widget::MakeListViewWidget(TSharedPtr<FLeaderboardRow> Item, int32 RowIndex)
{
	UFusionLeaderboardRow_Widget* RowToAdd = CreateWidget<UFusionLeaderboardRow_Widget>(MPC, RowToAddTemplate);

	RowWidgets.Add(RowToAdd);

	RowToAdd->RankText->SetText(FText::FromString(Item->Rank));

	
	if (Item->PlayerName.Len() > MAX_PLAYER_NAME_LENGTH)
	{
		RowToAdd->PlayerNameText->SetText(FText::FromString(Item->PlayerName.Left(MAX_PLAYER_NAME_LENGTH) + "..."));
	}
	else
	{
		RowToAdd->PlayerNameText->SetText(FText::FromString(Item->PlayerName));
	}

	RowToAdd->KillsText->SetText(FText::FromString(Item->Kills));

	RowToAdd->DeathsText->SetText(FText::FromString(Item->Deaths));

	UUniformGridSlot* GridSlot = LeaderboardScrollList->AddChildToUniformGrid(RowToAdd);
	GridSlot->SetRow(RowIndex);


}

void UFusionLeaderboard_Widget::ClearOnLeaderboardReadCompleteDelegate()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineLeaderboardsPtr Leaderboards = OnlineSub->GetLeaderboardsInterface();
		if (Leaderboards.IsValid())
		{
			Leaderboards->ClearOnLeaderboardReadCompleteDelegate_Handle(LeaderboardReadCompleteDelegateHandle);
		}
	}
}

bool UFusionLeaderboard_Widget::IsLeaderboardReadInProgress()
{
	return ReadObject.IsValid() && (ReadObject->ReadState == EOnlineAsyncTaskState::InProgress || ReadObject->ReadState == EOnlineAsyncTaskState::NotStarted);
}
