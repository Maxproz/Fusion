// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/MasterWidget.h"
#include "SessionInviteRecieved_Widget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API USessionInviteRecieved_Widget : public UMasterWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;


	UFUNCTION()
	void OnClickedAcceptInviteButton();

	UFUNCTION()
	void OnClickedCancelinviteButton();

protected:

	UPROPERTY(meta = (BindWidget))
	UButton* AcceptInviteButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CancelinviteButton;


	class AFusionGameInstance* GameInstanceRef;

};