// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MasterPickupActor.generated.h"

UCLASS(ABSTRACT)
class FUSION_API AMasterPickupActor : public AActor
{
	GENERATED_BODY()
	
protected:

	AMasterPickupActor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* MeshComp;

public:

	/* Player is looking at */
	virtual void OnBeginFocus();

	/* Player is no longer looking at */
	virtual void OnEndFocus();

	/* Called when player interacts with object */
	virtual void OnUsed(APawn* InstigatorPawn);

	/* Public accessor to the mesh component. With FORCEINLINE we are allowed to define the function in the header, use this only for simple accessors! */
	FORCEINLINE UStaticMeshComponent* GetMeshComponent() const { return MeshComp; }
	
};
