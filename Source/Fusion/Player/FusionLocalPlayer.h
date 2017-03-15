// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/LocalPlayer.h"
#include "FusionLocalPlayer.generated.h"

/**
 * 
 */
UCLASS(config = Engine, transient)
class FUSION_API UFusionLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()


		/* Set a player name if no online system like Steam is available */
		//virtual FString GetNickname() const override;

	UFusionLocalPlayer(const FObjectInitializer& ObjectInitializer);

public:

	virtual void SetControllerId(int32 NewControllerId) override;

	virtual FString GetNickname() const override;

	class UFusionPersistentUser* GetPersistentUser() const;

	/** Initializes the PersistentUser */
	void LoadPersistentUser();

private:
	/** Persistent user data stored between sessions (i.e. the user's savegame) */
	UPROPERTY()
	class UFusionPersistentUser* PersistentUser;

};
