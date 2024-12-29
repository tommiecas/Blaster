// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HaFGameMode.h"
#include "Characters/FillainCharacter.h"
#include "PlayerController/FillainPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerState/HAFPlayerState.h"


void AHaFGameMode::PlayerEliminated(AFillainCharacter* EliminatedCharacter, AFillainPlayerController* VictimController, AFillainPlayerController* KillerController)
{
	AHAFPlayerState* KillerPlayerState = KillerController ? Cast<AHAFPlayerState>(KillerController->PlayerState) : nullptr;
	AHAFPlayerState* VictimPlayerState = VictimController ? Cast<AHAFPlayerState>(VictimController->PlayerState) : nullptr;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->AddToScore(1.f);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(0.5);
	}
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminate();
	}
}

void AHaFGameMode::RequestRespawn(ACharacter* EliminatedPlayerCharacter, AFillainPlayerController* EliminatedPlayerController)
{
	if (EliminatedPlayerCharacter)
	{
		EliminatedPlayerCharacter->Reset();
		EliminatedPlayerCharacter->Destroy();
	}
	if (EliminatedPlayerController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedPlayerController, PlayerStarts[Selection]);
	}
}