// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionHUD.h"
#include "FusionPlayerController_Menu.h"

#include "Menu_Pawn.h"


// Sets default values
AMenu_Pawn::AMenu_Pawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMenu_Pawn::BeginPlay()
{
	Super::BeginPlay();
	

	//PlayerHUD = Cast<AFusionPlayerController_Menu>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetFusionHUD();


}

// Called every frame
void AMenu_Pawn::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void AMenu_Pawn::SetupPlayerInputComponent(class UInputComponent* PawnInputComponent)
{
	Super::SetupPlayerInputComponent(PawnInputComponent);

}

