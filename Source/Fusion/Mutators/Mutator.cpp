// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"
#include "Mutator.h"


bool AMutator::CheckRelevance_Implementation(AActor* Other)
{
	if (NextMutator)
	{
		return NextMutator->CheckRelevance(Other);
	}

	return true;
}


void AMutator::InitGame_Implementation(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	if (NextMutator)
	{
		NextMutator->InitGame(MapName, Options, ErrorMessage);
	}
}

