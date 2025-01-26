// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BuffComponent.h"
#include "Characters/FillainCHaracter.h"

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


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}



void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
}

