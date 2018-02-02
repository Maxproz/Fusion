// @Maxpro 2017

#include "Fusion.h"
#include "TimerActor.h"


// Sets default values
ATimerActor::ATimerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATimerActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Timer handler
	FTimerHandle TimerHandle;

	//Activate the Timer
	//1st Parameter: TimerHandle
	//2nd Parameter: The Object related to this timer
	//3rd Parameter: The function that is going to be fired
	//4th Parameter: The loop time
	//5th Parameter: True - if you want this timer to loop, false otherwise
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ATimerActor::DoAPeriodicCheck, LoopTime, true);

}

// Called every frame
void ATimerActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void ATimerActor::DoAPeriodicCheck()
{
	GLog->Log("Periodic Check has fired!");

}
