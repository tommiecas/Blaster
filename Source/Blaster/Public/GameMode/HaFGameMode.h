// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HaFGameMode.generated.h"

namespace MatchState
{
	extern BLASTER_API const FName Cooldown; // Match Duration has been reached. Display winner and begin cooldown timer
}

/**
 * 
 */
UCLASS()
class BLASTER_API AHAFGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AHAFGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class AFillainCharacter* EliminatedCharacter, class AFillainPlayerController* VictimController, AFillainPlayerController* KillerController);
	virtual void RequestRespawn(class ACharacter* EliminatedPlayerCharacter, class AFillainPlayerController* EliminatedPlayerController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

protected:

	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }

};
