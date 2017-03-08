// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"

#include "Player/TeamColors.h"

#include "FusionGameMode.generated.h"

UCLASS()
class AFusionGameMode : public AGameMode
{
	GENERATED_BODY()

protected:

	AFusionGameMode(const FObjectInitializer& ObjectInitializer);

	virtual void PreInitializeComponents() override;

	virtual void InitGameState();

	virtual void PostLogin(APlayerController* NewPlayer);

	virtual void DefaultTimer();

	virtual void StartMatch();

	//virtual void SpawnDefaultInventory(APawn* PlayerPawn);

	/**
	* Make sure pawn properties are back to default
	* Also a good place to modify them on spawn
	*/
	//virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

	/* Handle for efficient management of DefaultTimer timer */
	FTimerHandle TimerHandle_DefaultTimer;

	/* Can we deal damage to players in the same team */
	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	bool bAllowFriendlyFireDamage = true;

	/* Called once on every new player that enters the gamemode */
	virtual FString InitNewPlayer(class APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

	/* The teamcolors assigned to Players */
	ETeamColors RedTeam = ETeamColors::ETC_RED;

	ETeamColors BlueTeam = ETeamColors::ETC_BLUE;

	int32 NumberOfTeams = 2;

	UPROPERTY(VisibleInstanceOnly)
	int32 RedTeamPlayers = 0;

	UPROPERTY(VisibleInstanceOnly)
	int32 BlueTeamPlayers = 0;

	UPROPERTY(VisibleInstanceOnly)
	float MatchLength = 12.00f;

	ETeamColors AutoAssignTeamColor();

	/** best team */
	int32 WinnerTeam;

	/** pick team with least players in or random when it's equal */
	int32 ChooseTeam(class AFusionPlayerState* ForPlayerState) const;


	/** The bot pawn class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode)
	TSubclassOf<APawn> BotPawnClass;

	/************************************************************************/
	/* Player Spawning                                                      */
	/************************************************************************/

	/* Don't allow spectating of bots */
	virtual bool CanSpectate_Implementation(APlayerController* Viewer, APlayerState* ViewTarget) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/* Always pick a random location */
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

	virtual bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Controller);

	virtual bool IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Controller);

	/** returns default pawn class for given controller */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/************************************************************************/
	/* Damage & Killing                                                     */
	/************************************************************************/

public:

	virtual void Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType);

	/* Can the player deal damage according to gamemode rules (eg. friendly-fire disabled) */
	virtual bool CanDealDamage(class AFusionPlayerState* DamageCauser, class AFusionPlayerState* DamagedPlayer) const;

	virtual float ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	/** starts new match */
	virtual void HandleMatchHasStarted() override;

public:

	/* Primary sun of the level. Assigned in Blueprint during BeginPlay (BlueprintReadWrite is required as tag instead of EditDefaultsOnly) */
	UPROPERTY(BlueprintReadWrite, Category = "Level Brightness")
	ADirectionalLight* PrimarySunLight;

	/* The default weapons to spawn with */
	//UPROPERTY(EditDefaultsOnly, Category = "Player")
	//TArray<TSubclassOf<class ASWeapon>> DefaultInventoryClasses;

	/************************************************************************/
	/* Modding & Mutators                                                   */
	/************************************************************************/


protected:

	/* Mutators to create when game starts */
	UPROPERTY(EditAnywhere, Category = "Mutators")
	TArray<TSubclassOf<class AMutator>> MutatorClasses;

	/* First mutator in the execution chain */
	class AMutator* BaseMutator;

	void AddMutator(TSubclassOf<AMutator> MutClass);

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** From UT Source: Used to modify, remove, and replace Actors. Return false to destroy the passed in Actor. Default implementation queries mutators.
	* note that certain critical Actors such as PlayerControllers can't be destroyed, but we'll still call this code path to allow mutators
	* to change properties on them
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly)
	bool CheckRelevance(AActor* Other);

	/* Note: Functions flagged with BlueprintNativeEvent like above require _Implementation for a C++ implementation */
	virtual bool CheckRelevance_Implementation(AActor* Other);

	/* Hacked into ReceiveBeginPlay() so we can do mutator replacement of Actors and such */
	void BeginPlayMutatorHack(FFrame& Stack, RESULT_DECL);

};



