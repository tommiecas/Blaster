// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/FillainPlayerController.h"
#include "HUD/FillainHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"



void AFillainPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FillainHUD = Cast <AFillainHUD>(GetHUD());
}

void AFillainPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;

	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->HealthBar && FillainHUD->CharacterOverlay->HealthText;
	if (bIsHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		FillainHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		FillainHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}
