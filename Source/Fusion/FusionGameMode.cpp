// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Fusion.h"

#include "FusionHUD.h"
#include "FusionGameState.h"
#include "FusionCharacter.h"
#include "FusionBaseCharacter.h"
#include "FusionPlayerController.h"
#include "FusionPlayerState.h"
#include "FusionGameInstance.h"
#include "FusionGameSession.h"

#include "Player/FusionPlayerStart.h"
#include "Mutators/Mutator.h"

#include "FusionGameMode.h"


AFusionGameMode::AFusionGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Tracked/Characters/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	//static ConstructorHelpers::FClassFinder<APawn> BotPawnOb(TEXT("/Game/Blueprints/Pawns/BotPawn"));
	//BotPawnClass = BotPawnOb.Class;
	
	HUDClass = AFusionHUD::StaticClass();
	PlayerControllerClass = AFusionPlayerController::StaticClass();
	PlayerStateClass = AFusionPlayerState::StaticClass();
	//SpectatorClass = AShooterSpectatorPawn::StaticClass();
	GameStateClass = AFusionGameState::StaticClass();
	//ReplaySpectatorPlayerControllerClass = AShooterDemoSpectator::StaticClass();


	MinRespawnDelay = 5.0f;

	bAllowBots = true;
	bNeedsBotCreation = true;
	bUseSeamlessTravel = true;
}

FString AFusionGameMode::GetBotsCountOptionName()
{
	return FString(TEXT("Bots"));
}

void AFusionGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	const int32 BotsCountOptionValue = UGameplayStatics::GetIntOption(Options, GetBotsCountOptionName(), 0);
	SetAllowBots(BotsCountOptionValue > 0 ? true : false, BotsCountOptionValue);
	Super::InitGame(MapName, Options, ErrorMessage);

	const UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance && Cast<UFusionGameInstance>(GameInstance)->GetIsOnline())
	{
		bPauseable = false;
	}
}

void AFusionGameMode::SetAllowBots(bool bInAllowBots, int32 InMaxBots)
{
	bAllowBots = bInAllowBots;
	MaxBots = InMaxBots;
}

/** Returns game session class to use */
TSubclassOf<AGameSession> AFusionGameMode::GetGameSessionClass() const
{
	return AFusionGameSession::StaticClass();
}

void AFusionGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &AFusionGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void AFusionGameMode::DefaultTimer()
{
	// don't update timers for Play In Editor mode, it's not real match
	if (GetWorld()->IsPlayInEditor())
	{
		// start match if necessary.
		if (GetMatchState() == MatchState::WaitingToStart)
		{
			StartMatch();
		}
		return;
	}

	AFusionGameState* const MyGameState = Cast<AFusionGameState>(GameState);
	if (MyGameState && MyGameState->RemainingTime > 0 && !MyGameState->bTimerPaused)
	{
		MyGameState->RemainingTime--;

		if (MyGameState->RemainingTime <= 0)
		{
			if (GetMatchState() == MatchState::WaitingPostMatch)
			{
				RestartGame();
			}
			else if (GetMatchState() == MatchState::InProgress)
			{
				FinishMatch();

				// Send end round events
				for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
				{
					AFusionPlayerController* PlayerController = Cast<AFusionPlayerController>(*It);

					if (PlayerController && MyGameState)
					{
						AFusionPlayerState* PlayerState = Cast<AFusionPlayerState>((*It)->PlayerState);
						const bool bIsWinner = IsWinner(PlayerState);

						PlayerController->ClientSendRoundEndEvent(bIsWinner, MyGameState->ElapsedTime);
					}
				}
			}
			else if (GetMatchState() == MatchState::WaitingToStart)
			{
				StartMatch();
			}
		}
	}
}

void AFusionGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	if (bNeedsBotCreation)
	{
		//CreateBotControllers();
		bNeedsBotCreation = false;
	}

	if (bDelayedStart)
	{
		// start warmup if needed
		AFusionGameState* const MyGameState = Cast<AFusionGameState>(GameState);
		if (MyGameState && MyGameState->RemainingTime == 0)
		{
			const bool bWantsMatchWarmup = !GetWorld()->IsPlayInEditor();
			if (bWantsMatchWarmup && WarmupTime > 0)
			{
				MyGameState->RemainingTime = WarmupTime;
			}
			else
			{
				MyGameState->RemainingTime = 0.0f;
			}
		}
	}
}

void AFusionGameMode::HandleMatchHasStarted()
{
	bNeedsBotCreation = true;
	Super::HandleMatchHasStarted();

	AFusionGameState* const MyGameState = Cast<AFusionGameState>(GameState);
	MyGameState->RemainingTime = RoundTime;
	//StartBots();

	// notify players
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AFusionPlayerController* PC = Cast<AFusionPlayerController>(*It);
		if (PC)
		{
			PC->ClientGameStarted();
		}
	}
}

void AFusionGameMode::FinishMatch()
{
	AFusionGameState* const MyGameState = Cast<AFusionGameState>(GameState);
	if (IsMatchInProgress())
	{
		EndMatch();
		DetermineMatchWinner();

		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AFusionPlayerState* PlayerState = Cast<AFusionPlayerState>((*It)->PlayerState);
			const bool bIsWinner = IsWinner(PlayerState);

			(*It)->GameHasEnded(NULL, bIsWinner);
		}

		// lock all pawns
		// pawns are not marked as keep for seamless travel, so we will create new pawns on the next match rather than
		// turning these back on.
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			(*It)->TurnOff();
		}

		// set up to restart the match
		MyGameState->RemainingTime = TimeBetweenMatches;
	}
}

void AFusionGameMode::RequestFinishAndExitToMainMenu()
{
	FinishMatch();

	AFusionPlayerController* LocalPrimaryController = nullptr;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AFusionPlayerController* Controller = Cast<AFusionPlayerController>(*Iterator);

		if (Controller == NULL)
		{
			continue;
		}

		if (!Controller->IsLocalController())
		{
			const FString RemoteReturnReason = NSLOCTEXT("NetworkErrors", "HostHasLeft", "Host has left the game.").ToString();
			Controller->ClientReturnToMainMenu(RemoteReturnReason);
		}
		else
		{
			LocalPrimaryController = Controller;
		}
	}

	// GameInstance should be calling this from an EndState.  So call the PC function that performs cleanup, not the one that sets GI state.
	if (LocalPrimaryController != NULL)
	{
		LocalPrimaryController->HandleReturnToMainMenu();
	}
}

void AFusionGameMode::DetermineMatchWinner()
{
	// nothing to do here
}

bool AFusionGameMode::IsWinner(class AFusionPlayerState* PlayerState) const
{
	return false;
}

void AFusionGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	AFusionGameState* const MyGameState = Cast<AFusionGameState>(GameState);
	const bool bMatchIsOver = MyGameState && MyGameState->HasMatchEnded();
	if (bMatchIsOver)
	{
		ErrorMessage = TEXT("Match is over!");
	}
	else
	{
		// GameSession can be NULL if the match is over
		Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	}
}


void AFusionGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// update spectator location for client
	AFusionPlayerController* NewPC = Cast<AFusionPlayerController>(NewPlayer);
	if (NewPC && NewPC->GetPawn() == NULL)
	{
		NewPC->ClientSetSpectatorCamera(NewPC->GetSpawnLocation(), NewPC->GetControlRotation());
	}

	// notify new player if match is already in progress
	if (NewPC && IsMatchInProgress())
	{
		NewPC->ClientGameStarted();
		NewPC->ClientStartOnlineGame();
	}
}



bool AFusionGameMode::CanDealDamage(class AFusionPlayerState* DamageCauser, class AFusionPlayerState* DamagedPlayer) const
{
	return true;
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

void AFusionGameMode::Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	AFusionPlayerState* KillerPlayerState = Killer ? Cast<AFusionPlayerState>(Killer->PlayerState) : NULL;
	AFusionPlayerState* VictimPlayerState = KilledPlayer ? Cast<AFusionPlayerState>(KilledPlayer->PlayerState) : NULL;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->ScoreKill(VictimPlayerState, KillScore);
		KillerPlayerState->InformAboutKill(KillerPlayerState, DamageType, VictimPlayerState);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->ScoreDeath(KillerPlayerState, DeathScore);
		VictimPlayerState->BroadcastDeath(KillerPlayerState, DamageType, VictimPlayerState);
	}
}

bool AFusionGameMode::AllowCheats(APlayerController* P)
{
	return true;
}


bool AFusionGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	/* Always pick a random location */
	return false;
}


UClass* AFusionGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	//if (InController->IsA<AShooterAIController>())
	{
		//return BotPawnClass;
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* AFusionGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;
	TArray<APlayerStart*> FallbackSpawns;
	
	APlayerStart* BestStart = NULL;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* TestSpawn = *It;
		if (TestSpawn->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			BestStart = TestSpawn;
			break;
		}
		else
		{
			if (IsSpawnpointAllowed(TestSpawn, Player))
			{
				if (IsSpawnpointPreferred(TestSpawn, Player))
				{
					PreferredSpawns.Add(TestSpawn);
				}
				else
				{
					FallbackSpawns.Add(TestSpawn);
				}
			}
		}
	}


	

	if (BestStart == NULL)
	{
		if (PreferredSpawns.Num() > 0)
		{
			BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
		}
		else if (FallbackSpawns.Num() > 0)
		{
			BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
		}
	}
	return BestStart ? BestStart : Super::ChoosePlayerStart_Implementation(Player);
}

bool AFusionGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	if (Player == nullptr || Player->PlayerState == nullptr)
		return true;

	AFusionPlayerStart* FusionSpawnPoint = Cast<AFusionPlayerStart>(SpawnPoint);
	if (FusionSpawnPoint)
	{
		//AShooterAIController* AIController = Cast<AShooterAIController>(Player);
		//if (ShooterSpawnPoint->bNotForBots && AIController)
		{
			//return false;
		}

		if (FusionSpawnPoint->GetIsPlayerOnly() && !Player->PlayerState->bIsABot)
		{
			return true;
		}
		
		return false;
	}

	/* Cast failed, Anyone can spawn at the base playerstart class */
	return true;
}

bool AFusionGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const
{
	ACharacter* MyPawn = Cast<ACharacter>((*DefaultPawnClass)->GetDefaultObject<ACharacter>());
	//AShooterAIController* AIController = Cast<AShooterAIController>(Player);
	//if (AIController != nullptr)
	{
		//MyPawn = Cast<ACharacter>(BotPawnClass->GetDefaultObject<ACharacter>());
	}

	if (MyPawn)
	{
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			ACharacter* OtherPawn = Cast<ACharacter>(*It);
			if (OtherPawn && OtherPawn != MyPawn)
			{
				const float CombinedHeight = (MyPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * 2.0f;
				const float CombinedRadius = MyPawn->GetCapsuleComponent()->GetScaledCapsuleRadius() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
				const FVector OtherLocation = OtherPawn->GetActorLocation();

				// check if player start overlaps this pawn
				if (FMath::Abs(SpawnLocation.Z - OtherLocation.Z) < CombinedHeight && (SpawnLocation - OtherLocation).Size2D() < CombinedRadius)
				{
					return false;
				}
			}
		}

		/* Check if spawnpoint is exclusive to players */
		AFusionPlayerStart* MyPlayerStart = Cast<AFusionPlayerStart>(SpawnPoint);
		if (MyPlayerStart)
		{
			return MyPlayerStart->GetIsPlayerOnly() && !Player->PlayerState->bIsABot;
		}
	}
	else
	{
		return false;
	}

	return true;
}

/*
void AShooterGameMode::CreateBotControllers()
{
	UWorld* World = GetWorld();
	int32 ExistingBots = 0;
	for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It)
	{
		AShooterAIController* AIC = Cast<AShooterAIController>(*It);
		if (AIC)
		{
			++ExistingBots;
		}
	}

	// Create any necessary AIControllers.  Hold off on Pawn creation until pawns are actually necessary or need recreating.	
	int32 BotNum = ExistingBots;
	for (int32 i = 0; i < MaxBots - ExistingBots; ++i)
	{
		CreateBot(BotNum + i);
	}
}
*/

/*
AShooterAIController* AShooterGameMode::CreateBot(int32 BotNum)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = nullptr;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = nullptr;

	UWorld* World = GetWorld();
	AShooterAIController* AIC = World->SpawnActor<AShooterAIController>(SpawnInfo);
	InitBot(AIC, BotNum);

	return AIC;
}

void AShooterGameMode::StartBots()
{
	// checking number of existing human player.
	UWorld* World = GetWorld();
	for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It)
	{
		AShooterAIController* AIC = Cast<AShooterAIController>(*It);
		if (AIC)
		{
			RestartPlayer(AIC);
		}
	}
}

void AShooterGameMode::InitBot(AShooterAIController* AIController, int32 BotNum)
{
	if (AIController)
	{
		if (AIController->PlayerState)
		{
			FString BotName = FString::Printf(TEXT("Bot %d"), BotNum);
			AIController->PlayerState->PlayerName = BotName;
		}
	}
}
*/

void AFusionGameMode::RestartGame()
{
	// Hide the scoreboard too !
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AFusionPlayerController* PlayerController = Cast<AFusionPlayerController>(*It);
		if (PlayerController != nullptr)
		{
			AFusionHUD* FusionHUD = Cast<AFusionHUD>(PlayerController->GetHUD());
			if (FusionHUD != nullptr)
			{
				// Passing true to bFocus here ensures that focus is returned to the game viewport.
				//FusionHUD->ShowScoreboard(false, true);
			}
		}
	}

	Super::RestartGame();
}


