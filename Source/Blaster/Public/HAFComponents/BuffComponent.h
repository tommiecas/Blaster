// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class AFillainCharacter;
	void Heal(float HealAmount, float HealingTime);
	void ReplenishShield(float ShieldReplenishAmount, float ShieldReplenishingTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void BuffJump(float BuffJumpVelocity, float BuffTime);
	void ResetSpeed();
	void ResetJump();
	void SetInitialJumpVelocity(float JumpVelocity);	
	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);

protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);

private:
	UPROPERTY()
	class AFillainCharacter* Character;

	/***********************
	***   HEALING BUFF   ***
	***********************/
	bool bAmIAlreadyHealing = false;
	float HealingRate = 0;
	float AmountToHeal = 0.f;

	/***********************************
	***   REPLENISHING SHIELD BUFF   ***
	***********************************/
	bool bAmIAlreadyReplenishingShield = false;
	float ShieldReplenishingRate = 0;
	float AmountToReplenishShield = 0.f;

	/*********************
	***   SPEED BUFF   ***
	*********************/
	FTimerHandle SpeedBuffTimer;
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	/********************
	***   JUMP BUFF   ***
	********************/
	FTimerHandle JumpBuffTimer;
	float InitialJumpVelocity;
		
};
