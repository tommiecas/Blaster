// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HAFPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHAFPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;
	
	/*
	** Replication Notifies
	*/
	void AddToScore(float ScoreAmount);
	void AddToDefeats(float DefeatsAmount);

	UFUNCTION()
	virtual void OnRep_Defeats();

protected:

private:
	UPROPERTY(meta = (AllowPrvateAccess = "true"))
	class AFillainCharacter* Character;
	
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	class AFillainPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats);
	int32 Defeats;

public:
	AFillainCharacter* GetFillainPlayerCharacter() const { return Character; }
	AFillainPlayerController* GetFillainPlayerController() const { return Controller; }
	float GetScore(float InScore) const { return Score; }
	int32 GetDefeats(int32 InDefeats) const { return Defeats; }

};
