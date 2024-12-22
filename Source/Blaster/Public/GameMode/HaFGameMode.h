// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HaFGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHaFGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class AFillainCharacter* EliminatedCharacter, class AFillainPlayerController* VictimController, AFillainPlayerController* KillerController);
	
};
