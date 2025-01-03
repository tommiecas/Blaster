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
	void AddWeaponTypeText(FString WeaponTypeText);

	UFUNCTION()
	virtual void OnRep_Defeats();


private:
	UPROPERTY()
	class AFillainCharacter* PlayerCharacter;
	
	UPROPERTY()
	class AFillainPlayerController* PlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats);
	float Defeats;

};
