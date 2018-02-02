// @Maxpro 2017

#pragma once

#include "GameFramework/Actor.h"
#include "TimerActor.generated.h"

UCLASS()
class FUSION_API ATimerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATimerActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(EditAnywhere)
	float LoopTime;
	
	void DoAPeriodicCheck();

};
