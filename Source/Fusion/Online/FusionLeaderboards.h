// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"

#include "OnlineLeaderboardInterface.h"

//#include "FusionLeaderboards.generated.h"


 // these are normally exported from platform-specific tools
#define LEADERBOARD_STAT_SCORE				"Score"
#define LEADERBOARD_STAT_KILLS				"Frags"
#define LEADERBOARD_STAT_DEATHS				"Deaths"
#define LEADERBOARD_STAT_MATCHESPLAYED		"MatchesPlayed"

 /**
 *	'AllTime' leaderboard read object
 */
class FFusionAllTimeMatchResultsRead : public FOnlineLeaderboardRead
{
public:

	FFusionAllTimeMatchResultsRead()
	{
		// Default properties
		LeaderboardName = FName(TEXT("FusionAllTimeMatchResults"));
		SortedColumn = LEADERBOARD_STAT_SCORE;

		// Define default columns
		new (ColumnMetadata) FColumnMetaData(LEADERBOARD_STAT_SCORE, EOnlineKeyValuePairDataType::Int32);
		new (ColumnMetadata) FColumnMetaData(LEADERBOARD_STAT_KILLS, EOnlineKeyValuePairDataType::Int32);
		new (ColumnMetadata) FColumnMetaData(LEADERBOARD_STAT_DEATHS, EOnlineKeyValuePairDataType::Int32);
		new (ColumnMetadata) FColumnMetaData(LEADERBOARD_STAT_MATCHESPLAYED, EOnlineKeyValuePairDataType::Int32);
	}
};

/**
*	'AllTime' leaderboard write object
*/
class FFusionAllTimeMatchResultsWrite : public FOnlineLeaderboardWrite
{
public:

	FFusionAllTimeMatchResultsWrite()
	{
		// Default properties
		new (LeaderboardNames) FName(TEXT("FusionAllTimeMatchResults"));
		RatedStat = LEADERBOARD_STAT_SCORE;
		DisplayFormat = ELeaderboardFormat::Number;
		SortMethod = ELeaderboardSort::Descending;
		UpdateMethod = ELeaderboardUpdateMethod::KeepBest;
	}
};

