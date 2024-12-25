// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/HAFPlayerState.h"
#include "Characters/FillainCharacter.h"
#include "PlayerController/FillainPlayerController.h"
#include "Net/UnrealNetwork.h"


void AHAFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHAFPlayerState, Defeats);
}

void AHAFPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(Score + ScoreAmount);
	Character = Character == nullptr ? Cast<AFillainCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AFillainPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AHAFPlayerState::AddToDefeats(float DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AFillainCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AFillainPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AHAFPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AFillainCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AFillainPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
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
			Controller->SetHUDScore(GetScore());
		}
	}
}

