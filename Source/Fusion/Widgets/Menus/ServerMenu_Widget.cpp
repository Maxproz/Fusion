// @Maxpro 2017

#include "Fusion.h"

#include "Runtime/Engine/Classes/Kismet/KismetArrayLibrary.h"

#include "FusionHUD.h"
#include "ServerMenuStats_Widget.h"

#include "FusionPlayerController_Lobby.h"

#include "Runtime/Engine/Classes/Kismet/KismetTextLibrary.h"

#include "ServerMenu_Widget.h"



void UServerMenu_Widget::NativeConstruct()
{
	Super::NativeConstruct();



	SessionSearchCompleatedEvent.AddUObject(this, &UServerMenu_Widget::OnSessionSearchCompleated);

	LanPlayerNameTextbox->OnTextChanged.AddDynamic(this, &UServerMenu_Widget::OnTextChangedLanPlayerNameTextbox);
	LanPlayerNameTextbox->OnTextCommitted.AddDynamic(this, &UServerMenu_Widget::OnTextCommittedLanPlayerNameTextbox);

	IsLanButton->OnClicked.AddDynamic(this, &UServerMenu_Widget::OnClickedIsLanButton);
	SearchButton->OnClicked.AddDynamic(this, &UServerMenu_Widget::OnClickedSearchButton);
	BackButton->OnClicked.AddDynamic(this, &UServerMenu_Widget::OnClickedBackButton);
	BackButton2->OnClicked.AddDynamic(this, &UServerMenu_Widget::OnClickedBackButton2);
	RefreshButton->OnClicked.AddDynamic(this, &UServerMenu_Widget::OnClickedRefreshButton);
}

void UServerMenu_Widget::OnSessionSearchCompleated(TArray<FCustomBlueprintSessionResult> Results)
{
	ServerListScrollBox->ClearChildren();
	CustomSessionResults.Empty();

	CustomSessionResults = Results;

	if (CustomSessionResults.Num() > 0)
	{
		AFusionPlayerController_Lobby* LPC = Cast<AFusionPlayerController_Lobby>(GetOwningPlayer());

		int32 CurrentIndex = 0;
		for (const auto& Session : CustomSessionResults)
		{
			UServerMenuStats_Widget* ServerMenuStats_Widget = CreateWidget<UServerMenuStats_Widget>(LPC, LPC->GetFusionHUD()->ServerMenuStats_WidgetTemplate);
			ServerMenuStats_Widget->SetIndex(CurrentIndex);
			ServerMenuStats_Widget->SetCustomResult(Session);
			ServerMenuStats_Widget->SetGameInstanceRef(GameInstanceRef);

			ServerListScrollBox->AddChild(ServerMenuStats_Widget);

			CurrentIndex = CurrentIndex + 1;
		}

		ServerListWidgetSwitcher->SetActiveWidgetIndex(1);
	}
	else
	{
		// Show Error Message
		ServerListWidgetSwitcher->SetActiveWidgetIndex(2);
	}

}

// if the name is not empty set the lan player name in game instance
void UServerMenu_Widget::OnTextChangedLanPlayerNameTextbox(const FText &Text)
{
	FText EmptyText = FText::FromString(TEXT(""));

	if (UKismetTextLibrary::NotEqual_TextText(Text, EmptyText))
	{
		GameInstanceRef->LanPlayerName = UKismetTextLibrary::Conv_TextToString(Text);
	}
}


void UServerMenu_Widget::OnTextCommittedLanPlayerNameTextbox(const FText &Text, ETextCommit::Type Method)
{
	FText EmptyText = FText::FromString(TEXT(""));

	if (UKismetTextLibrary::NotEqual_TextText(Text, EmptyText) && Method == ETextCommit::OnEnter)
	{
		GameInstanceRef->LanPlayerName = UKismetTextLibrary::Conv_TextToString(Text);
	}
}

void UServerMenu_Widget::OnClickedIsLanButton()
{
	if (bIsLan)
	{
		bIsLan = false;
		IsLanButtonText->SetText(FText::FromString(TEXT("Online")));
	}
	else
	{
		bIsLan = true;
		IsLanButtonText->SetText(FText::FromString(TEXT("Lan")));
	}
}

// make the widget switcher open the server list then call the game instance to search for online games
void UServerMenu_Widget::OnClickedSearchButton()
{
	SessionWidgetSwitcher->SetActiveWidgetIndex(1);
	GameInstanceRef->FindOnlineGames(bIsLan, true);
}

// call the Game instance to show main menu and remove this from viewport
void UServerMenu_Widget::OnClickedBackButton()
{
	//GameInstanceRef->ShowMainMenu();
	RemoveFromParent();
}

// set the widget switcher back to default when back is pressed
void UServerMenu_Widget::OnClickedBackButton2()
{
	ServerListWidgetSwitcher->SetActiveWidgetIndex(0);
	SessionWidgetSwitcher->SetActiveWidgetIndex(0);
}

// set the widget switcher back to search and call the Game Instance to search for sessions again
void UServerMenu_Widget::OnClickedRefreshButton()
{
	ServerListWidgetSwitcher->SetActiveWidgetIndex(0);
	GameInstanceRef->FindOnlineGames(bIsLan, true);
}