// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameState.h"
#include "FusionCharacter.h"
#include "FusionPlayerState.h"
#include "FusionGameInstance.h"
#include "FusionHUD.h"

#include "Widgets/Gameplay/InGameHUD.h"
#include "FusionGameViewportClient.h"

#include "Online.h"
#include "OnlineAchievementsInterface.h"
#include "OnlineEventsInterface.h"
#include "OnlineIdentityInterface.h"
#include "OnlineSessionInterface.h"

#include "Online/FusionLeaderboards.h"

#include "Player/FusionPlayerCameraManager.h"

#include "Weapons/MasterWeapon.h"

#include "Player/FusionLocalPlayer.h"
#include "Player/FusionPersistentUser.h"

#include "Widgets/Gameplay/InGameHUD.h"

//#include "Sound/SoundNodeLocalPlayer.h"
#include "AudioThread.h"

#include "FusionPlayerController.h"


#define  ACH_FRAG_SOMEONE	TEXT("ACH_FRAG_SOMEONE")
#define  ACH_SOME_KILLS		TEXT("ACH_SOME_KILLS")
#define  ACH_LOTS_KILLS		TEXT("ACH_LOTS_KILLS")
#define  ACH_FINISH_MATCH	TEXT("ACH_FINISH_MATCH")
#define  ACH_LOTS_MATCHES	TEXT("ACH_LOTS_MATCHES")
#define  ACH_FIRST_WIN		TEXT("ACH_FIRST_WIN")
#define  ACH_LOTS_WIN		TEXT("ACH_LOTS_WIN")
#define  ACH_MANY_WIN		TEXT("ACH_MANY_WIN")
#define  ACH_SHOOT_BULLETS	TEXT("ACH_SHOOT_BULLETS")
#define  ACH_SHOOT_ROCKETS	TEXT("ACH_SHOOT_ROCKETS")
#define  ACH_GOOD_SCORE		TEXT("ACH_GOOD_SCORE")
#define  ACH_GREAT_SCORE	TEXT("ACH_GREAT_SCORE")
#define  ACH_PLAY_DOWNFALL	TEXT("ACH_PLAY_DOWNFALL")

static const int32 SomeKillsCount = 10;
static const int32 LotsKillsCount = 20;
static const int32 LotsMatchesCount = 5;
static const int32 LotsWinsCount = 3;
static const int32 ManyWinsCount = 5;
static const int32 LotsBulletsCount = 100;
static const int32 LotsRocketsCount = 10;
static const int32 GoodScoreCount = 10;
static const int32 GreatScoreCount = 15;


/* Define a log category for error messages */
DEFINE_LOG_CATEGORY_STATIC(LogGame, Log, All);


AFusionPlayerController::AFusionPlayerController(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//PlayerCameraManagerClass = AShooterPlayerCameraManager::StaticClass();
	//CheatClass = UShooterCheatManager::StaticClass();
	bAllowGameActions = true;
	bGameEndedFrame = false;
	LastDeathLocation = FVector::ZeroVector;

	ServerSayString = TEXT("Say");
	FusionFriendUpdateTimer = 0.0f;
	bHasSentStartEvents = false;

}

void AFusionPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// These are bound to Tab
	InputComponent->BindAction("Scoreboard", IE_Pressed, this, &AFusionPlayerController::OnShowScoreboard);
	InputComponent->BindAction("Scoreboard", IE_Released, this, &AFusionPlayerController::OnHideScoreboard);

	// Unbound at the moment
	InputComponent->BindAction("ConditionalCloseScoreboard", IE_Pressed, this, &AFusionPlayerController::OnConditionalCloseScoreboard); 
	// Unbound at the moment
	InputComponent->BindAction("ToggleScoreboard", IE_Pressed, this, &AFusionPlayerController::OnToggleScoreboard); 
	
	/*
	// UI input
	InputComponent->BindAction("InGameMenu", IE_Pressed, this, &AFusionPlayerController::OnToggleInGameMenu);


	// voice chat
	InputComponent->BindAction("PushToTalk", IE_Pressed, this, &AFusionPlayerController::StartTalking);
	InputComponent->BindAction("PushToTalk", IE_Released, this, &AFusionPlayerController::StopTalking);

	InputComponent->BindAction("ToggleChat", IE_Pressed, this, &AFusionPlayerController::ToggleChatWindow);
	*/
}

void AFusionPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//FShooterStyle::Initialize();
	FusionFriendUpdateTimer = 0;

}

void AFusionPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		GetFusionHUD()->CreateGameWidgets();
		ClientShowInGameHUD();

		UFusionGameInstance* FGI = GetWorld() != NULL ? Cast<UFusionGameInstance>(GetWorld()->GetGameInstance()) : NULL;
		FGI->GotoState(FusionGameInstanceState::Playing);
	}
}

void AFusionPlayerController::ClientShowInGameHUD_Implementation()
{
	GetFusionHUD()->GetInGameHUDWidget()->ShowInGameHUD();
}

void AFusionPlayerController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (IsGameMenuVisible())
	{
		if (FusionFriendUpdateTimer > 0)
		{
			FusionFriendUpdateTimer -= DeltaTime;
		}
		else
		{
			//TSharedPtr<class FShooterFriends> ShooterFriends = ShooterIngameMenu->GetShooterFriends();
			ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
			//if (ShooterFriends.IsValid() && LocalPlayer && LocalPlayer->GetControllerId() >= 0)
			//{
				//ShooterFriends->UpdateFriends(LocalPlayer->GetControllerId());
			//}
			FusionFriendUpdateTimer = 4; //make sure the time between calls is long enough that we won't trigger (0x80552C81) and not exceed the web api rate limit
		}
	}

	// Is this the first frame after the game has ended
	if (bGameEndedFrame)
	{
		bGameEndedFrame = false;

		// ONLY PUT CODE HERE WHICH YOU DON'T WANT TO BE DONE DUE TO HOST LOSS

		// Do we need to show the end of round scoreboard?
		if (IsPrimaryPlayer())
		{
			AFusionHUD* FusionHUD = GetFusionHUD();
			if (FusionHUD)
			{
				FusionHUD->ShowScoreboard(true, true);
			}
		}
	}

	const bool bLocallyControlled = IsLocalController();
	const uint32 UniqueID = GetUniqueID();
	FAudioThread::RunCommandOnAudioThread([UniqueID, bLocallyControlled]()
	{
		//USoundNodeLocalPlayer::GetLocallyControlledActorCache().Add(UniqueID, bLocallyControlled);
	});
};

void AFusionPlayerController::BeginDestroy()
{
	Super::BeginDestroy();

	if (!GExitPurge)
	{
		const uint32 UniqueID = GetUniqueID();
		FAudioThread::RunCommandOnAudioThread([UniqueID]()
		{
			//USoundNodeLocalPlayer::GetLocallyControlledActorCache().Remove(UniqueID);
		});
	}
}

void AFusionPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);

	//Build menu only after game is initialized
	//ShooterIngameMenu = MakeShareable(new FShooterIngameMenu());
	//ShooterIngameMenu->Construct(Cast<ULocalPlayer>(Player));

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void AFusionPlayerController::QueryAchievements()
{
	// precache achievements
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer && LocalPlayer->GetControllerId() != -1)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
			if (Identity.IsValid())
			{
				TSharedPtr<const FUniqueNetId> UserId = Identity->GetUniquePlayerId(LocalPlayer->GetControllerId());

				if (UserId.IsValid())
				{
					IOnlineAchievementsPtr Achievements = OnlineSub->GetAchievementsInterface();

					if (Achievements.IsValid())
					{
						Achievements->QueryAchievements(*UserId.Get(), FOnQueryAchievementsCompleteDelegate::CreateUObject(this, &AFusionPlayerController::OnQueryAchievementsComplete));
					}
				}
				else
				{
					UE_LOG(LogOnline, Warning, TEXT("No valid user id for this controller."));
				}
			}
			else
			{
				UE_LOG(LogOnline, Warning, TEXT("No valid identity interface."));
			}
		}
		else
		{
			UE_LOG(LogOnline, Warning, TEXT("No default online subsystem."));
		}
	}
	else
	{
		UE_LOG(LogOnline, Warning, TEXT("No local player, cannot read achievements."));
	}
}

void AFusionPlayerController::OnQueryAchievementsComplete(const FUniqueNetId& PlayerId, const bool bWasSuccessful)
{
	UE_LOG(LogOnline, Display, TEXT("AFusionPlayerController::OnQueryAchievementsComplete(bWasSuccessful = %s)"), bWasSuccessful ? TEXT("TRUE") : TEXT("FALSE"));
}


void AFusionPlayerController::UnFreeze()
{
	Super::UnFreeze();

	ServerRestartPlayer();
	
	
}

void AFusionPlayerController::FailedToSpawnPawn()
{
	if (StateName == NAME_Inactive)
	{
		BeginInactiveState();
	}
	Super::FailedToSpawnPawn();
}

void AFusionPlayerController::PawnPendingDestroy(APawn* P)
{
	LastDeathLocation = P->GetActorLocation();
	FVector CameraLocation = LastDeathLocation + FVector(0, 0, 300.0f);
	FRotator CameraRotation(-90.0f, 0.0f, 0.0f);
	FindDeathCameraSpot(CameraLocation, CameraRotation);

	Super::PawnPendingDestroy(P);

	ClientSetSpectatorCamera(CameraLocation, CameraRotation);
}

void AFusionPlayerController::GameHasEnded(class AActor* EndGameFocus, bool bIsWinner)
{
	UpdateSaveFileOnGameEnd(bIsWinner);
	UpdateAchievementsOnGameEnd();
	UpdateLeaderboardsOnGameEnd();

	Super::GameHasEnded(EndGameFocus, bIsWinner);
}

void AFusionPlayerController::ClientSetSpectatorCamera_Implementation(FVector CameraLocation, FRotator CameraRotation)
{
	SetInitialLocationAndRotation(CameraLocation, CameraRotation);
	SetViewTarget(this);
}

bool AFusionPlayerController::FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation)
{
	const FVector PawnLocation = GetPawn()->GetActorLocation();
	FRotator ViewDir = GetControlRotation();
	ViewDir.Pitch = -45.0f;

	const float YawOffsets[] = { 0.0f, -180.0f, 90.0f, -90.0f, 45.0f, -45.0f, 135.0f, -135.0f };
	const float CameraOffset = 600.0f;
	FCollisionQueryParams TraceParams(TEXT("DeathCamera"), true, GetPawn());

	FHitResult HitResult;
	for (int32 i = 0; i < ARRAY_COUNT(YawOffsets); i++)
	{
		FRotator CameraDir = ViewDir;
		CameraDir.Yaw += YawOffsets[i];
		CameraDir.Normalize();

		const FVector TestLocation = PawnLocation - CameraDir.Vector() * CameraOffset;

		const bool bBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, PawnLocation, TestLocation, ECC_Camera, TraceParams);

		if (!bBlocked)
		{
			CameraLocation = TestLocation;
			CameraRotation = CameraDir;
			return true;
		}
	}

	return false;
}

bool AFusionPlayerController::ServerCheat_Validate(const FString& Msg)
{
	return true;
}

void AFusionPlayerController::ServerCheat_Implementation(const FString& Msg)
{
	if (CheatManager)
	{
		ClientMessage(ConsoleCommand(Msg));
	}
}

void AFusionPlayerController::SimulateInputKey(FKey Key, bool bPressed)
{
	InputKey(Key, bPressed ? IE_Pressed : IE_Released, 1, false);
}

void AFusionPlayerController::OnKill()
{
	UpdateAchievementProgress(ACH_FRAG_SOMEONE, 100.0f);

	const auto Events = Online::GetEventsInterface();
	const auto Identity = Online::GetIdentityInterface();

	if (Events.IsValid() && Identity.IsValid())
	{
		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
		if (LocalPlayer)
		{
			int32 UserIndex = LocalPlayer->GetControllerId();
			TSharedPtr<const FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);
			if (UniqueID.IsValid())
			{
				AFusionCharacter* FusionChar = Cast<AFusionCharacter>(GetCharacter());
				// If player is dead, use location stored during pawn cleanup.
				FVector Location = FusionChar ? FusionChar->GetActorLocation() : LastDeathLocation;
				AMasterWeapon* Weapon = FusionChar ? FusionChar->GetCurrentWeapon() : 0;
				int32 WeaponType = Weapon ? (int32)Weapon->GetAmmoType() : 0;

				FOnlineEventParms Params;

				Params.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
				Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
				Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

				Params.Add(TEXT("PlayerRoleId"), FVariantData((int32)0)); // unused
				Params.Add(TEXT("PlayerWeaponId"), FVariantData((int32)WeaponType));
				Params.Add(TEXT("EnemyRoleId"), FVariantData((int32)0)); // unused
				Params.Add(TEXT("EnemyWeaponId"), FVariantData((int32)0)); // untracked			
				Params.Add(TEXT("KillTypeId"), FVariantData((int32)0)); // unused
				Params.Add(TEXT("LocationX"), FVariantData(Location.X));
				Params.Add(TEXT("LocationY"), FVariantData(Location.Y));
				Params.Add(TEXT("LocationZ"), FVariantData(Location.Z));

				Events->TriggerEvent(*UniqueID, TEXT("KillOponent"), Params);
			}
		}
	}
}

void AFusionPlayerController::OnDeathMessage(class AFusionPlayerState* KillerPlayerState, class AFusionPlayerState* KilledPlayerState, const UDamageType* KillerDamageType)
{
	AFusionHUD* FusionHUD = GetFusionHUD();
	if (FusionHUD)
	{
		FusionHUD->ShowDeathMessage(KillerPlayerState, KilledPlayerState, KillerDamageType);
	}

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer && LocalPlayer->GetCachedUniqueNetId().IsValid() && KilledPlayerState->UniqueId.IsValid())
	{
		// if this controller is the player who died, update the hero stat.
		if (*LocalPlayer->GetCachedUniqueNetId() == *KilledPlayerState->UniqueId)
		{
			const auto Events = Online::GetEventsInterface();
			const auto Identity = Online::GetIdentityInterface();

			if (Events.IsValid() && Identity.IsValid())
			{
				const int32 UserIndex = LocalPlayer->GetControllerId();
				TSharedPtr<const FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);
				if (UniqueID.IsValid())
				{
					AFusionCharacter* FusionChar = Cast<AFusionCharacter>(GetCharacter());
					AMasterWeapon* Weapon = FusionChar ? FusionChar->GetCurrentWeapon() : NULL;

					FVector Location = FusionChar ? FusionChar->GetActorLocation() : FVector::ZeroVector;
					int32 WeaponType = Weapon ? (int32)Weapon->GetAmmoType() : 0;

					FOnlineEventParms Params;
					Params.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
					Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
					Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

					Params.Add(TEXT("PlayerRoleId"), FVariantData((int32)0)); // unused
					Params.Add(TEXT("PlayerWeaponId"), FVariantData((int32)WeaponType));
					Params.Add(TEXT("EnemyRoleId"), FVariantData((int32)0)); // unused
					Params.Add(TEXT("EnemyWeaponId"), FVariantData((int32)0)); // untracked

					Params.Add(TEXT("LocationX"), FVariantData(Location.X));
					Params.Add(TEXT("LocationY"), FVariantData(Location.Y));
					Params.Add(TEXT("LocationZ"), FVariantData(Location.Z));

					Events->TriggerEvent(*UniqueID, TEXT("PlayerDeath"), Params);
				}
			}
		}
	}
}

void AFusionPlayerController::UpdateAchievementProgress(const FString& Id, float Percent)
{
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
			if (Identity.IsValid())
			{
				TSharedPtr<const FUniqueNetId> UserId = LocalPlayer->GetCachedUniqueNetId();

				if (UserId.IsValid())
				{

					IOnlineAchievementsPtr Achievements = OnlineSub->GetAchievementsInterface();
					if (Achievements.IsValid() && (!WriteObject.IsValid() || WriteObject->WriteState != EOnlineAsyncTaskState::InProgress))
					{
						WriteObject = MakeShareable(new FOnlineAchievementsWrite());
						WriteObject->SetFloatStat(*Id, Percent);

						FOnlineAchievementsWriteRef WriteObjectRef = WriteObject.ToSharedRef();
						Achievements->WriteAchievements(*UserId, WriteObjectRef);
					}
					else
					{
						UE_LOG(LogOnline, Warning, TEXT("No valid achievement interface or another write is in progress."));
					}
				}
				else
				{
					UE_LOG(LogOnline, Warning, TEXT("No valid user id for this controller."));
				}
			}
			else
			{
				UE_LOG(LogOnline, Warning, TEXT("No valid identity interface."));
			}
		}
		else
		{
			UE_LOG(LogOnline, Warning, TEXT("No default online subsystem."));
		}
	}
	else
	{
		UE_LOG(LogOnline, Warning, TEXT("No local player, cannot update achievements."));
	}
}

void AFusionPlayerController::OnToggleInGameMenu()
{
	if (GEngine->GameViewport == nullptr)
	{
		return;
	}

	// this is not ideal, but necessary to prevent both players from pausing at the same time on the same frame
	UWorld* GameWorld = GEngine->GameViewport->GetWorld();

	for (auto It = GameWorld->GetControllerIterator(); It; ++It)
	{
		AFusionPlayerController* Controller = Cast<AFusionPlayerController>(*It);
		if (Controller && Controller->IsPaused())
		{
			return;
		}
	}

	// if no one's paused, pause
	//if (ShooterIngameMenu.IsValid())
	//{
		//ShooterIngameMenu->ToggleGameMenu();
	//}
}

void AFusionPlayerController::OnConditionalCloseScoreboard()
{
	AFusionHUD* FusionHUD = GetFusionHUD();
	if (FusionHUD && (FusionHUD->IsMatchOver() == false))
	{
		FusionHUD->ConditionalCloseScoreboard();
	}
}

void AFusionPlayerController::OnToggleScoreboard()
{
	AFusionHUD* FusionHUD = GetFusionHUD();
	if (FusionHUD && (FusionHUD->IsMatchOver() == false))
	{
		FusionHUD->ToggleScoreboard();
	}
}

void AFusionPlayerController::OnShowScoreboard()
{
	AFusionHUD* FusionHUD = GetFusionHUD();
	if (FusionHUD)
	{
		FusionHUD->ShowScoreboard(true);
	}
}

void AFusionPlayerController::OnHideScoreboard()
{
	AFusionHUD* FusionHUD = GetFusionHUD();
	// If have a valid match and the match is over - hide the scoreboard
	if ((FusionHUD != NULL) && (FusionHUD->IsMatchOver() == false))
	{
		FusionHUD->ShowScoreboard(false);
	}
}

bool AFusionPlayerController::IsGameMenuVisible() const
{
	bool Result = false;
	//if (ShooterIngameMenu.IsValid())
	{
		//Result = ShooterIngameMenu->GetIsGameMenuUp();
	}

	return Result;
}

void AFusionPlayerController::SetInfiniteAmmo(bool bEnable)
{
	bInfiniteAmmo = bEnable;
}

void AFusionPlayerController::SetInfiniteClip(bool bEnable)
{
	bInfiniteClip = bEnable;
}

void AFusionPlayerController::SetHealthRegen(bool bEnable)
{
	bHealthRegen = bEnable;
}

void AFusionPlayerController::SetGodMode(bool bEnable)
{
	bGodMode = bEnable;
}

void AFusionPlayerController::ClientReturnToMainMenu_Implementation(const FString& InReturnReason)
{
	UFusionGameInstance* FGI = GetWorld() != NULL ? Cast<UFusionGameInstance>(GetWorld()->GetGameInstance()) : NULL;

	if (!ensure(FGI != NULL))
	{
		return;
	}

	if (GetNetMode() == NM_Client)
	{
		const FText ReturnReason = NSLOCTEXT("NetworkErrors", "HostQuit", "The host has quit the match.");

		FGI->ShowMessageThenGotoState(ReturnReason, FusionGameInstanceState::MainMenu);
	}
	else
	{
		FGI->GotoState(FusionGameInstanceState::MainMenu);
	}

	// Clear the flag so we don't do normal end of round stuff next
	bGameEndedFrame = false;
}

void AFusionPlayerController::ClientGameEnded_Implementation(class AActor* EndGameFocus, bool bIsWinner)
{
	Super::ClientGameEnded_Implementation(EndGameFocus, bIsWinner);

	// Disable controls now the game has ended
	SetIgnoreMoveInput(true);

	bAllowGameActions = false;

	// Make sure that we still have valid view target
	SetViewTarget(GetPawn());

	AFusionHUD* FusionHUD = GetFusionHUD();
	if (FusionHUD)
	{
		FusionHUD->SetMatchState(bIsWinner ? EFusionMatchState::Won : EFusionMatchState::Lost);
	}

	UpdateSaveFileOnGameEnd(bIsWinner);
	UpdateAchievementsOnGameEnd();
	UpdateLeaderboardsOnGameEnd();

	// Flag that the game has just ended (if it's ended due to host loss we want to wait for ClientReturnToMainMenu_Implementation first, incase we don't want to process)
	bGameEndedFrame = true;
}

void AFusionPlayerController::SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning)
{
	Super::SetCinematicMode(bInCinematicMode, bHidePlayer, bAffectsHUD, bAffectsMovement, bAffectsTurning);

	// If we have a pawn we need to determine if we should show/hide the weapon
	AFusionCharacter* MyPawn = Cast<AFusionCharacter>(GetPawn());
	AMasterWeapon* MyWeapon = MyPawn ? MyPawn->GetCurrentWeapon() : NULL;
	if (MyWeapon)
	{
		if (bInCinematicMode && bHidePlayer)
		{
			MyWeapon->SetActorHiddenInGame(true);
		}
		else if (!bCinematicMode)
		{
			MyWeapon->SetActorHiddenInGame(false);
		}
	}
}

bool AFusionPlayerController::IsMoveInputIgnored() const
{
	if (IsInState(NAME_Spectating))
	{
		return false;
	}
	else
	{
		return Super::IsMoveInputIgnored();
	}
}

bool AFusionPlayerController::IsLookInputIgnored() const
{
	if (IsInState(NAME_Spectating))
	{
		return false;
	}
	else
	{
		return Super::IsLookInputIgnored();
	}
}

void AFusionPlayerController::InitInputSystem()
{
	Super::InitInputSystem();

	UFusionPersistentUser* PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();
	}
}

void AFusionPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AFusionPlayerController, bInfiniteAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFusionPlayerController, bInfiniteClip, COND_OwnerOnly);
}


void AFusionPlayerController::Suicide()
{
	if (IsInState(NAME_Playing))
	{
		ServerSuicide();
	}
}

void AFusionPlayerController::ServerSuicide_Implementation()
{
	if ((GetPawn() != NULL) && ((GetWorld()->TimeSeconds - GetPawn()->CreationTime > 1) || (GetNetMode() == NM_Standalone)))
	{
		AFusionCharacter* MyPawn = Cast<AFusionCharacter>(GetPawn());
		if (MyPawn)
		{
			MyPawn->Suicide();
		}
	}
}

bool AFusionPlayerController::HasInfiniteAmmo() const
{
	return bInfiniteAmmo;
}

bool AFusionPlayerController::HasInfiniteClip() const
{
	return bInfiniteClip;
}

bool AFusionPlayerController::HasHealthRegen() const
{
	return bHealthRegen;
}

bool AFusionPlayerController::HasGodMode() const
{
	return bGodMode;
}


void AFusionPlayerController::ToggleChatWindow()
{
	AFusionHUD* FusionHUD = Cast<AFusionHUD>(GetHUD());
	if (FusionHUD)
	{
		//FusionHUD->ToggleChat();
	}
}

void AFusionPlayerController::ClientTeamMessage_Implementation(APlayerState* SenderPlayerState, const FString& S, FName Type, float MsgLifeTime)
{
	AFusionHUD* FusionHUD = Cast<AFusionHUD>(GetHUD());
	if (FusionHUD)
	{
		if (Type == ServerSayString)
		{
			if (SenderPlayerState != PlayerState)
			{
				//FusionHUD->AddChatLine(FText::FromString(S), false);
			}
		}
	}
}

void AFusionPlayerController::Say(const FString& Msg)
{
	ServerSay(Msg.Left(128));
}

bool AFusionPlayerController::ServerSay_Validate(const FString& Msg)
{
	return true;
}

void AFusionPlayerController::ServerSay_Implementation(const FString& Msg)
{
	//GetWorld()->GetAuthGameMode<AFusionGameMode>()->Broadcast(this, Msg, ServerSayString);
}

UFusionPersistentUser* AFusionPlayerController::GetPersistentUser() const
{
	UFusionLocalPlayer* const FusionLocalPlayer = Cast<UFusionLocalPlayer>(Player);
	return FusionLocalPlayer ? FusionLocalPlayer->GetPersistentUser() : nullptr;
}

bool AFusionPlayerController::SetPause(bool bPause, FCanUnpause CanUnpauseDelegate /*= FCanUnpause()*/)
{
	const bool Result = APlayerController::SetPause(bPause, CanUnpauseDelegate);

	// Update rich presence.
	const auto PresenceInterface = Online::GetPresenceInterface();
	const auto Events = Online::GetEventsInterface();
	const auto LocalPlayer = Cast<ULocalPlayer>(Player);
	TSharedPtr<const FUniqueNetId> UserId = LocalPlayer ? LocalPlayer->GetCachedUniqueNetId() : nullptr;

	if (PresenceInterface.IsValid() && UserId.IsValid())
	{
		FOnlineUserPresenceStatus PresenceStatus;
		if (Result && bPause)
		{
			PresenceStatus.Properties.Add(DefaultPresenceKey, FString("Paused"));
		}
		else
		{
			PresenceStatus.Properties.Add(DefaultPresenceKey, FString("InGame"));
		}
		PresenceInterface->SetPresence(*UserId, PresenceStatus);

	}

	// Don't send pause events while online since the game doesn't actually pause
	if (GetNetMode() == NM_Standalone && Events.IsValid() && PlayerState->UniqueId.IsValid())
	{
		FOnlineEventParms Params;
		Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
		Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
		if (Result && bPause)
		{
			Events->TriggerEvent(*PlayerState->UniqueId, TEXT("PlayerSessionPause"), Params);
		}
		else
		{
			Events->TriggerEvent(*PlayerState->UniqueId, TEXT("PlayerSessionResume"), Params);
		}
	}

	return Result;
}


void AFusionPlayerController::ShowInGameMenu()
{
	AFusionHUD* FusionHUD = GetFusionHUD();
	//if (ShooterIngameMenu.IsValid() && !ShooterIngameMenu->GetIsGameMenuUp() && ShooterHUD && (ShooterHUD->IsMatchOver() == false))
	{
		//ShooterIngameMenu->ToggleGameMenu();
	}
}
void AFusionPlayerController::UpdateAchievementsOnGameEnd()
{
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer)
	{
		AFusionPlayerState* FusionPlayerState = Cast<AFusionPlayerState>(PlayerState);
		if (FusionPlayerState)
		{
			const UFusionPersistentUser*  PersistentUser = GetPersistentUser();

			if (PersistentUser)
			{
				const int32 Wins = PersistentUser->GetWins();
				const int32 Losses = PersistentUser->GetLosses();
				const int32 Matches = Wins + Losses;

				const int32 TotalKills = PersistentUser->GetKills();
				const int32 MatchScore = (int32)FusionPlayerState->GetScore();

				const int32 TotalBulletsFired = PersistentUser->GetBulletsFired();
				const int32 TotalRocketsFired = PersistentUser->GetRocketsFired();

				float TotalGameAchievement = 0;
				float CurrentGameAchievement = 0;

				///////////////////////////////////////
				// Kill achievements
				if (TotalKills >= 1)
				{
					CurrentGameAchievement += 100.0f;
				}
				TotalGameAchievement += 100;

				{
					float fSomeKillPct = ((float)TotalKills / (float)SomeKillsCount) * 100.0f;
					fSomeKillPct = FMath::RoundToFloat(fSomeKillPct);
					UpdateAchievementProgress(ACH_SOME_KILLS, fSomeKillPct);

					CurrentGameAchievement += FMath::Min(fSomeKillPct, 100.0f);
					TotalGameAchievement += 100;
				}

				{
					float fLotsKillPct = ((float)TotalKills / (float)LotsKillsCount) * 100.0f;
					fLotsKillPct = FMath::RoundToFloat(fLotsKillPct);
					UpdateAchievementProgress(ACH_LOTS_KILLS, fLotsKillPct);

					CurrentGameAchievement += FMath::Min(fLotsKillPct, 100.0f);
					TotalGameAchievement += 100;
				}
				///////////////////////////////////////

				///////////////////////////////////////
				// Match Achievements
				{
					UpdateAchievementProgress(ACH_FINISH_MATCH, 100.0f);

					CurrentGameAchievement += 100;
					TotalGameAchievement += 100;
				}

				{
					float fLotsRoundsPct = ((float)Matches / (float)LotsMatchesCount) * 100.0f;
					fLotsRoundsPct = FMath::RoundToFloat(fLotsRoundsPct);
					UpdateAchievementProgress(ACH_LOTS_MATCHES, fLotsRoundsPct);

					CurrentGameAchievement += FMath::Min(fLotsRoundsPct, 100.0f);
					TotalGameAchievement += 100;
				}
				///////////////////////////////////////

				///////////////////////////////////////
				// Win Achievements
				if (Wins >= 1)
				{
					UpdateAchievementProgress(ACH_FIRST_WIN, 100.0f);

					CurrentGameAchievement += 100.0f;
				}
				TotalGameAchievement += 100;

				{
					float fLotsWinPct = ((float)Wins / (float)LotsWinsCount) * 100.0f;
					fLotsWinPct = FMath::RoundToInt(fLotsWinPct);
					UpdateAchievementProgress(ACH_LOTS_WIN, fLotsWinPct);

					CurrentGameAchievement += FMath::Min(fLotsWinPct, 100.0f);
					TotalGameAchievement += 100;
				}

				{
					float fManyWinPct = ((float)Wins / (float)ManyWinsCount) * 100.0f;
					fManyWinPct = FMath::RoundToInt(fManyWinPct);
					UpdateAchievementProgress(ACH_MANY_WIN, fManyWinPct);

					CurrentGameAchievement += FMath::Min(fManyWinPct, 100.0f);
					TotalGameAchievement += 100;
				}
				///////////////////////////////////////

				///////////////////////////////////////
				// Ammo Achievements
				{
					float fLotsBulletsPct = ((float)TotalBulletsFired / (float)LotsBulletsCount) * 100.0f;
					fLotsBulletsPct = FMath::RoundToFloat(fLotsBulletsPct);
					UpdateAchievementProgress(ACH_SHOOT_BULLETS, fLotsBulletsPct);

					CurrentGameAchievement += FMath::Min(fLotsBulletsPct, 100.0f);
					TotalGameAchievement += 100;
				}

				{
					float fLotsRocketsPct = ((float)TotalRocketsFired / (float)LotsRocketsCount) * 100.0f;
					fLotsRocketsPct = FMath::RoundToFloat(fLotsRocketsPct);
					UpdateAchievementProgress(ACH_SHOOT_ROCKETS, fLotsRocketsPct);

					CurrentGameAchievement += FMath::Min(fLotsRocketsPct, 100.0f);
					TotalGameAchievement += 100;
				}
				///////////////////////////////////////

				///////////////////////////////////////
				// Score Achievements
				{
					float fGoodScorePct = ((float)MatchScore / (float)GoodScoreCount) * 100.0f;
					fGoodScorePct = FMath::RoundToFloat(fGoodScorePct);
					UpdateAchievementProgress(ACH_GOOD_SCORE, fGoodScorePct);
				}

				{
					float fGreatScorePct = ((float)MatchScore / (float)GreatScoreCount) * 100.0f;
					fGreatScorePct = FMath::RoundToFloat(fGreatScorePct);
					UpdateAchievementProgress(ACH_GREAT_SCORE, fGreatScorePct);
				}
				///////////////////////////////////////

				///////////////////////////////////////
				// Map Play Achievements
				UWorld* World = GetWorld();
				if (World)
				{
					FString MapName = *FPackageName::GetShortName(World->PersistentLevel->GetOutermost()->GetName());
					if (MapName.Find(TEXT("Downfall")) != -1)
					{
						UpdateAchievementProgress(ACH_PLAY_DOWNFALL, 100.0f);
					}/*
					else if (MapName.Find(TEXT("Sanctuary")) != -1)
					{
						UpdateAchievementProgress(ACH_PLAY_SANCTUARY, 100.0f);
					}
					*/
				}
				///////////////////////////////////////			

				const auto Events = Online::GetEventsInterface();
				const auto Identity = Online::GetIdentityInterface();

				if (Events.IsValid() && Identity.IsValid())
				{
					const int32 UserIndex = LocalPlayer->GetControllerId();
					TSharedPtr<const FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);
					if (UniqueID.IsValid())
					{
						FOnlineEventParms Params;

						float fGamePct = (CurrentGameAchievement / TotalGameAchievement) * 100.0f;
						fGamePct = FMath::RoundToFloat(fGamePct);
						Params.Add(TEXT("CompletionPercent"), FVariantData((float)fGamePct));
						if (UniqueID.IsValid())
						{
							Events->TriggerEvent(*UniqueID, TEXT("GameProgress"), Params);
						}
					}
				}
			}
		}
	}
}

void AFusionPlayerController::UpdateLeaderboardsOnGameEnd()
{
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer)
	{
		// update leaderboards - note this does not respect existing scores and overwrites them. We would first need to read the leaderboards if we wanted to do that.
		IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
			if (Identity.IsValid())
			{
				TSharedPtr<const FUniqueNetId> UserId = Identity->GetUniquePlayerId(LocalPlayer->GetControllerId());
				if (UserId.IsValid())
				{
					IOnlineLeaderboardsPtr Leaderboards = OnlineSub->GetLeaderboardsInterface();
					if (Leaderboards.IsValid())
					{
						AFusionPlayerState* FusionPlayerState = Cast<AFusionPlayerState>(PlayerState);
						if (FusionPlayerState)
						{
							FFusionAllTimeMatchResultsWrite ResultsWriteObject;

							ResultsWriteObject.SetIntStat(LEADERBOARD_STAT_SCORE, FusionPlayerState->GetScore());
							ResultsWriteObject.SetIntStat(LEADERBOARD_STAT_KILLS, FusionPlayerState->GetKills());
							ResultsWriteObject.SetIntStat(LEADERBOARD_STAT_DEATHS, FusionPlayerState->GetDeaths());
							ResultsWriteObject.SetIntStat(LEADERBOARD_STAT_MATCHESPLAYED, 1);

							// the call will copy the user id and write object to its own memory
							Leaderboards->WriteLeaderboards(FusionPlayerState->SessionName, *UserId, ResultsWriteObject);
							Leaderboards->FlushLeaderboards(TEXT("FUSION"));
						}
					}
				}
			}
		}
	}
}

void AFusionPlayerController::UpdateSaveFileOnGameEnd(bool bIsWinner)
{
	AFusionPlayerState* FusionPlayerState = Cast<AFusionPlayerState>(PlayerState);
	if (FusionPlayerState)
	{
		// update local saved profile
		UFusionPersistentUser* const PersistentUser = GetPersistentUser();
		if (PersistentUser)
		{
			PersistentUser->AddMatchResult(FusionPlayerState->GetKills(), FusionPlayerState->GetDeaths(), FusionPlayerState->GetNumBulletsFired(), FusionPlayerState->GetNumRocketsFired(), bIsWinner);
			PersistentUser->SaveIfDirty();
		}
	}
}



void AFusionPlayerController::StartSpectating()
{
	/*
	// Update the state on server 
	PlayerState->bIsSpectator = true;
	// Waiting to respawn 
	bPlayerIsWaiting = true;
	ChangeState(NAME_Spectating);
	// Push the state update to the client 
	ClientGotoState(NAME_Spectating);

	// Focus on the remaining alive player 
	ViewAPlayer(1);

	// Update the HUD to show the spectator screen 
	ClientHUDStateChanged(EHUDState::Spectating);
	*/
}


bool AFusionPlayerController::ServerSuicide_Validate()
{
	return true;
}




/* Remove the namespace definition so it doesn't exist in other files compiled after this one. */
#undef LOCTEXT_NAMESPACE


void AFusionPlayerController::ClientGameStarted_Implementation()
{
	bAllowGameActions = true;

	// Enable controls mode now the game has started
	SetIgnoreMoveInput(false);

	AFusionHUD* FusionHUD = GetFusionHUD();
	if (FusionHUD)
	{
		FusionHUD->SetMatchState(EFusionMatchState::Playing);
		FusionHUD->ShowScoreboard(false);
	}
	bGameEndedFrame = false;

	QueryAchievements();

	// Send round start event
	const auto Events = Online::GetEventsInterface();
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	if (LocalPlayer != nullptr && Events.IsValid())
	{
		auto UniqueId = LocalPlayer->GetPreferredUniqueNetId();

		if (UniqueId.IsValid())
		{
			// Generate a new session id
			Events->SetPlayerSessionId(*UniqueId, FGuid::NewGuid());

			FString MapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());

			// Fire session start event for all cases
			FOnlineEventParms Params;
			Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
			Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("MapName"), FVariantData(MapName));

			Events->TriggerEvent(*UniqueId, TEXT("PlayerSessionStart"), Params);

			// Online matches require the MultiplayerRoundStart event as well
			UFusionGameInstance* FGI = GetWorld() != NULL ? Cast<UFusionGameInstance>(GetWorld()->GetGameInstance()) : NULL;


			if (FGI->GetIsOnline())
			{
				FOnlineEventParms MultiplayerParams;

				// @todo: fill in with real values
				MultiplayerParams.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
				MultiplayerParams.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
				MultiplayerParams.Add(TEXT("MatchTypeId"), FVariantData((int32)1)); // @todo abstract the specific meaning of this value across platforms
				MultiplayerParams.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

				Events->TriggerEvent(*UniqueId, TEXT("MultiplayerRoundStart"), MultiplayerParams);
			}

			bHasSentStartEvents = true;
		}
	}
}

/** Starts the online game using the session name in the PlayerState */
void AFusionPlayerController::ClientStartOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	AFusionPlayerState* FusionPlayerState = Cast<AFusionPlayerState>(PlayerState);
	if (FusionPlayerState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				UE_LOG(LogOnline, Log, TEXT("Starting session %s on client"), *FusionPlayerState->SessionName.ToString());
				Sessions->StartSession(FusionPlayerState->SessionName);
			}
		}
	}
	else
	{
		// Keep retrying until player state is replicated
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ClientStartOnlineGame, this, &AFusionPlayerController::ClientStartOnlineGame_Implementation, 0.2f, false);
	}
}

/** Ends the online game using the session name in the PlayerState */
void AFusionPlayerController::ClientEndOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	AFusionPlayerState* FusionPlayerState = Cast<AFusionPlayerState>(PlayerState);
	if (FusionPlayerState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				UE_LOG(LogOnline, Log, TEXT("Ending session %s on client"), *FusionPlayerState->SessionName.ToString());
				Sessions->EndSession(FusionPlayerState->SessionName);
			}
		}
	}
}

void AFusionPlayerController::ClientSendRoundEndEvent_Implementation(bool bIsWinner, int32 ExpendedTimeInSeconds)
{
	const auto Events = Online::GetEventsInterface();
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	if (bHasSentStartEvents && LocalPlayer != nullptr && Events.IsValid())
	{
		auto UniqueId = LocalPlayer->GetPreferredUniqueNetId();

		if (UniqueId.IsValid())
		{
			FString MapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());
			AFusionPlayerState* FusionPlayerState = Cast<AFusionPlayerState>(PlayerState);
			int32 PlayerScore = FusionPlayerState ? FusionPlayerState->GetScore() : 0;

			// Fire session end event for all cases
			FOnlineEventParms Params;
			Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
			Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("ExitStatusId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("PlayerScore"), FVariantData((int32)PlayerScore));
			Params.Add(TEXT("PlayerWon"), FVariantData((bool)bIsWinner));
			Params.Add(TEXT("MapName"), FVariantData(MapName));
			Params.Add(TEXT("MapNameString"), FVariantData(MapName)); // @todo workaround for a bug in backend service, remove when fixed

			Events->TriggerEvent(*UniqueId, TEXT("PlayerSessionEnd"), Params);

			// Online matches require the MultiplayerRoundEnd event as well
			UFusionGameInstance* FGI = GetWorld() != NULL ? Cast<UFusionGameInstance>(GetWorld()->GetGameInstance()) : NULL;
			if (FGI->GetIsOnline())
			{
				FOnlineEventParms MultiplayerParams;

				AFusionGameState* const MyGameState = GetWorld() != NULL ? GetWorld()->GetGameState<AFusionGameState>() : NULL;
				if (ensure(MyGameState != nullptr))
				{
					MultiplayerParams.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
					MultiplayerParams.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
					MultiplayerParams.Add(TEXT("MatchTypeId"), FVariantData((int32)1)); // @todo abstract the specific meaning of this value across platforms
					MultiplayerParams.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
					MultiplayerParams.Add(TEXT("TimeInSeconds"), FVariantData((float)ExpendedTimeInSeconds));
					MultiplayerParams.Add(TEXT("ExitStatusId"), FVariantData((int32)0)); // unused

					Events->TriggerEvent(*UniqueId, TEXT("MultiplayerRoundEnd"), MultiplayerParams);
				}
			}
		}

		bHasSentStartEvents = false;
	}
}


bool AFusionPlayerController::IsGameInputAllowed() const
{
	return bAllowGameActions && !bCinematicMode;
}

AFusionHUD* AFusionPlayerController::GetFusionHUD() const
{
	return Cast<AFusionHUD>(GetHUD());
}

void AFusionPlayerController::HandleReturnToMainMenu()
{
	OnHideScoreboard();
	CleanupSessionOnReturnToMenu();
}




/** Ends and/or destroys game session */
void AFusionPlayerController::CleanupSessionOnReturnToMenu()
{
	UFusionGameInstance * FGI = GetWorld() != NULL ? Cast<UFusionGameInstance>(GetWorld()->GetGameInstance()) : NULL;

	if (ensure(FGI != NULL))
	{
		FGI->CleanupSessionOnReturnToMenu();
	}
}


void AFusionPlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);

	if (GetWorld() != NULL)
	{
		UFusionGameViewportClient* FusionViewport = Cast<UFusionGameViewportClient>(GetWorld()->GetGameViewport());

		if (FusionViewport != NULL)
		{
			FusionViewport->ShowLoadingScreen();
		}

		AFusionHUD* FusionHUD = Cast<AFusionHUD>(GetHUD());
		if (FusionHUD != nullptr)
		{
			// Passing true to bFocus here ensures that focus is returned to the game viewport.
			FusionHUD->ShowScoreboard(false, true);
		}
	}
}


