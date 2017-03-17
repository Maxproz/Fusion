// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"

#include "Player/TeamColors.h"

#include "FusionPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API AFusionPlayerState : public APlayerState
{
	GENERATED_BODY()
	

	AFusionPlayerState(const FObjectInitializer& ObjectInitializer);


	virtual void Reset() override;

	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const override;

	virtual void ClientInitialize(AController* InController) override;

	virtual void UnregisterPlayerWithSession() override;

	virtual void CopyProperties(class APlayerState* PlayerState) override; 

public:

	/** get current team */
	int32 GetTeamNum() const;

	void AddKill();

	void AddDeath();

	/** player killed someone */
	void ScoreKill(class AFusionPlayerState* Victim, int32 Points);

	/** player died */
	void ScoreDeath(class AFusionPlayerState* KilledBy, int32 Points);

	void SetTeamNum(int32 NewTeamNumber);

	void AddScore(int32 Amount);


	int32 GetKills() const;

	int32 GetDeaths() const;

	float GetScore() const;

	/** get number of bullets fired this match */
	int32 GetNumBulletsFired() const;

	/** get number of rockets fired this match */
	int32 GetNumRocketsFired() const;

	void UpdateTeamColors();
	
	/** Set whether the player is a quitter */
	void SetQuitter(bool bInQuitter);

	/** get whether the player quit the match */
	bool IsQuitter() const;

	UFUNCTION()
	void OnRep_TeamColor();

	/** gets truncated player name to fit in death log and scoreboards */
	FString GetShortPlayerName() const;

	//We don't need stats about amount of ammo fired to be server authenticated, so just increment these with local functions
	void AddBulletsFired(int32 NumBullets);
	void AddRocketsFired(int32 NumRockets);

	/** Sends kill (excluding self) to clients */
	UFUNCTION(Reliable, Client)
	void InformAboutKill(class AFusionPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AFusionPlayerState* KilledPlayerState);
	void InformAboutKill_Implementation(class AFusionPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AFusionPlayerState* KilledPlayerState);

	/** broadcast death to local clients */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastDeath(class AFusionPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AFusionPlayerState* KilledPlayerState);
	void BroadcastDeath_Implementation(class AFusionPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AFusionPlayerState* KilledPlayerState);

protected:
	/** team number */
	UPROPERTY(VisibleInstanceOnly, Transient, ReplicatedUsing = OnRep_TeamColor)
	int32 TeamNumber;
	
	UPROPERTY(VisibleInstanceOnly, Transient, Replicated)
	int32 NumKills;

	UPROPERTY(VisibleInstanceOnly, Transient, Replicated)
	int32 NumDeaths;

	/** number of Bullets fired this match */
	UPROPERTY()
	int32 NumBulletsFired;

	/** number of rockets fired this match */
	UPROPERTY()
	int32 NumRocketsFired;

	/** whether the user quit the match */
	UPROPERTY()
	bool bQuitter = false;

	/** helper for scoring points */
	void ScorePoints(int32 Points);

};
