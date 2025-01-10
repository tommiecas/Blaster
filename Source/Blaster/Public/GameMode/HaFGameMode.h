// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HAFGameMode.generated.h"

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
	void PlayerEliminated(class AFillainCharacter* VictimCharacter, class AFillainPlayerController* VictimController, AFillainPlayerController* KillerController);
	virtual void RequestRespawn(ACharacter* VictimCharacter, AController* VictimController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	AFillainCharacter* Character;

	float CountdownTime = 0.f;

public:
	class AFillainCharacter* GetCharacter() const { return Character; }

};
