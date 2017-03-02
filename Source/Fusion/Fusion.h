// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#ifndef __FUSION_H__
#define __FUSION_H__

//#include "EngineMinimal.h"
#include "Engine.h"
#include "UnrealNetwork.h"
#include "Online.h"


/** when you modify this, please note that this information can be saved with instances
* also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list **/
#define COLLISION_WEAPON				ECC_GameTraceChannel1


/** when you modify this, please note that this information can be saved with instances
* also DefaultEngine.ini [/Script/Engine.PhysicsSettings] should match with this list **/
#define SURFACE_DEFAULT				SurfaceType_Default
#define SURFACE_FLESH				SurfaceType1
#define SURFACE_BODY				SurfaceType2
#define SURFACE_HEAD				SurfaceType3
#define SURFACE_ARM					SurfaceType4
#define SURFACE_LEG					SurfaceType5


#endif
