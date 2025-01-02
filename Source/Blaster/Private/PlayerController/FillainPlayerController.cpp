// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/FillainPlayerController.h"
#include "HUD/FillainHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Characters/FillainCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Public/GameMode/HAFGameMode.h"
#include "HUD/Announcement.h"


void AFillainPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FillainHUD = Cast <AFillainHUD>(GetHUD());
	if (FillainHUD)
	{
		FillainHUD->AddAnnouncement();
	}
}

void AFillainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}

void AFillainPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFillainPlayerController, MatchState);
}

void AFillainPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}
	CountdownInt = SecondsLeft;
}

void AFillainPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AFillainPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (FillainHUD && FillainHUD->CharacterOverlay)
		{
			CharacterOverlay = FillainHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}



void AFillainPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AFillainPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedCLientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedCLientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();

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

float AFillainPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AFillainPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;

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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
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

void AFillainPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::WaitingToStart)
	{

	}

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void AFillainPlayerController::HandleMatchHasStarted()
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	if (FillainHUD)
	{
		FillainHUD->AddCharacterOverlay();
		if (FillainHUD->Announcement)
		{
			FillainHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AFillainPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}