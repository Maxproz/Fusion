// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameInstance.h"

#include "FusionOnlineSessionClient.h"



UFusionOnlineSessionClient::UFusionOnlineSessionClient()
{
}

void UFusionOnlineSessionClient::OnSessionUserInviteAccepted(
	const bool							bWasSuccess,
	const int32							ControllerId,
	TSharedPtr< const FUniqueNetId > 	UserId,
	const FOnlineSessionSearchResult &	InviteResult
)
{
	UE_LOG(LogOnline, Verbose, TEXT("HandleSessionUserInviteAccepted: bSuccess: %d, ControllerId: %d, User: %s"), bWasSuccess, ControllerId, UserId.IsValid() ? *UserId->ToString() : TEXT("NULL"));

	if (!bWasSuccess)
	{
		return;
	}

	if (!InviteResult.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("Invite accept returned no search result."));
		return;
	}

	if (!UserId.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("Invite accept returned no user."));
		return;
	}

	UFusionGameInstance* FusionGameInstance = Cast<UFusionGameInstance>(GetGameInstance());

	if (FusionGameInstance)
	{
		FFusionPendingInvite PendingInvite;

		// Set the pending invite, and then go to the initial screen, which is where we will process it
		PendingInvite.ControllerId = ControllerId;
		PendingInvite.UserId = UserId;
		PendingInvite.InviteResult = InviteResult;
		PendingInvite.bPrivilegesCheckedAndAllowed = false;

		FusionGameInstance->SetPendingInvite(PendingInvite);
		FusionGameInstance->GotoState(FusionGameInstanceState::PendingInvite);
	}
}

void UFusionOnlineSessionClient::OnPlayTogetherEventReceived(int32 UserIndex, TArray<TSharedPtr<const FUniqueNetId>> UserIdList)
{
	if (UFusionGameInstance* const FusionGameInstance = Cast<UFusionGameInstance>(GetGameInstance()))
	{
		FusionGameInstance->OnPlayTogetherEventReceived(UserIndex, UserIdList);
	}
}
