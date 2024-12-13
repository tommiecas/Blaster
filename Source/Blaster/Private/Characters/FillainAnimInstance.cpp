// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/FillainAnimInstance.h"
#include "Characters/FillainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UFillainAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	FillainCharacter = Cast<AFillainCharacter>(TryGetPawnOwner());	
}

void UFillainAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (FillainCharacter == nullptr)
	{
		FillainCharacter = Cast<AFillainCharacter>(TryGetPawnOwner());
	}
	if (FillainCharacter == nullptr) return;

	FVector Velocity = FillainCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = FillainCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = FillainCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = FillainCharacter->IsWeaponEquipped();
	bIsCrouched = FillainCharacter->bIsCrouched;
}
