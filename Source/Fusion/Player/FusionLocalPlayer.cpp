// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameInstance.h"
#include "OnlineSubsystemUtilsClasses.h"

#include "FusionPersistentUser.h"

#include "FusionLocalPlayer.h"

/*
FString UFusionLocalPlayer::GetNickname() const
{
	// Try to fetch a nickname from the online subsystem (eg. Steam) if available 
	FString NickName = Super::GetNickname();

	// Fall back if no nickname was available through the online subsystem.
	if (NickName.IsEmpty())
	{
		const FString Suffix = FString::FromInt(FMath::RandRange(0, 999));
		NickName = FPlatformProcess::ComputerName() + Suffix;
	}

	return NickName;
}*/

UFusionLocalPlayer::UFusionLocalPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UFusionPersistentUser* UFusionLocalPlayer::GetPersistentUser() const
{
	// if persistent data isn't loaded yet, load it
	if (PersistentUser == nullptr)
	{
		UFusionLocalPlayer* const MutableThis = const_cast<UFusionLocalPlayer*>(this);
		// casting away constness to enable caching implementation behavior
		MutableThis->LoadPersistentUser();
	}
	return PersistentUser;
}

void UFusionLocalPlayer::LoadPersistentUser()
{
	FString SaveGameName = GetNickname();

#if PLATFORM_WOLF
	// on Wolf, the displayable nickname can change, so we can't use it as a save ID (explicitly stated in docs, so changing for pre-cert)
	FPlatformMisc::GetUniqueStringNameForControllerId(GetControllerId(), SaveGameName);
#endif

	// if we changed controllerid / user, then we need to load the appropriate persistent user.
	if (PersistentUser != nullptr && (GetControllerId() != PersistentUser->GetUserIndex() || SaveGameName != PersistentUser->GetName()))
	{
		PersistentUser->SaveIfDirty();
		PersistentUser = nullptr;
	}

	if (PersistentUser == NULL)
	{
		// Use the platform id here to be resilient in the face of controller swapping and similar situations.
		FPlatformUserId PlatformId = GetControllerId();

		auto Identity = Online::GetIdentityInterface();
		if (Identity.IsValid() && GetPreferredUniqueNetId().IsValid())
		{
			PlatformId = Identity->GetPlatformUserIdFromUniqueNetId(*GetPreferredUniqueNetId());
		}

		PersistentUser = UFusionPersistentUser::LoadPersistentUser(SaveGameName, PlatformId);
	}
}

void UFusionLocalPlayer::SetControllerId(int32 NewControllerId)
{
	FString SaveGameName = GetNickname();

#if PLATFORM_WOLF
	// on Wolf, the displayable nickname can change, so we can't use it as a save ID (explicitly stated in docs, so changing for pre-cert)
	FPlatformMisc::GetUniqueStringNameForControllerId(GetControllerId(), SaveGameName);
#endif

	ULocalPlayer::SetControllerId(NewControllerId);

	// if we changed controllerid / user, then we need to load the appropriate persistent user.
	if (PersistentUser != nullptr && (GetControllerId() != PersistentUser->GetUserIndex() || SaveGameName != PersistentUser->GetName()))
	{
		PersistentUser->SaveIfDirty();
		PersistentUser = nullptr;
	}

	if (!PersistentUser)
	{
		LoadPersistentUser();
	}
}

FString UFusionLocalPlayer::GetNickname() const
{
	FString UserNickName = Super::GetNickname();

	if (UserNickName.Len() > MAX_PLAYER_NAME_LENGTH)
	{
		UserNickName = UserNickName.Left(MAX_PLAYER_NAME_LENGTH) + "...";
	}

	bool bReplace = (UserNickName.Len() == 0);

	// Check for duplicate nicknames...and prevent reentry
	static bool bReentry = false;
	if (!bReentry)
	{
		bReentry = true;
		UFusionGameInstance* GameInstance = GetWorld() != NULL ? Cast<UFusionGameInstance>(GetWorld()->GetGameInstance()) : NULL;
		if (GameInstance)
		{
			// Check all the names that occur before ours that are the same
			const TArray<ULocalPlayer*>& LocalPlayers = GameInstance->GetLocalPlayers();
			for (int i = 0; i < LocalPlayers.Num(); ++i)
			{
				const ULocalPlayer* LocalPlayer = LocalPlayers[i];
				if (this == LocalPlayer)
				{
					break;
				}

				if (UserNickName == LocalPlayer->GetNickname())
				{
					bReplace = true;
					break;
				}
			}
		}
		bReentry = false;
	}

	if (bReplace)
	{
		UserNickName = FString::Printf(TEXT("Player%i"), GetControllerId() + 1);
	}

	return UserNickName;
}
