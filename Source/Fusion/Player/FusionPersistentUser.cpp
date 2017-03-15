// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionLocalPlayer.h"

#include "FusionPersistentUser.h"




UFusionPersistentUser::UFusionPersistentUser(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetToDefaults();
}

void UFusionPersistentUser::SetToDefaults()
{
	bIsDirty = false;

	bInvertedYAxis = false;
	AimSensitivity = 1.0f;
	Gamma = 2.2f;
	BotsCount = 1;
	bIsRecordingDemos = false;
}

bool UFusionPersistentUser::IsAimSensitivityDirty() const
{
	bool bIsAimSensitivityDirty = false;

	// Fixme: UFusionPersistentUser is not setup to work with multiple worlds.
	// For now, user settings are global to all world instances.
	if (GEngine)
	{
		TArray<APlayerController*> PlayerList;
		GEngine->GetAllLocalPlayerControllers(PlayerList);

		for (auto It = PlayerList.CreateIterator(); It; ++It)
		{
			APlayerController* PC = *It;
			if (!PC || !PC->Player || !PC->PlayerInput)
			{
				continue;
			}

			// Update key bindings for the current user only
			UFusionLocalPlayer* LocalPlayer = Cast<UFusionLocalPlayer>(PC->Player);
			if (!LocalPlayer || LocalPlayer->GetPersistentUser() != this)
			{
				continue;
			}

			// check if the aim sensitivity is off anywhere
			for (int32 Idx = 0; Idx < PC->PlayerInput->AxisMappings.Num(); Idx++)
			{
				FInputAxisKeyMapping &AxisMapping = PC->PlayerInput->AxisMappings[Idx];
				if (AxisMapping.AxisName == "Lookup" || AxisMapping.AxisName == "LookupRate" || AxisMapping.AxisName == "Turn" || AxisMapping.AxisName == "TurnRate")
				{
					if (FMath::Abs(AxisMapping.Scale) != GetAimSensitivity())
					{
						bIsAimSensitivityDirty = true;
						break;
					}
				}
			}
		}
	}

	return bIsAimSensitivityDirty;
}

bool UFusionPersistentUser::IsInvertedYAxisDirty() const
{
	bool bIsInvertedYAxisDirty = false;
	if (GEngine)
	{
		TArray<APlayerController*> PlayerList;
		GEngine->GetAllLocalPlayerControllers(PlayerList);

		for (auto It = PlayerList.CreateIterator(); It; ++It)
		{
			APlayerController* PC = *It;
			if (!PC || !PC->Player || !PC->PlayerInput)
			{
				continue;
			}

			// Update key bindings for the current user only
			UFusionLocalPlayer* LocalPlayer = Cast<UFusionLocalPlayer>(PC->Player);
			if (!LocalPlayer || LocalPlayer->GetPersistentUser() != this)
			{
				continue;
			}

			bIsInvertedYAxisDirty |= PC->PlayerInput->GetInvertAxis("Lookup") != GetInvertedYAxis();
			bIsInvertedYAxisDirty |= PC->PlayerInput->GetInvertAxis("LookupRate") != GetInvertedYAxis();
		}
	}

	return bIsInvertedYAxisDirty;
}

void UFusionPersistentUser::SavePersistentUser()
{
	UGameplayStatics::SaveGameToSlot(this, SlotName, UserIndex);
	bIsDirty = false;
}

UFusionPersistentUser* UFusionPersistentUser::LoadPersistentUser(FString SlotName, const int32 UserIndex)
{
	UFusionPersistentUser* Result = nullptr;

	// first set of player signins can happen before the UWorld exists, which means no OSS, which means no user names, which means no slotnames.
	// Persistent users aren't valid in this state.
	if (SlotName.Len() > 0)
	{
		Result = Cast<UFusionPersistentUser>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
		if (Result == NULL)
		{
			// if failed to load, create a new one
			Result = Cast<UFusionPersistentUser>(UGameplayStatics::CreateSaveGameObject(UFusionPersistentUser::StaticClass()));
		}
		check(Result != NULL);

		Result->SlotName = SlotName;
		Result->UserIndex = UserIndex;
	}

	return Result;
}

void UFusionPersistentUser::SaveIfDirty()
{
	if (bIsDirty || IsInvertedYAxisDirty() || IsAimSensitivityDirty())
	{
		SavePersistentUser();
	}
}

void UFusionPersistentUser::AddMatchResult(int32 MatchKills, int32 MatchDeaths, int32 MatchBulletsFired, int32 MatchRocketsFired, bool bIsMatchWinner)
{
	Kills += MatchKills;
	Deaths += MatchDeaths;
	BulletsFired += MatchBulletsFired;
	RocketsFired += MatchRocketsFired;

	if (bIsMatchWinner)
	{
		Wins++;
	}
	else
	{
		Losses++;
	}

	bIsDirty = true;
}

void UFusionPersistentUser::TellInputAboutKeybindings()
{
	TArray<APlayerController*> PlayerList;
	GEngine->GetAllLocalPlayerControllers(PlayerList);

	for (auto It = PlayerList.CreateIterator(); It; ++It)
	{
		APlayerController* PC = *It;
		if (!PC || !PC->Player || !PC->PlayerInput)
		{
			continue;
		}

		// Update key bindings for the current user only
		UFusionLocalPlayer* LocalPlayer = Cast<UFusionLocalPlayer>(PC->Player);
		if (!LocalPlayer || LocalPlayer->GetPersistentUser() != this)
		{
			continue;
		}

		//set the aim sensitivity
		for (int32 Idx = 0; Idx < PC->PlayerInput->AxisMappings.Num(); Idx++)
		{
			FInputAxisKeyMapping &AxisMapping = PC->PlayerInput->AxisMappings[Idx];
			if (AxisMapping.AxisName == "Lookup" || AxisMapping.AxisName == "LookupRate" || AxisMapping.AxisName == "Turn" || AxisMapping.AxisName == "TurnRate")
			{
				AxisMapping.Scale = (AxisMapping.Scale < 0.0f) ? -GetAimSensitivity() : +GetAimSensitivity();
			}
		}
		PC->PlayerInput->ForceRebuildingKeyMaps();

		//invert it, and if does not equal our bool, invert it again
		if (PC->PlayerInput->GetInvertAxis("LookupRate") != GetInvertedYAxis())
		{
			PC->PlayerInput->InvertAxis("LookupRate");
		}

		if (PC->PlayerInput->GetInvertAxis("Lookup") != GetInvertedYAxis())
		{
			PC->PlayerInput->InvertAxis("Lookup");
		}
	}
}

int32 UFusionPersistentUser::GetUserIndex() const
{
	return UserIndex;
}

void UFusionPersistentUser::SetInvertedYAxis(bool bInvert)
{
	bIsDirty |= bInvertedYAxis != bInvert;

	bInvertedYAxis = bInvert;
}

void UFusionPersistentUser::SetAimSensitivity(float InSensitivity)
{
	bIsDirty |= AimSensitivity != InSensitivity;

	AimSensitivity = InSensitivity;
}

void UFusionPersistentUser::SetGamma(float InGamma)
{
	bIsDirty |= Gamma != InGamma;

	Gamma = InGamma;
}

void UFusionPersistentUser::SetBotsCount(int32 InCount)
{
	bIsDirty |= BotsCount != InCount;

	BotsCount = InCount;
}

void UFusionPersistentUser::SetIsRecordingDemos(const bool InbIsRecordingDemos)
{
	bIsDirty |= bIsRecordingDemos != InbIsRecordingDemos;

	bIsRecordingDemos = InbIsRecordingDemos;
}
