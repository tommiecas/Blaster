// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/FillainAnimInstance.h"
#include "Characters/FillainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/Weapon.h"


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
	EquippedWeapon = FillainCharacter->GetEquippedWeapon();
	bIsCrouched = FillainCharacter->bIsCrouched;
	bAiming = FillainCharacter->IsAiming();
	TurningInPlace = FillainCharacter->GetTurningInPlace();

	//Offset Yaw for Strafing
	FRotator AimRotation = FillainCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(FillainCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = FillainCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = FillainCharacter->GetAO_Yaw();
	AO_Pitch = FillainCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && FillainCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		FillainCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}
}
