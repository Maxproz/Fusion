// @Maxpro 2017

#pragma once

#include "Widgets/MasterWidget.h"
#include "MainMenuOptions_Widget.generated.h"


/**
 * 
 */
UCLASS()
class FUSION_API UMainMenuOptions_Widget : public UMasterWidget
{
	GENERATED_BODY()
		
public:

	/** supported resolutions */
	FIntPoint DefaultFusionResolutions[3]; 


	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedApplyChangesButton();

	UFUNCTION()
	void OnClickedFullScreenToggleButton();

	UFUNCTION()
	void OnValueChangedGammaSpinBox(const float Value);

	UFUNCTION()
	void OnValueCommittedGammaSpinBox(const float Value, ETextCommit::Type Method);

	UFUNCTION()
	void OnSelectionChangedQualityComboBox(const FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSelectionChangedResolutionComboBox(const FString SelectedItem, ESelectInfo::Type SelectionType);

	FIntPoint CurrentResolution;

	float CurrentGamma;

	EWindowMode::Type CurrentWindowMode;

	bool bIsCurrentlyFullScreen;

	int32 GraphicsQuality;

	/** Get the persistence user associated with PlayerOwner*/
	class UFusionPersistentUser* GetPersistentUser() const;

	/** Owning player controller */
	ULocalPlayer* PlayerOwner;

	class AFusionHUD* PlayerHUDRef;

protected:

	UPROPERTY(meta = (BindWidget))
	UButton* ApplyChangesButton;

	UPROPERTY(meta = (BindWidget))
	UButton* FullScreenToggleButton;
	
	UPROPERTY(meta = (BindWidget))
	USpinBox* GammaSpinBox;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* QualityComboBox;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ResolutionComboBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* OnOffTextBlock;

	/** User settings pointer */
	class UFusionGameUserSettings* UserSettings;


};
