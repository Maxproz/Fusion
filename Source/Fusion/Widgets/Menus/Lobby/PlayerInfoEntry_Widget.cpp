// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionPlayerController_Lobby.h"

#include "Runtime/Engine/Classes/Kismet/KismetStringLibrary.h"
//#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetTextLibrary.h"

#include "PlayerInfoEntry_Widget.h"



void UPlayerInfoEntry_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerNameTextBlock->SetText(FText::FromString(PlayerLobbyInfo.PlayerName));
	

	IdsReadyCheckBox->OnCheckStateChanged.AddDynamic(this, &UPlayerInfoEntry_Widget::OnCheckStateChangedIdsReadyCheckBox);
	KickButton->OnClicked.AddDynamic(this, &UPlayerInfoEntry_Widget::OnClickedKickButton);
}



void UPlayerInfoEntry_Widget::OnCheckStateChangedIdsReadyCheckBox(bool IsChecked)
{

}

void UPlayerInfoEntry_Widget::OnClickedKickButton()
{
	// makes sure only the host can kick other players and that the host doesent accidently kick himself as well
	bool bIsValidKick = (UKismetSystemLibrary::IsServer(GetWorld()) && PlayerIndex > 0);

	if (bIsValidKick)
	{
		// calls the PlayerController to call the server and kick the selected players
		AFusionPlayerController_Lobby* LPC = Cast<AFusionPlayerController_Lobby>(GetOwningPlayer());
		
		if (LPC)
		{
			LPC->KickPlayer(PlayerIndex);
		}
	}
}

