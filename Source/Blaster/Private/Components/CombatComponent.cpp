// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CombatComponent.h"
#include "Weapons/Weapon.h"
#include "Characters/FillainCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bIsFireButtonPressed = bPressed;

	if (bIsFireButtonPressed)
	{
		ServerFire();
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			HitTarget = End;
		}
		else
		{
			HitTarget = TraceHitResult.ImpactPoint;
			DrawDebugSphere(
				GetWorld(),
				TraceHitResult.ImpactPoint,
				12.f,
				12,
				FColor::Red
			);
		}
	}
}



void UCombatComponent::ServerFire_Implementation()
{
	MulticastFire();
}

void UCombatComponent::MulticastFire_Implementation()
{
	if (EquippedWeapon == nullptr) return;
	if (PlayerCharacter)
	{
		PlayerCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(HitTarget);
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		PlayerCharacter->bUseControllerRotationYaw = true;
	}
}










void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);

}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (PlayerCharacter == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = PlayerCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, PlayerCharacter->GetMesh());
	}
	EquippedWeapon->SetOwner(PlayerCharacter);
	PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	PlayerCharacter->bUseControllerRotationYaw = true;

}

