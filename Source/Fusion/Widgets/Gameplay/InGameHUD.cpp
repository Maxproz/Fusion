// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionCharacter.h"
#include "FusionPlayerController.h"

#include "InGameHUD.h"



void UInGameHUD::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerControllerRef = Cast<AFusionPlayerController>(GetOwningPlayer());


}



void UInGameHUD::UpdateDynamicInfo(const float InDeltaTime, float Shields, float Health)
{

	//ASSERTV(HasValidData() == true, TEXT("Info Widget Not Initialized"));

	//const AGESGame_Satellite* SatContext = CurrentContextItem->GetAssignedSatellite();
	//ASSERTV(SatContext != nullptr, TEXT("Invalid Satellite"));

	// Interpolate values for nicer transition effect
	//BattProgBar_Ptr->SetPercent(FMath::FInterpTo(BattProgBar_Ptr->Percent, SatContext->SatData.Battery_CurrentChargeRatio, InDeltaTime, ValueInterpSpeed));
	//ShieldBar->SetPercent(FMath::FInterpTo(ShieldBar->Percent, Shields, InDeltaTime, 1.f));
	//HealthBar->SetPercent(FMath::FInterpTo(HealthBar->Percent, Health, InDeltaTime, 1.f));
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

float UInGameHUD::UpdatePlayerHealthBar() const
{
	if (PlayerControllerRef)
	{
		if (PlayerControllerRef->GetControlledPawn())
		{
			return Cast<AFusionCharacter>(PlayerControllerRef->GetControlledPawn())->GetHealth() / Cast<AFusionCharacter>(PlayerControllerRef->GetControlledPawn())->GetMaxHealth();
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		return 0.0f;
	}
}

float UInGameHUD::UpdatePlayerShieldBar() const
{
	if (PlayerControllerRef)
	{
		if (PlayerControllerRef->GetPawn())
		{
			return Cast<AFusionCharacter>(PlayerControllerRef->GetControlledPawn())->GetShields() / Cast<AFusionCharacter>(PlayerControllerRef->GetControlledPawn())->GetMaxShields(); 
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		return 0.0f;
	}
}
