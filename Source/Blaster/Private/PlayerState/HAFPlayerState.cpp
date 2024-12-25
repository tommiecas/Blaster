// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/HAFPlayerState.h"
#include "Characters/FillainCharacter.h"
#include "PlayerController/FillainPlayerController.h"


void AHAFPlayerState::AddToScore(float ScoreAmount)
{
	Score += ScoreAmount;
	Character = Character == nullptr ? Cast<AFillainCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AFillainPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(ScoreAmount);
		}
	}
}

void AHAFPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AFillainCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller =  Controller == nullptr ? Cast<AFillainPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}
}

