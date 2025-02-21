// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HAFGameMode.h"
#include "Characters/FillainCharacter.h"
#include "PlayerController/FillainPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerState/HAFPlayerState.h"
#include "GameStates/HAFGameState.h"

namespace MatchState
{
	const FName Cooldown = FName(TEXT("Cooldown"));
}


AHAFGameMode::AHAFGameMode()
{
	bDelayedStart = false;
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
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void AHAFGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFillainPlayerController* FillainPlayer = Cast<AFillainPlayerController>(*It);
		if (FillainPlayer)
		{
			FillainPlayer->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

float AHAFGameMode::CalculateDamage(AController* Killer, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

void AHAFGameMode::PlayerEliminated(class AFillainCharacter* VictimCharacter, class AFillainPlayerController* VictimController, AFillainPlayerController* KillerController)
{
	if (KillerController == nullptr || KillerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	AHAFPlayerState* KillerPlayerState = KillerController ? Cast<AHAFPlayerState>(KillerController->PlayerState) : nullptr;
	AHAFPlayerState* VictimPlayerState = VictimController ? Cast<AHAFPlayerState>(VictimController->PlayerState) : nullptr;

	AHAFGameState* HAFGameState = GetGameState<AHAFGameState>();
	if (KillerPlayerState && KillerPlayerState != VictimPlayerState && HAFGameState)
	{
		TArray<AHAFPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : HAFGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		KillerPlayerState->AddToScore(1.f);
		HAFGameState->UpdateTopScore(KillerPlayerState);

		if (HAFGameState->TopScoringPlayers.Contains(KillerPlayerState))
		{
			AFillainCharacter* Leader = Cast<AFillainCharacter>(KillerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!HAFGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				AFillainCharacter* Loser = Cast<AFillainCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
		if (VictimPlayerState)
		{
			VictimPlayerState->AddToDefeats(1);
		}
		if (VictimCharacter)
		{
			VictimCharacter->Eliminate(false);
		}
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AFillainPlayerController* FillainPlayer = Cast<AFillainPlayerController>(*It);
			if (FillainPlayer && KillerPlayerState && VictimPlayerState)
			{
				FillainPlayer->BroadcastElimination(KillerPlayerState, VictimPlayerState);
			}
		}
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

void AHAFGameMode::PlayerLeftGame(AHAFPlayerState* LeavingPlayer)
{
	if (LeavingPlayer == nullptr) return;
	AHAFGameState* HAFGameState = GetGameState<AHAFGameState>();
	if (HAFGameState && HAFGameState->TopScoringPlayers.Contains(LeavingPlayer))
	{
		HAFGameState->TopScoringPlayers.Remove(LeavingPlayer);
	}
	AFillainCharacter* CharacterLeaving = Cast<AFillainCharacter>(LeavingPlayer->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Eliminate(true);
	}
}

void AHAFGameMode::SendChat(const FString& Text, const FString& PlayerName)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFillainPlayerController* FillainPlayerController = Cast<AFillainPlayerController>(*It);
		if (FillainPlayerController)
		{
			FillainPlayerController->ClientSetText(Text, PlayerName);
		}
	}
}


