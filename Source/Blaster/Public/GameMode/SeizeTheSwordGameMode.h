// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/TeamsGameMode.h"
#include "SeizeTheSwordGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ASeizeTheSwordGameMode : public ATeamsGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class AFillainCharacter* VictimCharacter, class AFillainPlayerController* VictimController, AFillainPlayerController* KillerController) override;
	void SwordSeized(class ASword* Sword, class ASwordZone* Zone);

	
};
