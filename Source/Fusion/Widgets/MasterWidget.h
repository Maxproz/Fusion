// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "MasterWidget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UMasterWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void AssignAnimations();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Anims)
	TArray<UWidgetAnimation*> WidgetAnimations;
	
	void ShowWidget()
	{
		SetVisibility(ESlateVisibility::Visible);
	}

	void HideWidget()
	{
		SetVisibility(ESlateVisibility::Hidden);
	}

};
