// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#ifndef __FUSIONGAMELOADINGSCREEN_H__
#define __FUSIONGAMELOADINGSCREEN_H__

#include "ModuleInterface.h"


/** Module interface for this game's loading screens */
class IFusionGameLoadingScreenModule : public IModuleInterface
{
public:
	/** Kicks off the loading screen for in game loading (not startup) */
	virtual void StartInGameLoadingScreen() = 0;
};

#endif // __FUSIONGAMELOADINGSCREEN_H__
