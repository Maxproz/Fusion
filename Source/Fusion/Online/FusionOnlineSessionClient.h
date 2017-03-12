// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "OnlineSessionClient.h"
#include "FusionOnlineSessionClient.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UFusionOnlineSessionClient : public UOnlineSessionClient
{
	GENERATED_BODY()
	
public:
	/** Ctor */
	UFusionOnlineSessionClient();

	virtual void OnSessionUserInviteAccepted(
		const bool							bWasSuccess,
		const int32							ControllerId,
		TSharedPtr< const FUniqueNetId >	UserId,
		const FOnlineSessionSearchResult &	InviteResult
	) override;

	virtual void OnPlayTogetherEventReceived(int32 UserIndex, TArray<TSharedPtr<const FUniqueNetId>> UserIdList) override;

};
