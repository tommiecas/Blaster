// Fill out your copyright notice in the Description page of Project Settings.


#include "HAFComponents/BuffComponent.h"
#include "Characters/FillainCHaracter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	 
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bAmIAlreadyHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::ReplenishShield(float ShieldReplenishAmount, float ShieldReplenishingTime)
{
	bAmIAlreadyReplenishingShield = true;
	ShieldReplenishingRate = ShieldReplenishAmount / ShieldReplenishingTime;
	AmountToReplenishShield += ShieldReplenishAmount;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeed, BuffTime, false);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

void UBuffComponent::ResetSpeed()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;

	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float JumpVelocity)
{
	InitialJumpVelocity = JumpVelocity;
}
void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UBuffComponent::ResetJump, BuffTime, false);
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	MulticastJumpBuff(BuffJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float Velocity)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->JumpZVelocity = Velocity;
}

void UBuffComponent::ResetJump()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	MulticastJumpBuff(InitialJumpVelocity);
}
void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bAmIAlreadyHealing || Character == nullptr || Character->IsEliminated()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character -> GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bAmIAlreadyHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bAmIAlreadyReplenishingShield || Character == nullptr || Character->IsEliminated()) return;

	const float ReplenishShieldThisFrame = ShieldReplenishingRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishShieldThisFrame, 0.f, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	AmountToReplenishShield -= ReplenishShieldThisFrame;

	if (AmountToReplenishShield <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bAmIAlreadyReplenishingShield = false;
		AmountToReplenishShield = 0.f;
	}
}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}



void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

