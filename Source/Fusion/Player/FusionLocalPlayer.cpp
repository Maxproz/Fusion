// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"
#include "FusionLocalPlayer.h"


FString UFusionLocalPlayer::GetNickname() const
{
	/* Try to fetch a nickname from the online subsystem (eg. Steam) if available */
	FString NickName = Super::GetNickname();

	// Fall back if no nickname was available through the online subsystem.
	if (NickName.IsEmpty())
	{
		const FString Suffix = FString::FromInt(FMath::RandRange(0, 999));
		NickName = FPlatformProcess::ComputerName() + Suffix;
	}

	return NickName;
}