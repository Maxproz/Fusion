// @Maxpro 2017

#include "Fusion.h"

#include "Types/TakeHitInfo.h"
#include "Player/FusionGameUserSettings.h"
#include "Player/FusionPersistentUser.h"
#include "Player/FusionLocalPlayer.h"

#include "FusionPlayerController.h"
#include "FusionGameState.h"

#include "FusionOptions.h"



#define LOCTEXT_NAMESPACE "Fusion.HUD.Menu"
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( FPaths::GameContentDir() / "UI"/ RelativePath + TEXT(".ttf"), __VA_ARGS__ )


void FFusionOptions::Construct(ULocalPlayer* InPlayerOwner)
{

	BorderBrush = new FSlateBrush();
	BorderBrush->ImageSize = FVector2D(140.f, 25.f);
	BorderBrush->DrawAs = ESlateBrushDrawType::Image;
	BorderBrush->TintColor = FSlateColor(FLinearColor(FColor::White));


	// Fonts still need to be specified in code for now
	Style.Set("Fusion.MenuServerListTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 14))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.ScoreboardListTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 14))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.MenuProfileNameStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 18))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.MenuTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 20))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.MenuHeaderTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 26))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.WelcomeScreen.WelcomeTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 32))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.DefaultScoreboard.Row.HeaderTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 24))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FVector2D(0, 1))
	);

	Style.Set("Fusion.DefaultScoreboard.Row.StatTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 18))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FVector2D(0, 1))
	);

	Style.Set("Fusion.SplitScreenLobby.StartMatchTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 16))
		.SetColorAndOpacity(FLinearColor::Green)
		.SetShadowOffset(FVector2D(0, 1))
	);

	Style.Set("Fusion.DemoListCheckboxTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 12))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);


	PlayerOwner = InPlayerOwner;
	MinSensitivity = 1;

	TArray<FText> ResolutionList;
	TArray<FText> OnOffList;
	TArray<FText> SensitivityList;
	TArray<FText> GammaList;
	TArray<FText> LowHighList;

	FDisplayMetrics DisplayMetrics;
	FSlateApplication::Get().GetInitialDisplayMetrics(DisplayMetrics);

	bool bAddedNativeResolution = false;
	const FIntPoint NativeResolution(DisplayMetrics.PrimaryDisplayWidth, DisplayMetrics.PrimaryDisplayHeight);

	for (int32 i = 0; i < DefaultFusionResCount; i++)
	{
		if (DefaultFusionResolutions[i].X <= DisplayMetrics.PrimaryDisplayWidth && DefaultFusionResolutions[i].Y <= DisplayMetrics.PrimaryDisplayHeight)
		{
			ResolutionList.Add(FText::Format(FText::FromString("{0}x{1}"), FText::FromString(FString::FromInt(DefaultFusionResolutions[i].X)), FText::FromString(FString::FromInt(DefaultFusionResolutions[i].Y))));
			Resolutions.Add(DefaultFusionResolutions[i]);

			bAddedNativeResolution = bAddedNativeResolution || (DefaultFusionResolutions[i] == NativeResolution);
		}
	}

	// Always make sure that the native resolution is available
	if (!bAddedNativeResolution)
	{
		ResolutionList.Add(FText::Format(FText::FromString("{0}x{1}"), FText::FromString(FString::FromInt(NativeResolution.X)), FText::FromString(FString::FromInt(NativeResolution.Y))));
		Resolutions.Add(NativeResolution);
	}

	OnOffList.Add(LOCTEXT("Off", "OFF"));
	OnOffList.Add(LOCTEXT("On", "ON"));

	LowHighList.Add(LOCTEXT("Low", "LOW"));
	LowHighList.Add(LOCTEXT("High", "HIGH"));

	//Mouse sensitivity 0-50
	for (int32 i = 0; i < 51; i++)
	{
		SensitivityList.Add(FText::AsNumber(i));
	}

	for (int32 i = -50; i < 51; i++)
	{
		GammaList.Add(FText::AsNumber(i));
	}

	/** Options menu root item */
	TSharedPtr<FFusionMenuItemData> OptionsRoot = FFusionMenuItemData::CreateRoot();

	/** Cheats menu root item */
	TSharedPtr<FFusionMenuItemData> CheatsRoot = FFusionMenuItemData::CreateRoot();

	CheatsItem = MenuHelper::AddMenuItem(CheatsRoot, LOCTEXT("Cheats", "CHEATS"));
	MenuHelper::AddMenuOptionSP(CheatsItem, LOCTEXT("InfiniteAmmo", "INFINITE AMMO"), OnOffList, this, &FFusionOptions::InfiniteAmmoOptionChanged);
	MenuHelper::AddMenuOptionSP(CheatsItem, LOCTEXT("InfiniteClip", "INFINITE CLIP"), OnOffList, this, &FFusionOptions::InfiniteClipOptionChanged);
	MenuHelper::AddMenuOptionSP(CheatsItem, LOCTEXT("FreezeMatchTimer", "FREEZE MATCH TIMER"), OnOffList, this, &FFusionOptions::FreezeTimerOptionChanged);
	MenuHelper::AddMenuOptionSP(CheatsItem, LOCTEXT("HealthRegen", "HP REGENERATION"), OnOffList, this, &FFusionOptions::HealthRegenOptionChanged);

	OptionsItem = MenuHelper::AddMenuItem(OptionsRoot, LOCTEXT("Options", "OPTIONS"));
#if PLATFORM_DESKTOP
	VideoResolutionOption = MenuHelper::AddMenuOptionSP(OptionsItem, LOCTEXT("Resolution", "RESOLUTION"), ResolutionList, this, &FFusionOptions::VideoResolutionOptionChanged);
	GraphicsQualityOption = MenuHelper::AddMenuOptionSP(OptionsItem, LOCTEXT("Quality", "QUALITY"), LowHighList, this, &FFusionOptions::GraphicsQualityOptionChanged);
	FullScreenOption = MenuHelper::AddMenuOptionSP(OptionsItem, LOCTEXT("FullScreen", "FULL SCREEN"), OnOffList, this, &FFusionOptions::FullScreenOptionChanged);
#endif
	GammaOption = MenuHelper::AddMenuOptionSP(OptionsItem, LOCTEXT("Gamma", "GAMMA CORRECTION"), GammaList, this, &FFusionOptions::GammaOptionChanged);
	AimSensitivityOption = MenuHelper::AddMenuOptionSP(OptionsItem, LOCTEXT("AimSensitivity", "AIM SENSITIVITY"), SensitivityList, this, &FFusionOptions::AimSensitivityOptionChanged);
	InvertYAxisOption = MenuHelper::AddMenuOptionSP(OptionsItem, LOCTEXT("InvertYAxis", "INVERT Y AXIS"), OnOffList, this, &FFusionOptions::InvertYAxisOptionChanged);
	MenuHelper::AddMenuItemSP(OptionsItem, LOCTEXT("ApplyChanges", "APPLY CHANGES"), this, &FFusionOptions::OnApplySettings);

	//Do not allow to set aim sensitivity to 0
	AimSensitivityOption->MinMultiChoiceIndex = MinSensitivity;

	UserSettings = CastChecked<UFusionGameUserSettings>(GEngine->GetGameUserSettings());
	ResolutionOpt = UserSettings->GetScreenResolution();
	bFullScreenOpt = UserSettings->GetFullscreenMode();
	GraphicsQualityOpt = UserSettings->GetGraphicsQuality();

	UFusionPersistentUser* PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		bInvertYAxisOpt = PersistentUser->GetInvertedYAxis();
		SensitivityOpt = PersistentUser->GetAimSensitivity();
		GammaOpt = PersistentUser->GetGamma();
	}
	else
	{
		bInvertYAxisOpt = false;
		SensitivityOpt = 1.0f;
		GammaOpt = 2.2f;
	}
}

void FFusionOptions::OnApplySettings()
{
	//FSlateApplication::Get().PlaySound(OptionsStyle->AcceptChangesSound, GetOwnerUserIndex());
	ApplySettings();
}

void FFusionOptions::ApplySettings()
{
	UFusionPersistentUser* PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		PersistentUser->SetAimSensitivity(SensitivityOpt);
		PersistentUser->SetInvertedYAxis(bInvertYAxisOpt);
		PersistentUser->SetGamma(GammaOpt);
		PersistentUser->TellInputAboutKeybindings();

		PersistentUser->SaveIfDirty();
	}

	UserSettings->SetScreenResolution(ResolutionOpt);
	UserSettings->SetFullscreenMode(bFullScreenOpt);
	UserSettings->SetGraphicsQuality(GraphicsQualityOpt);
	UserSettings->ApplySettings(false);

	OnApplyChanges.ExecuteIfBound();
}

void FFusionOptions::TellInputAboutKeybindings()
{
	UFusionPersistentUser* PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();
	}
}

void FFusionOptions::RevertChanges()
{
	//FSlateApplication::Get().PlaySound(OptionsStyle->DiscardChangesSound, GetOwnerUserIndex());
	UpdateOptions();
	GEngine->DisplayGamma = 2.2f + 2.0f * (-0.5f + GammaOption->SelectedMultiChoice / 100.0f);
}

int32 FFusionOptions::GetCurrentResolutionIndex(FIntPoint CurrentRes)
{
	int32 Result = 0; // return first valid resolution if match not found
	for (int32 i = 0; i < Resolutions.Num(); i++)
	{
		if (Resolutions[i] == CurrentRes)
		{
			Result = i;
			break;
		}
	}
	return Result;
}

int32 FFusionOptions::GetCurrentMouseYAxisInvertedIndex()
{
	UFusionPersistentUser* PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		return InvertYAxisOption->SelectedMultiChoice = PersistentUser->GetInvertedYAxis() ? 1 : 0;
	}
	else
	{
		return 0;
	}
}

int32 FFusionOptions::GetCurrentMouseSensitivityIndex()
{
	UFusionPersistentUser* PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		//mouse sensitivity is a floating point value ranged from 0.0f to 1.0f
		int32 IntSensitivity = FMath::RoundToInt((PersistentUser->GetAimSensitivity() - 0.5f) * 10.0f);
		//Clamp to valid index range
		return FMath::Clamp(IntSensitivity, MinSensitivity, 100);
	}

	return FMath::RoundToInt((1.0f - 0.5f) * 10.0f);
}

int32 FFusionOptions::GetCurrentGammaIndex()
{
	UFusionPersistentUser* PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		//reverse gamma calculation
		int32 GammaIndex = FMath::TruncToInt(((PersistentUser->GetGamma() - 2.2f) / 2.0f + 0.5f) * 100);
		//Clamp to valid index range
		return FMath::Clamp(GammaIndex, 0, 100);
	}

	return FMath::TruncToInt(((2.2f - 2.2f) / 2.0f + 0.5f) * 100);
}

int32 FFusionOptions::GetOwnerUserIndex() const
{
	return PlayerOwner ? PlayerOwner->GetControllerId() : 0;
}

UFusionPersistentUser* FFusionOptions::GetPersistentUser() const
{
	UFusionLocalPlayer* const SLP = Cast<UFusionLocalPlayer>(PlayerOwner);
	if (SLP)
	{
		return SLP->GetPersistentUser();
	}

	return nullptr;
}

void FFusionOptions::UpdateOptions()
{
#if UE_BUILD_SHIPPING
	CheatsItem->bVisible = false;
#else
	//Toggle Cheat menu visibility depending if we are client or server
	UWorld* const World = PlayerOwner->GetWorld();
	if (World && World->GetNetMode() == NM_Client)
	{
		CheatsItem->bVisible = false;
	}
	else
	{
		CheatsItem->bVisible = true;
	}
#endif

	//grab the user settings
	UFusionPersistentUser* const PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		// Update bInvertYAxisOpt, SensitivityOpt and GammaOpt because the ShooterOptions can be created without the controller having a player
		// by the in-game menu which will leave them with default values
		bInvertYAxisOpt = PersistentUser->GetInvertedYAxis();
		SensitivityOpt = PersistentUser->GetAimSensitivity();
		GammaOpt = PersistentUser->GetGamma();
	}

	InvertYAxisOption->SelectedMultiChoice = GetCurrentMouseYAxisInvertedIndex();
	AimSensitivityOption->SelectedMultiChoice = GetCurrentMouseSensitivityIndex();
	GammaOption->SelectedMultiChoice = GetCurrentGammaIndex();

	GammaOptionChanged(GammaOption, GammaOption->SelectedMultiChoice);
#if PLATFORM_DESKTOP
	VideoResolutionOption->SelectedMultiChoice = GetCurrentResolutionIndex(UserSettings->GetScreenResolution());
	GraphicsQualityOption->SelectedMultiChoice = UserSettings->GetGraphicsQuality();
	FullScreenOption->SelectedMultiChoice = UserSettings->GetFullscreenMode() != EWindowMode::Windowed ? 1 : 0;
#endif
}

void FFusionOptions::InfiniteAmmoOptionChanged(TSharedPtr<FFusionMenuItemData> MenuItem, int32 MultiOptionIndex)
{
	UWorld* const World = PlayerOwner->GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			AFusionPlayerController* FusionPC = Cast<AFusionPlayerController>(*It);
			if (FusionPC)
			{
				FusionPC->SetInfiniteAmmo(MultiOptionIndex > 0 ? true : false);
			}
		}
	}
}

void FFusionOptions::InfiniteClipOptionChanged(TSharedPtr<FFusionMenuItemData> MenuItem, int32 MultiOptionIndex)
{
	UWorld* const World = PlayerOwner->GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			AFusionPlayerController* const FusionPC = Cast<AFusionPlayerController>(*It);
			if (FusionPC)
			{
				FusionPC->SetInfiniteClip(MultiOptionIndex > 0 ? true : false);
			}
		}
	}
}

void FFusionOptions::FreezeTimerOptionChanged(TSharedPtr<FFusionMenuItemData> MenuItem, int32 MultiOptionIndex)
{
	UWorld* const World = PlayerOwner->GetWorld();
	AFusionGameState* const GameState = World ? World->GetGameState<AFusionGameState>() : nullptr;
	if (GameState)
	{
		GameState->bTimerPaused = MultiOptionIndex > 0 ? true : false;
	}
}


void FFusionOptions::HealthRegenOptionChanged(TSharedPtr<FFusionMenuItemData> MenuItem, int32 MultiOptionIndex)
{
	UWorld* const World = PlayerOwner->GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			AFusionPlayerController* const FusionPC = Cast<AFusionPlayerController>(*It);
			if (FusionPC)
			{
				FusionPC->SetHealthRegen(MultiOptionIndex > 0 ? true : false);
			}
		}
	}
}

void FFusionOptions::VideoResolutionOptionChanged(TSharedPtr<FFusionMenuItemData> MenuItem, int32 MultiOptionIndex)
{
	ResolutionOpt = Resolutions[MultiOptionIndex];
}

void FFusionOptions::GraphicsQualityOptionChanged(TSharedPtr<FFusionMenuItemData> MenuItem, int32 MultiOptionIndex)
{
	GraphicsQualityOpt = MultiOptionIndex;
}

void FFusionOptions::FullScreenOptionChanged(TSharedPtr<FFusionMenuItemData> MenuItem, int32 MultiOptionIndex)
{
	static auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.FullScreenMode"));
	auto FullScreenMode = CVar->GetValueOnGameThread() == 1 ? EWindowMode::WindowedFullscreen : EWindowMode::Fullscreen;
	bFullScreenOpt = MultiOptionIndex == 0 ? EWindowMode::Windowed : FullScreenMode;
}

void FFusionOptions::AimSensitivityOptionChanged(TSharedPtr<FFusionMenuItemData> MenuItem, int32 MultiOptionIndex)
{
	SensitivityOpt = 0.5f + (MultiOptionIndex / 10.0f);
}

void FFusionOptions::GammaOptionChanged(TSharedPtr<FFusionMenuItemData> MenuItem, int32 MultiOptionIndex)
{
	GammaOpt = 2.2f + 2.0f * (-0.5f + MultiOptionIndex / 100.0f);
	GEngine->DisplayGamma = GammaOpt;
}

void FFusionOptions::InvertYAxisOptionChanged(TSharedPtr<FFusionMenuItemData> MenuItem, int32 MultiOptionIndex)
{
	bInvertYAxisOpt = MultiOptionIndex > 0 ? true : false;
}


#undef TTF_FONT
#undef LOCTEXT_NAMESPACE
