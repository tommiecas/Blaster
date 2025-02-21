// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/HAFGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/HAFPlayerState.h"

void AHAFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHAFGameState, TopScoringPlayers);
	DOREPLIFETIME(AHAFGameState, RedTeamScore);
	DOREPLIFETIME(AHAFGameState, BlueTeamScore);

}

void AHAFGameState::UpdateTopScore(AHAFPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);

		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void AHAFGameState::OnRep_RedTeamScore()
{
}

void AHAFGameState::OnRep_BlueTeamScore()
{
}
