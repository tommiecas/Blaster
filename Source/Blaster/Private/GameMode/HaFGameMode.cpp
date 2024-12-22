// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HaFGameMode.h"
#include "Characters/FillainCharacter.h"
#include "PlayerController/FillainPlayerController.h"

void AHaFGameMode::PlayerEliminated(AFillainCharacter* EliminatedCharacter, AFillainPlayerController* VictimController, AFillainPlayerController* KillerController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminate();
	}
}
