// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/SeizeTheSwordGameMode.h"
#include "Weapons/Sword.h"
#include "SeizeTheSword/SwordZone.h"
#include "GameStates/HAFGameState.h"
#include "Kismet/GameplayStatics.h"

void ASeizeTheSwordGameMode::PlayerEliminated(AFillainCharacter* VictimCharacter, AFillainPlayerController* VictimController, AFillainPlayerController* KillerController)
{
	AHAFGameMode::PlayerEliminated(VictimCharacter, VictimController, KillerController);
}

void ASeizeTheSwordGameMode::SwordSeized(ASword* Sword, ASwordZone* Zone)
{
	bool bValidSeizure = Sword->GetTeam() != Zone->Team;
	AHAFGameState* BGameState = Cast<AHAFGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}
