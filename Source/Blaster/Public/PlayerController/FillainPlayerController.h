// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FillainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AFillainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDEliminationText(FString EliminationText);
	void SetHUDWeaponType(FString WeaponTypeText);
	void SetHUDMatchCountdown(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

private:
	UPROPERTY()
	class AFillainHUD* FillainHUD;

	float MatchTime = 120.f;
	uint32 CountdownInt = 0.f;
	
};
