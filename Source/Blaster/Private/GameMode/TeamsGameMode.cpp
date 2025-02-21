// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TeamsGameMode.h"
#include "GameStates/HAFGameState.h"
#include "PlayerState/HAFPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/FillainPLayerController.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AHAFGameState* BGameState = Cast<AHAFGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		AHAFPlayerState* BPState = NewPlayer->GetPlayerState<AHAFPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	AHAFGameState* BGameState = Cast<AHAFGameState>(UGameplayStatics::GetGameState(this));
	AHAFPlayerState* BPState = Exiting->GetPlayerState<AHAFPlayerState>();
	if (BGameState && BPState)
	{
		if (BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}
		if (BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AHAFGameState* BGameState = Cast<AHAFGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		for (auto PState : BGameState->PlayerArray)
		{
			AHAFPlayerState* BPState = Cast<AHAFPlayerState>(PState.Get());
			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController * Killer, AController * Victim, float BaseDamage)
{
	AHAFPlayerState* KillerPState = Killer->GetPlayerState<AHAFPlayerState>();
	AHAFPlayerState* VictimPState = Victim->GetPlayerState<AHAFPlayerState>();
	if (KillerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (VictimPState == KillerPState)
	{
		return BaseDamage;
	}
	if (KillerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(AFillainCharacter* VictimCharacter, AFillainPlayerController* VictimController, AFillainPlayerController* KillerController)
{
	Super::PlayerEliminated(VictimCharacter, VictimController, KillerController);

	AHAFGameState* BGameState = Cast<AHAFGameState>(UGameplayStatics::GetGameState(this));
	AHAFPlayerState* KillerPlayerState = KillerController ? Cast<AHAFPlayerState>(KillerController->PlayerState) : nullptr;
	if (BGameState && KillerPlayerState)
	{
		if (KillerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		if (KillerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}
