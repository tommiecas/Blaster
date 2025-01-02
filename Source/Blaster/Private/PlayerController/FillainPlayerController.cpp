// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/FillainPlayerController.h"
#include "HUD/FillainHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Characters/FillainCharacter.h"



void AFillainPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FillainHUD = Cast <AFillainHUD>(GetHUD());
}

void AFillainPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetWorld()->GetTimeSeconds());
	if (CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetWorld()->GetTimeSeconds());
	}
	CountdownInt = SecondsLeft;
}

void AFillainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AFillainCharacter* FillainCharacter = Cast<AFillainCharacter>(InPawn);
	if (FillainCharacter)
	{
		SetHUDHealth(FillainCharacter->GetHealth(), FillainCharacter->GetMaxHealth());
	}
}

void AFillainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
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

void AFillainPlayerController::SetHUDScore(float Score)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->ScoreAmount;
	if (bIsHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		FillainHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}

}

void AFillainPlayerController::SetHUDDefeats(int32 Defeats)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->DefeatsAmount;
	if (bIsHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		FillainHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}

void AFillainPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bIsHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		FillainHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AFillainPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bIsHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		FillainHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AFillainPlayerController::SetHUDEliminationText(FString EliminationText)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->EliminationText;
	if (bIsHUDValid)
	{
		FillainHUD->CharacterOverlay->EliminationText->SetText(FText::FromString(EliminationText));
	}
}

void AFillainPlayerController::SetHUDWeaponType(FString WeaponTypeText)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->WeaponType;
	if (bIsHUDValid)
	{
		FillainHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(WeaponTypeText));
	}
}

void AFillainPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->MatchCountdownText;
	if (bIsHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		FillainHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}
