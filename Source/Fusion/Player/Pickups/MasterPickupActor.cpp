// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"
#include "MasterPickupActor.h"


AMasterPickupActor::AMasterPickupActor(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MeshComp = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Mesh"));
	RootComponent = MeshComp;

}


void AMasterPickupActor::OnUsed(APawn* InstigatorPawn)
{
	// Nothing to do here...
}


void AMasterPickupActor::OnBeginFocus()
{
	// Used by custom PostProcess to render outlines
	MeshComp->SetRenderCustomDepth(true);
}


void AMasterPickupActor::OnEndFocus()
{
	// Used by custom PostProcess to render outlines
	MeshComp->SetRenderCustomDepth(false);
}

