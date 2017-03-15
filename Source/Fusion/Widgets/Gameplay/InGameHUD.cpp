// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"
#include "InGameHUD.h"


void UInGameHUD::UpdateDynamicInfo(const float InDeltaTime)
{

	//ASSERTV(HasValidData() == true, TEXT("Info Widget Not Initialized"));

	//const AGESGame_Satellite* SatContext = CurrentContextItem->GetAssignedSatellite();
	//ASSERTV(SatContext != nullptr, TEXT("Invalid Satellite"));

	// Interpolate values for nicer transition effect
	//BattProgBar_Ptr->SetPercent(FMath::FInterpTo(BattProgBar_Ptr->Percent, SatContext->SatData.Battery_CurrentChargeRatio, InDeltaTime, ValueInterpSpeed));

	//HealthBar->SetPercent(FMath::FInterpTo(HealthBar->Percent, 2.5f, InDeltaTime, 2.5f));
	//PlayerContext.GetPawn()
}

void UInGameHUD::ShowInGameHUD()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UInGameHUD::HideInGameHUD()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UInGameHUD::UpdatePlayerHealthBar(float Health)
{
	HealthBar->SetPercent(Health);
}