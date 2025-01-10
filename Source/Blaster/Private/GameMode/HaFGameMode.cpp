// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HAFGameMode.h"
#include "Characters/FillainCharacter.h"
#include "PlayerController/FillainPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerState/HAFPlayerState.h"


AHAFGameMode::AHAFGameMode()
{
	bDelayedStart = true;
}

void AHAFGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
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
}

void AHAFGameMode::PlayerEliminated(class AFillainCharacter* VictimCharacter, class AFillainPlayerController* VictimController, AFillainPlayerController* KillerController)
{
	if (KillerController == nullptr || KillerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	AHAFPlayerState* KillerPlayerState = KillerController ? Cast<AHAFPlayerState>(KillerController->PlayerState) : nullptr;
	AHAFPlayerState* VictimPlayerState = VictimController ? Cast<AHAFPlayerState>(VictimController->PlayerState) : nullptr;	

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->AddToScore(1.f);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if (VictimCharacter)
	{
		VictimCharacter->Eliminate();
	}
}

void AHAFGameMode::RequestRespawn(ACharacter* VictimCharacter, AController* VictimController)
{
	if (VictimCharacter)
	{
		VictimCharacter->Reset();
		VictimCharacter->Destroy();
	}
	if (VictimController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(VictimController, PlayerStarts[Selection]);
	}
}



