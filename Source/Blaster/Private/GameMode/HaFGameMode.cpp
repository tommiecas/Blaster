// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HaFGameMode.h"
#include "Characters/FillainCharacter.h"
#include "PlayerController/FillainPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerState/HAFPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AHAFGameMode::AHAFGameMode()
{
	bDelayedStart = true;
}

void AHAFGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
}

void AHAFGameMode::PlayerEliminated(AFillainCharacter* EliminatedCharacter, AFillainPlayerController* VictimController, AFillainPlayerController* KillerController)
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

void AHAFGameMode::RequestRespawn(ACharacter* EliminatedPlayerCharacter, AFillainPlayerController* EliminatedPlayerController)
{
	if (EliminatedPlayerCharacter)
	{
		EliminatedPlayerCharacter->Reset();
		EliminatedPlayerCharacter->Destroy();
	}
	if (EliminatedPlayerController)
	{
		AHAFPlayerState* VictimPlayerState = EliminatedPlayerController ? Cast<AHAFPlayerState>(EliminatedPlayerController->PlayerState) : nullptr;
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedPlayerController, PlayerStarts[Selection]);
	}
}

void AHAFGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AHAFGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFillainPlayerController* FillainPlayer = Cast<AFillainPlayerController>(*It);
		if (FillainPlayer)
		{
			FillainPlayer->OnMatchStateSet(MatchState);
		}
	}


}
