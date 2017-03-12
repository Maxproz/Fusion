// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
//#include "FusionOnlineGameSettings.generated.h"


 /**
 * General session settings for a Shooter game
 */
class FFusionOnlineSessionSettings : public FOnlineSessionSettings
{
public:

	FFusionOnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4);
	virtual ~FFusionOnlineSessionSettings() {}
};

/**
* General search setting for a Shooter game
*/
class FFusionOnlineSearchSettings : public FOnlineSessionSearch
{
public:
	FFusionOnlineSearchSettings(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FFusionOnlineSearchSettings() {}
};

/**
* Search settings for an empty dedicated server to host a match
*/
class FFusionOnlineSearchSettingsEmptyDedicated : public FFusionOnlineSearchSettings
{
public:
	FFusionOnlineSearchSettingsEmptyDedicated(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FFusionOnlineSearchSettingsEmptyDedicated() {}
};
