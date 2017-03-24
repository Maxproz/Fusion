// @Maxpro 2017

#pragma once

#include "Widgets/MasterWidget.h"

#include "Types/TakeHitInfo.h"

#include "ConfirmationDialog_Widget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UConfirmationDialog_Widget : public UMasterWidget
{
	GENERATED_BODY()
	
	
public:

	virtual void NativeConstruct() override;

	/** build menu */
	void FinishConstruct(TWeakObjectPtr<ULocalPlayer> InPlayerOwner, const FText& Message, EFusionDialogType::Type DialogType, TScriptDelegate<FWeakObjectPtr> OkButton, TScriptDelegate<FWeakObjectPtr> CancelButton);

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	//virtual FReply NativeOnFocusReceived(const FGeometry & InGeometry, const FFocusEvent & InFocusEvent) override;

	virtual FReply NativeOnKeyDown(const FGeometry & InGeometry, const FKeyEvent & InKeyEvent) override;

	UFUNCTION()
	void OnClickedOkButton();

	UFUNCTION()
	void OnClickedCancelButton();

	UFUNCTION()
	void OnRep_DisplayMessage();

	UPROPERTY(replicatedUsing = OnRep_DisplayMessage)
	FText DisplayMessage;

	void ExecuteConfirm();

	/** The player that owns the dialog. */
	TWeakObjectPtr<ULocalPlayer> PlayerOwner;

	/* The type of dialog this is */
	EFusionDialogType::Type DialogType;

	virtual bool NativeSupportsKeyboardFocus() const override;

protected:


	UPROPERTY(meta = (BindWidget))
	UButton* MessageCancelButton;

	UPROPERTY(meta = (BindWidget))
	UButton* MessageOkButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageTextBlock;



};
