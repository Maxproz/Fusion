// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Fusion.h"

#include "FusionHUD.h"
#include "FusionGameState.h"
#include "FusionCharacter.h"
#include "FusionBaseCharacter.h"
#include "FusionPlayerController.h"
#include "FusionPlayerState.h"

#include "Player/FusionPlayerStart.h"
#include "Mutators/Mutator.h"

#include "FusionGameMode.h"

//#include "SCharacter.h"
//#include "STypes.h"
//#include "SSpectatorPawn.h"


AFusionGameMode::AFusionGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Tracked/Characters/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	HUDClass = AFusionHUD::StaticClass();

	/* Assign the class types used by this gamemode */
	PlayerControllerClass = AFusionPlayerController::StaticClass();
	PlayerStateClass = AFusionPlayerState::StaticClass();
	GameStateClass = AFusionGameState::StaticClass();
	//SpectatorClass = ASSpectatorPawn::StaticClass();

}


void AFusionGameMode::InitGameState()
{
	Super::InitGameState();

	AFusionGameState* MyGameState = Cast<AFusionGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->ElapsedGameMinutes = MatchLength;
	}
}


void AFusionGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	/* Set timer to run every second */
	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &AFusionGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}


void AFusionGameMode::StartMatch()
{
	Super::StartMatch();

}


void AFusionGameMode::DefaultTimer()
{
	/* Immediately start the match while playing in editor */
	//if (GetWorld()->IsPlayInEditor())
	{
		if (GetMatchState() == MatchState::WaitingToStart)
		{
			StartMatch();
		}
	}

	/* Only increment time of day while game is active */
	if (IsMatchInProgress())
	{
		AFusionGameState* MyGameState = Cast<AFusionGameState>(GameState);
		if (MyGameState)
		{
			/* decerase out match timer */
			// TODO: This timer is going to be off at first, but it can be adjusted easily.
			MyGameState->ElapsedGameMinutes -= 0.01f;
		}
	}
}


bool AFusionGameMode::CanDealDamage(class AFusionPlayerState* DamageCauser, class AFusionPlayerState* DamagedPlayer) const
{
	if (bAllowFriendlyFireDamage)
	{
		return true;
	}

	/* Allow damage to self */
	if (DamagedPlayer == DamageCauser)
	{
		return true;
	}

	// Compare Team Colors
	return DamageCauser && DamagedPlayer && (DamageCauser->GetTeamColor() != DamagedPlayer->GetTeamColor());
}


FString AFusionGameMode::InitNewPlayer(class APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString Result = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	AFusionPlayerState* NewPlayerState = Cast<AFusionPlayerState>(NewPlayerController->PlayerState);
	if (NewPlayerState)
	{
		NewPlayerState->SetTeamColor(AutoAssignTeamColor());
	}

	return Result;
}

ETeamColors AFusionGameMode::AutoAssignTeamColor()
{
	// TODO: Figure out how I am going to do the rules for max players and associate it with this functionality below.

	if (RedTeamPlayers == BlueTeamPlayers)
	{
		RedTeamPlayers++;
		return ETeamColors::ETC_RED;
	}
	else
	{
		if (RedTeamPlayers > BlueTeamPlayers)
		{
			BlueTeamPlayers++;
			return ETeamColors::ETC_BLUE;
		}
		else
		{
			RedTeamPlayers++;
			return ETeamColors::ETC_RED;
		}
	}
}

float AFusionGameMode::ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float ActualDamage = Damage;

	AFusionBaseCharacter* DamagedPawn = Cast<AFusionBaseCharacter>(DamagedActor);
	if (DamagedPawn && EventInstigator)
	{
		AFusionPlayerState* DamagedPlayerState = Cast<AFusionPlayerState>(DamagedPawn->PlayerState);
		AFusionPlayerState* InstigatorPlayerState = Cast<AFusionPlayerState>(EventInstigator->PlayerState);

		// Check for friendly fire
		if (!CanDealDamage(InstigatorPlayerState, DamagedPlayerState))
		{
			ActualDamage = 0.f;
		}
	}

	return ActualDamage;
}


bool AFusionGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	/* Always pick a random location */
	return false;
}


AActor* AFusionGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;
	TArray<APlayerStart*> FallbackSpawns;

	/* Get all playerstart objects in level */
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

	/* Split the player starts into two arrays for preferred and fallback spawns */
	for (int32 i = 0; i < PlayerStarts.Num(); i++)
	{
		APlayerStart* TestStart = Cast<APlayerStart>(PlayerStarts[i]);

		if (TestStart && IsSpawnpointAllowed(TestStart, Player))
		{
			if (IsSpawnpointPreferred(TestStart, Player))
			{
				PreferredSpawns.Add(TestStart);
			}
			else
			{
				FallbackSpawns.Add(TestStart);
			}
		}

	}

	/* Pick a random spawnpoint from the filtered spawn points */
	APlayerStart* BestStart = nullptr;
	if (PreferredSpawns.Num() > 0)
	{
		BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
	}
	else if (FallbackSpawns.Num() > 0)
	{
		BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
	}

	/* If we failed to find any (so BestStart is nullptr) fall back to the base code */
	return BestStart ? BestStart : Super::ChoosePlayerStart_Implementation(Player);
}


bool AFusionGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Controller)
{
	if (Controller == nullptr || Controller->PlayerState == nullptr)
		return true;

	/* Check for extended playerstart class */
	AFusionPlayerStart* MyPlayerStart = Cast<AFusionPlayerStart>(SpawnPoint);
	if (MyPlayerStart)
	{
		return MyPlayerStart->GetIsPlayerOnly() && !Controller->PlayerState->bIsABot;
	}

	/* Cast failed, Anyone can spawn at the base playerstart class */
	return true;
}


bool AFusionGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Controller)
{
	if (SpawnPoint)
	{
		/* Iterate all pawns to check for collision overlaps with the spawn point */
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
		{
			ACharacter* OtherPawn = Cast<ACharacter>(*It);
			if (OtherPawn)
			{
				const float CombinedHeight = (SpawnPoint->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * 2.0f;
				const float CombinedWidth = SpawnPoint->GetCapsuleComponent()->GetScaledCapsuleRadius() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
				const FVector OtherLocation = OtherPawn->GetActorLocation();

				// Check if player overlaps the playerstart
				if (FMath::Abs(SpawnLocation.Z - OtherLocation.Z) < CombinedHeight && (SpawnLocation - OtherLocation).Size2D() < CombinedWidth)
				{
					return false;
				}
			}
		}

		/* Check if spawnpoint is exclusive to players */
		AFusionPlayerStart* MyPlayerStart = Cast<AFusionPlayerStart>(SpawnPoint);
		if (MyPlayerStart)
		{
			return MyPlayerStart->GetIsPlayerOnly() && !Controller->PlayerState->bIsABot;
		}
	}

	return false;
}


/*
// Used by RestartPlayer() to determine the pawn to create and possess when a bot or player spawns 
UClass* AFusionGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (Cast<ASZombieAIController>(InController))
	{
		return BotPawnClass;
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}
*/

bool AFusionGameMode::CanSpectate_Implementation(APlayerController* Viewer, APlayerState* ViewTarget)
{
	/* Don't allow spectating of other non-player bots */
	return (ViewTarget && !ViewTarget->bIsABot);
}



void AFusionGameMode::Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType)
{
	// Do nothing (can we used to apply score or keep track of kill count)
}


/*
void ASGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	SpawnDefaultInventory(PlayerPawn);
}
*/


/*
void ASGameMode::SpawnDefaultInventory(APawn* PlayerPawn)
{
	ASCharacter* MyPawn = Cast<ASCharacter>(PlayerPawn);
	if (MyPawn)
	{
		for (int32 i = 0; i < DefaultInventoryClasses.Num(); i++)
		{
			if (DefaultInventoryClasses[i])
			{
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				ASWeapon* NewWeapon = GetWorld()->SpawnActor<ASWeapon>(DefaultInventoryClasses[i], SpawnInfo);

				MyPawn->AddWeapon(NewWeapon);
			}
		}
	}
}
*/

/************************************************************************/
/* Modding & Mutators                                                   */
/************************************************************************/


void AFusionGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	// HACK: workaround to inject CheckRelevance() into the BeginPlay sequence
	UFunction* Func = AActor::GetClass()->FindFunctionByName(FName(TEXT("ReceiveBeginPlay")));
	Func->FunctionFlags |= FUNC_Native;
	Func->SetNativeFunc((Native)&AFusionGameMode::BeginPlayMutatorHack);

	/* Spawn all mutators. */
	for (int32 i = 0; i < MutatorClasses.Num(); i++)
	{
		AddMutator(MutatorClasses[i]);
	}

	if (BaseMutator)
	{
		BaseMutator->InitGame(MapName, Options, ErrorMessage);
	}

	Super::InitGame(MapName, Options, ErrorMessage);
}


bool AFusionGameMode::CheckRelevance_Implementation(AActor* Other)
{
	/* Execute the first in the mutator chain */
	if (BaseMutator)
	{
		return BaseMutator->CheckRelevance(Other);
	}

	return true;
}


void AFusionGameMode::BeginPlayMutatorHack(FFrame& Stack, RESULT_DECL)
{
	P_FINISH;

	// WARNING: This function is called by every Actor in the level during his BeginPlay sequence. Meaning:  'this' is actually an AActor! Only do AActor things!
	if (!IsA(ALevelScriptActor::StaticClass()) && !IsA(AMutator::StaticClass()) &&
		(RootComponent == NULL || RootComponent->Mobility != EComponentMobility::Static || (!IsA(AStaticMeshActor::StaticClass()) && !IsA(ALight::StaticClass()))))
	{
		AFusionGameMode* Game = GetWorld()->GetAuthGameMode<AFusionGameMode>();
		// a few type checks being AFTER the CheckRelevance() call is intentional; want mutators to be able to modify, but not outright destroy
		if (Game != NULL && Game != this && !Game->CheckRelevance((AActor*)this) && !IsA(APlayerController::StaticClass()))
		{
			/* Actors are destroyed if they fail the relevance checks (which moves through the gamemode specific check AND the chain of mutators) */
			Destroy();
		}
	}
}


void AFusionGameMode::AddMutator(TSubclassOf<AMutator> MutClass)
{
	AMutator* NewMut = GetWorld()->SpawnActor<AMutator>(MutClass);
	if (NewMut)
	{
		if (BaseMutator == nullptr)
		{
			BaseMutator = NewMut;
		}
		else
		{
			// Add as child in chain
			BaseMutator->NextMutator = NewMut;
		}
	}
}