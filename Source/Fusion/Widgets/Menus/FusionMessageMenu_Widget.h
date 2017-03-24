// @Maxpro 2017

#pragma once

#include "Widgets/MasterWidget.h"
#include "FusionMessageMenu_Widget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UFusionMessageMenu_Widget : public UMasterWidget
{
	GENERATED_BODY()


public:

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedCancelButton();

	UFUNCTION()
	void OnClickedOkButton();

	UFUNCTION()
	void OnRep_DisplayMessage();

	UPROPERTY(replicatedUsing = OnRep_DisplayMessage)
	FText DisplayMessage;

	void UpdateDisplayMessage(FText NewDisplayMessage);

protected:


	UPROPERTY(meta = (BindWidget))
	UButton* MessageCancelButton;

	UPROPERTY(meta = (BindWidget))
	UButton* MessageOkButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageTextBlock;


private:

	/** Owning game instance */
	TWeakObjectPtr<class UFusionGameInstance> GameInstance;

};


	

