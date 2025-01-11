// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "HAFGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHAFGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class AHAFPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<AHAFPlayerState*> TopScoringPlayers;

private:
	float TopScore = 0.f;
	
};
