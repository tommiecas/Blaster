// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/PickupWidgetComponent.h"
#include "Components/TextBlock.h"
#include "Weapons/Weapon.h"
#include "Weapons/WeaponTypes.h"

FString UPickupWidgetComponent::GetWeaponTypeDisplayName(EWeaponType WeaponType)
{
	switch (WeaponType)
	{
	case EWeaponType::EWT_AssaultRifle:
		return FString("Assault Rifle");
	case EWeaponType::EWT_RocketLauncher:
		return FString("Rocket Launcher");
	case EWeaponType::EWT_Pistol:
		return FString("Pistol");
	case EWeaponType::EWT_SubmachineGun:
		return FString("Submachine Gun");
	case EWeaponType::EWT_Shotgun:
		return FString("Shotgun");
	case EWeaponType::EWT_SniperRifle:
		return FString("Sniper Rifle");
	case EWeaponType::EWT_GrenadeLauncher:
		return FString("Grenade Launcher");
		// Add other weapon types here
	default:
		return FString("Unknown Weapon");
	}
}

void UPickupWidgetComponent::SetWeaponNameText(FString WeaponNameTextToDisplay)
{
	if (WeaponNameText)
	{
		WeaponNameText->SetText(FText::FromString(WeaponNameTextToDisplay));
	}
}

void UPickupWidgetComponent::ShowWeaponName(class AWeapon* InWeapon)
{
	if (InWeapon == nullptr || InWeapon->GetWeaponType() == EWeaponType::EWT_None)
	{
		// Handle the null case, maybe log an error or set a default player name
		SetWeaponNameText(FString("Unknown Weapon"));
		return;
	}

	FString WeaponName = GetWeaponTypeDisplayName(InWeapon->GetWeaponType());
	SetWeaponNameText(WeaponName);
}