// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HAFGameMode.generated.h"

namespace MatchState
{
	extern BLASTER_API const FName Cooldown;
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
	virtual void PlayerEliminated(class AFillainCharacter* VictimCharacter, class AFillainPlayerController* VictimController, AFillainPlayerController* KillerController);
	virtual void RequestRespawn(ACharacter* VictimCharacter, AController* VictimController);
	void PlayerLeftGame(class AHAFPlayerState* LeavingPlayer);
	virtual float CalculateDamage(AController* Killer, AController* Victim, float BaseDamage);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float LevelStartingTime = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	void SendChat(const FString& Text, const FString& PlayerName);

	bool bTeamsMatch = false;


protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	UPROPERTY()
	AFillainCharacter* Character;

	float CountdownTime = 0.f;

public:
	class AFillainCharacter* GetCharacter() const { return Character; }
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }

};
