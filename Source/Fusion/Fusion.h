// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#ifndef __FUSION_H__
#define __FUSION_H__

//#include "EngineMinimal.h"
#include "Engine.h"
#include "UnrealNetwork.h"
#include "Online.h"


#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"


/** when you modify this, please note that this information can be saved with instances
* also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list **/
#define COLLISION_WEAPON		ECC_GameTraceChannel1
#define COLLISION_PROJECTILE	ECC_GameTraceChannel2
#define COLLISION_PICKUP		ECC_GameTraceChannel3

#define MAX_PLAYER_NAME_LENGTH 16

/** when you modify this, please note that this information can be saved with instances
* also DefaultEngine.ini [/Script/Engine.PhysicsSettings] should match with this list **/
#define SURFACE_DEFAULT				SurfaceType_Default
#define SURFACE_FLESH				SurfaceType1
#define SURFACE_BODY				SurfaceType2
#define SURFACE_HEAD				SurfaceType3
#define SURFACE_ARM					SurfaceType4
#define SURFACE_LEG					SurfaceType5


/** Set to 1 to pretend we're building for console even on a PC, for testing purposes */
#define FUSION_SIMULATE_CONSOLE_UI	0

#if PLATFORM_PS4 || PLATFORM_XBOXONE || PLATFORM_WOLF || FUSION_SIMULATE_CONSOLE_UI
#define FUSION_CONSOLE_UI 1
#else
#define FUSION_CONSOLE_UI 0
#endif


#endif
