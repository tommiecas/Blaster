// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Weapons/WeaponTypes.h"
#include "PickupWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UPickupWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WeaponNameText;

	class FString GetWeaponTypeDisplayName(EWeaponType WeaponType);

	void SetWeaponNameText(FString WeaponNameTextToDisplay);

	UFUNCTION(BlueprintCallable)
	void ShowWeaponName(class AWeapon* InWeapon);

	
};
