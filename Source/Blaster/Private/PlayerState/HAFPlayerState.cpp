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
	SetScore(GetScore() + ScoreAmount);
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AFillainCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<AFillainPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDScore(GetScore());
		}
	}
}

void AHAFPlayerState::AddToDefeats(float DefeatsAmount)
{
	Defeats += DefeatsAmount;
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AFillainCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<AFillainPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDDefeats(Defeats);
		}
	}
}

void AHAFPlayerState::AddWeaponTypeText(FString WeaponTypeText)
{
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AFillainCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<AFillainPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDWeaponType(WeaponTypeText);
		}
	}
}

void AHAFPlayerState::OnRep_Defeats()
{
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AFillainCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<AFillainPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDDefeats(Defeats);
		}
	}
}


void AHAFPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AFillainCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<AFillainPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDScore(GetScore());
		}
	}
}

