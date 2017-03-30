// @Maxpro 2017

#include "Fusion.h"

#include "Player/FusionGameUserSettings.h"
#include "Player/FusionPersistentUser.h"
#include "Player/FusionLocalPlayer.h"

#include "FusionHUD.h"
#include "FusionPlayerController_Menu.h"

#include "MainMenuOptions_Widget.h"




void UMainMenuOptions_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	//PlayerOwner = GetOwningLocalPlayer();
	AFusionPlayerController_Menu* MPC = Cast<AFusionPlayerController_Menu>(GetOwningPlayer());
	
	PlayerOwner = MPC->GetLocalPlayer();
	PlayerHUDRef = MPC->GetFusionHUD();

	DefaultFusionResolutions[0] = FIntPoint(1024, 768); 
	DefaultFusionResolutions[1] = FIntPoint(1280, 720);
	DefaultFusionResolutions[2] = FIntPoint(1920, 1080);

	// Add the indexes for the Quality combo box
	const FString FirstQualityIndex("Low");
	const FString SecondQualityIndex("High");

	QualityComboBox->AddOption(FirstQualityIndex);
	QualityComboBox->AddOption(SecondQualityIndex);

	// Add the indexes for the Resolution combo box
	const FString FirstResolutionIndex("1024x768");
	const FString SecondResolutionIndex("1280x720");
	const FString ThirdResolutionIndex("1920x1080");

	ResolutionComboBox->AddOption(FirstResolutionIndex);
	ResolutionComboBox->AddOption(SecondResolutionIndex);
	ResolutionComboBox->AddOption(ThirdResolutionIndex);


	UserSettings = CastChecked<UFusionGameUserSettings>(GEngine->GetGameUserSettings());
	CurrentResolution = UserSettings->GetScreenResolution();
	CurrentWindowMode = UserSettings->GetFullscreenMode();
	
	if (CurrentWindowMode == EWindowMode::WindowedFullscreen || CurrentWindowMode == EWindowMode::Fullscreen)
	{
		bIsCurrentlyFullScreen = true;
		OnOffTextBlock->SetText(FText::FromString("On"));
	}
	else
	{
		bIsCurrentlyFullScreen = false;
		OnOffTextBlock->SetText(FText::FromString("Off"));
	}
	
	GraphicsQuality = UserSettings->GetGraphicsQuality();



	UFusionPersistentUser* PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		CurrentGamma = PersistentUser->GetGamma();
	}
	else
	{
		CurrentGamma = 2.2f;
	}

	GammaSpinBox->SetValue(CurrentGamma);
	ResolutionComboBox->SetSelectedOption(ResolutionComboBox->GetOptionAtIndex(0));
	QualityComboBox->SetSelectedOption(QualityComboBox->GetOptionAtIndex(1));


	ApplyChangesButton->OnClicked.AddDynamic(this, &UMainMenuOptions_Widget::OnClickedApplyChangesButton);
	FullScreenToggleButton->OnClicked.AddDynamic(this, &UMainMenuOptions_Widget::OnClickedFullScreenToggleButton);
	GammaSpinBox->OnValueChanged.AddDynamic(this, &UMainMenuOptions_Widget::OnValueChangedGammaSpinBox);
	GammaSpinBox->OnValueCommitted.AddDynamic(this, &UMainMenuOptions_Widget::OnValueCommittedGammaSpinBox);
	QualityComboBox->OnSelectionChanged.AddDynamic(this, &UMainMenuOptions_Widget::OnSelectionChangedQualityComboBox);
	ResolutionComboBox->OnSelectionChanged.AddDynamic(this, &UMainMenuOptions_Widget::OnSelectionChangedResolutionComboBox);
}

UFusionPersistentUser* UMainMenuOptions_Widget::GetPersistentUser() const
{
	UFusionLocalPlayer* const SLP = Cast<UFusionLocalPlayer>(PlayerOwner);
	if (SLP)
	{
		return SLP->GetPersistentUser();
	}

	return nullptr;
}

void UMainMenuOptions_Widget::OnClickedApplyChangesButton()
{
	UFusionPersistentUser* PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		PersistentUser->SetGamma(CurrentGamma);
		PersistentUser->TellInputAboutKeybindings();
		PersistentUser->SaveIfDirty();
	}
	UserSettings->SetScreenResolution(CurrentResolution);
	UserSettings->SetFullscreenMode(CurrentWindowMode);
	UserSettings->SetGraphicsQuality(GraphicsQuality);
	UserSettings->ApplySettings(false);

	PlayerHUDRef->HideMainMenuOptions();
	PlayerHUDRef->ShowMainMenu();
}

void UMainMenuOptions_Widget::OnClickedFullScreenToggleButton()
{
	static auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.FullScreenMode"));
	auto FullScreenMode = CVar->GetValueOnGameThread() == 1 ? EWindowMode::WindowedFullscreen : EWindowMode::Fullscreen;

	if (bIsCurrentlyFullScreen)
	{
		bIsCurrentlyFullScreen = false;
		OnOffTextBlock->SetText(FText::FromString("Off"));
		CurrentWindowMode = EWindowMode::Windowed;
	}
	else
	{
		bIsCurrentlyFullScreen = true;
		OnOffTextBlock->SetText(FText::FromString("On"));
		CurrentWindowMode = FullScreenMode;
	}
}

void UMainMenuOptions_Widget::OnValueChangedGammaSpinBox(const float Value)
{
	CurrentGamma = 2.2f + 2.0f * (-0.5f + Value / 100.0f);
	GEngine->DisplayGamma = CurrentGamma;
}

void UMainMenuOptions_Widget::OnValueCommittedGammaSpinBox(const float Value, ETextCommit::Type Method)
{
	if (Method == ETextCommit::OnEnter)
	{
		CurrentGamma = 2.2f + 2.0f * (-0.5f + Value / 100.0f);
		GEngine->DisplayGamma = CurrentGamma;
	}
}

void UMainMenuOptions_Widget::OnSelectionChangedQualityComboBox(const FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectedItem == QualityComboBox->GetOptionAtIndex(0))
	{
		GraphicsQuality = 0;

	}
	else if (SelectedItem == QualityComboBox->GetOptionAtIndex(1))
	{
		GraphicsQuality = 1;

	}
}

void UMainMenuOptions_Widget::OnSelectionChangedResolutionComboBox(const FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectedItem == ResolutionComboBox->GetOptionAtIndex(0))
	{
		CurrentResolution = DefaultFusionResolutions[0];

	}
	else if (SelectedItem == ResolutionComboBox->GetOptionAtIndex(1))
	{
		CurrentResolution = DefaultFusionResolutions[1];

	}
	else if (SelectedItem == ResolutionComboBox->GetOptionAtIndex(2))
	{
		CurrentResolution = DefaultFusionResolutions[2];
	}
}

