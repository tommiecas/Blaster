// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/FillainCharacter.h"

// Sets default values
AFillainCharacter::AFillainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFillainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFillainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFillainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

