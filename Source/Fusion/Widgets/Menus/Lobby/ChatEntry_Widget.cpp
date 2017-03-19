// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "Runtime/Engine/Classes/Kismet/KismetStringLibrary.h"
//#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetTextLibrary.h"

#include "ChatEntry_Widget.h"




void UChatEntry_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	FText OutPlayerNameText;
	FText OutChatMessageText;

	SplitPlayerNameAndChatMessage(OutPlayerNameText, OutChatMessageText);

	PlayerNameTextBlock->SetText(OutPlayerNameText);
	ChatEntryTextblock->SetText(OutChatMessageText);
}

void UChatEntry_Widget::SplitPlayerNameAndChatMessage(FText& OutPlayerName, FText& OutChatMessage)
{
	FString ChatEntryTextString = UKismetTextLibrary::Conv_TextToString(ChatEntryText);
	
	// Get where is the : and split the string there
	int32 SubStrIndex = UKismetStringLibrary::FindSubstring(ChatEntryTextString, ":", false, false, -1);
	SubStrIndex = SubStrIndex + 1;

	// player name is the one before the colin
	FString PlayerName = UKismetStringLibrary::GetSubstring(ChatEntryTextString, 0, SubStrIndex);
	FText PlayerNameText = UKismetTextLibrary::Conv_StringToText(PlayerName);
	OutPlayerName = PlayerNameText;

	// Chat Message is the one after the colin
	FString ChatMsg = UKismetStringLibrary::GetSubstring(ChatEntryTextString, SubStrIndex, UKismetStringLibrary::Len(ChatEntryTextString));
	FText ChatMsgText = UKismetTextLibrary::Conv_StringToText(ChatMsg);
	OutChatMessage = ChatMsgText;
}