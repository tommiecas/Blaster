// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/FillainPlayerController.h"
#include "HUD/FillainHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Characters/FillainCharacter.h"
#include "Weapons/Weapon.h"
#include "PlayerState/HAFPlayerState.h"
#include "Weapons/WeaponTypes.h"
#include "UObject/EnumProperty.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "HAFComponents/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerState.h"
#include "HUD/OverheadWidget.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Characters/FillainAnimInstance.h"
#include "Blaster/Blaster.h"
#include "GameMode/HAFGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameMode/LobbyGameMode.h"
#include "HUD/Announcement.h"
#include "GameStates/HAFGameState.h"
#include "Components/Image.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "HUD/ReturnToMainMenu.h"


AFillainPlayerController::AFillainPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	MatchTimeElapsedTime = 0.f;

	MatchCountdownColor = FLinearColor(0.10033f, 8.602301f, 10.0f, 10.0f);
	MatchCountdownBlinkingColor = FLinearColor(10.0f, 0.0f, 0.491076f, 10.0f);

	bIsMatchCountdownVisible = true;
}

void AFillainPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FillainHUD = Cast<AFillainHUD>(GetHUD());
	ServerCheckMatchState();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(FillainMappingContext, 0);
	}
}

void AFillainPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFillainPlayerController, MatchState);
}

void AFillainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);

	/************************
	*** OPTIONAL CHALLENGE **
	************************/
	
	MatchTimeElapsedTime += DeltaTime;
	if (static_cast<int32>(ThirtySecondsOnTheClock) >= static_cast<int32>(CountdownInt) && CountdownInt >= 0)

	{
		if (MatchState == MatchState::InProgress)
		{
			UpdateMatchCountdownColor();
		}
		if (FMath::Frac(MatchTimeElapsedTime) >= 0.5f && MatchState == MatchState::InProgress)
		{
			ToggleMatchCountdownVisibility();
		}
		else if (MatchTimeElapsedTime >= MatchTime)
		{
			FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
			bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->MatchCountdownText;
			const FString TimesUpText = TEXT("TIME'S UP!!");
			if (bIsHUDValid)
			{
				FillainHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(TimesUpText));
			}
		}
	}
}

void AFillainPlayerController::CheckPing(float DeltaTime)
{
	if (HasAuthority()) return;

    HighPingRunningTime += DeltaTime;
    if (HighPingRunningTime > CheckPingFrequency)
    {
        PlayerState = PlayerState == nullptr ? TObjectPtr<APlayerState>(GetPlayerState<APlayerState>()) : PlayerState;
        if (PlayerState)
        {
			UE_LOG(LogTemp, Warning, TEXT("PlayerState->GetPingInMilliseconds() * 4 : %d"), static_cast<int32>(PlayerState->GetPingInMilliseconds() * 4));
			float PingInMs = PlayerState->GetPingInMilliseconds(); // Assuming GetPingInMilliseconds() is a method that returns the ping in milliseconds
            if (PingInMs * 4 > HighPingThreshold) // ping is compressed; it's actually ping / 4
            {
                HighPingWarning();
                PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
            }
        }
        HighPingRunningTime = 0.f;
    }
    bool bIsHighPingAnimationPlaying =
        FillainHUD &&
        FillainHUD->CharacterOverlay &&
        FillainHUD->CharacterOverlay->HighPingAnimation &&
        FillainHUD->CharacterOverlay->IsAnimationPlaying(FillainHUD->CharacterOverlay->HighPingAnimation);
    if (bIsHighPingAnimationPlaying)
    {
        PingAnimationRunningTime += DeltaTime;
        if (PingAnimationRunningTime > HighPingDuration)
        {
            StopHighPingWarning();
        }
    }
    else
    {
        StopHighPingWarning();
    }
}

// Is the ping too high?
void AFillainPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
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

void AFillainPlayerController::HighPingWarning()
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;

	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->HighPingImage && FillainHUD->CharacterOverlay->HighPingAnimation;
	if (bIsHUDValid)
	{
		FillainHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		FillainHUD->CharacterOverlay->PlayAnimation(FillainHUD->CharacterOverlay->HighPingAnimation, 0.f, 5);
	}
}

void AFillainPlayerController::StopHighPingWarning()
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;

	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->HighPingImage && FillainHUD->CharacterOverlay->HighPingAnimation;
	if (bIsHUDValid)
	{
		FillainHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (FillainHUD->CharacterOverlay->IsAnimationPlaying(FillainHUD->CharacterOverlay->HighPingAnimation))
		{
			FillainHUD->CharacterOverlay->StopAnimation(FillainHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}



void AFillainPlayerController::ServerCheckMatchState_Implementation()
{
	AHAFGameMode* GameMode = Cast<AHAFGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);

		if (FillainHUD && MatchState == MatchState::Cooldown)
		{
			HandleCooldown();
		}
	}
}

void AFillainPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);

	if (FillainHUD && MatchState == MatchState::WaitingToStart)
	{
		FillainHUD->AddAnnouncement();
	}
}

void AFillainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AFillainCharacter* PlayerCharacter = Cast<AFillainCharacter>(InPawn);
	if (PlayerCharacter)
	{
		SetHUDHealth(PlayerCharacter->GetHealth(), PlayerCharacter->GetMaxHealth());
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
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AFillainPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;

	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->ShieldBar && FillainHUD->CharacterOverlay->ShieldText;
	if (bIsHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		FillainHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		FillainHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
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
		bInitializeScore = true;
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
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void AFillainPlayerController::SetHUDWeaponAmmo(int32 WeaponAmmo)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bIsHUDValid)
	{
		FString WeaponAmmoText = FString::Printf(TEXT("%d"), WeaponAmmo);
		FillainHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(WeaponAmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = WeaponAmmo;
	}
}

void AFillainPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmo)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bIsHUDValid)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), CarriedAmmo);
		FillainHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = CarriedAmmo;
	}
}

void AFillainPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->MatchCountdownText;
	if (bIsHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			FillainHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		FillainHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AFillainPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->Announcement && FillainHUD->Announcement->WarmupTime;
	if (bIsHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			FillainHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		FillainHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AFillainPlayerController::SetHUDGrenades(int32 Grenades)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->GrenadesText;
	if (bIsHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		FillainHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void AFillainPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	/*
	if (HasAuthority())
	{
		Mode = Mode == nullptr ? Cast<AHAFGameMode>(UGameplayStatics::GetGameMode(this)) : Mode;
		if (Mode)
		{
			SecondsLeft = FMath::CeilToInt(Mode->GetCountdownTime() + LevelStartingTime);
		}
	} */

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
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
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeGrenades) SetHUDGrenades(HUDGrenades);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				AFillainCharacter* FillainCharacter = Cast<AFillainCharacter>(GetPawn());
				if (FillainCharacter && FillainCharacter->GetCombatComponent())
				{
					if (bInitializeGrenades) SetHUDGrenades(FillainCharacter->GetCombatComponent()->GetGrenades());
				}
			}
		}
	}
}

void AFillainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();	
	if (InputComponent == nullptr) return;

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	
	EnhancedInputComponent->BindAction(QuitAction, ETriggerEvent::Triggered, this, &AFillainPlayerController::ShowReturnToMainMenu);

}

void AFillainPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void AFillainPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AFillainPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;	
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
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

void AFillainPlayerController::OnMatchStateSet(FName NewState)
{
	MatchState = NewState;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AFillainPlayerController::OnRep_MatchState()
{

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AFillainPlayerController::HandleMatchHasStarted()
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	if (FillainHUD)
	{
		if (FillainHUD->CharacterOverlay == nullptr) FillainHUD->AddCharacterOverlay();
		if (FillainHUD->Announcement)
		{
			FillainHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AFillainPlayerController::HandleCooldown()
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	if (FillainHUD)
	{
		FillainHUD->CharacterOverlay->RemoveFromParent();;
		bool bHUDValid = FillainHUD->Announcement && FillainHUD->Announcement->AnnouncementText && FillainHUD->Announcement->InfoText;
		if (bHUDValid)
		{
			FillainHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In:");
			FillainHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			AHAFGameState* HAFGameState = Cast<AHAFGameState>(UGameplayStatics::GetGameState(this));
			AHAFPlayerState* HAFPlayerState = GetPlayerState<AHAFPlayerState>();
			if (HAFGameState && HAFPlayerState)
			{
				TArray<AHAFPlayerState*> TopPlayers = HAFGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("Nobody Won! \n You're all losers!");

				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == HAFPlayerState)
				{
					InfoTextString = FString("YOU are the Winner! \n The rest of you suck!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("The Winner Is: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("We have a tie! \n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n Fight to the Death!"), *TiedPlayer->GetPlayerName()));
					}
				}
				else if (TopPlayers.Num() > 1 && TopPlayers.Contains(HAFPlayerState))
				{
					InfoTextString = FString("YOU Tied for the Win, Alongside: \n");
					TopPlayers.Remove(HAFPlayerState);
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n Fight to the death!"), *TiedPlayer->GetPlayerName()));
					}
				}
				FillainHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	AFillainCharacter* FillainCharacter = Cast<AFillainCharacter>(GetPawn());
	if (FillainCharacter && FillainCharacter->GetCombatComponent())
	{
		FillainCharacter->bDisableGameplay = true;
		FillainCharacter->GetCombatComponent()->FireButtonPressed(false);
	}
}






/************************************************************************
**   I added the following functions to complete optional challenges   **
**   in the course, and they're proven to work correctly.			   **
************************************************************************/

void AFillainPlayerController::UpdateMatchCountdownColor()
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->MatchCountdownText;
	if (bIsHUDValid)
	{
		if (MatchTimeElapsedTime >= (MatchTime - ThirtySecondsOnTheClock) && MatchTimeElapsedTime < MatchTime)
		{
			if (FillainHUD->CharacterOverlay->MatchCountdownText)
			{
				FillainHUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(MatchCountdownBlinkingColor);
			}
		}
		else if (MatchTimeElapsedTime < (MatchTime - ThirtySecondsOnTheClock))
		{
			if (FillainHUD->CharacterOverlay->MatchCountdownText)
			{
				FillainHUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(MatchCountdownColor);
			}
		}
	}
}

void AFillainPlayerController::ToggleMatchCountdownVisibility()
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->MatchCountdownText;
	if (bIsHUDValid) 
	{
		FTimerHandle MatchCountdownTimer;
		GetWorldTimerManager().SetTimer(MatchCountdownTimer, [&]()
			{FillainHUD->CharacterOverlay->MatchCountdownText->SetVisibility(ESlateVisibility::Hidden); }, .5f, false);
		FillainHUD->CharacterOverlay->MatchCountdownText->SetVisibility(ESlateVisibility::Visible);
	}
}

FString AFillainPlayerController::GetWeaponTypeDisplayName(EWeaponType WeaponType)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("WeaponType"), true);
	if (!EnumPtr) return FString("");

	return EnumPtr->GetDisplayNameTextByValue((int64)WeaponType).ToString();
}

void AFillainPlayerController::SetHUDWeaponType(APawn* InPawn)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	AFillainCharacter* FCharacter = Cast<AFillainCharacter>(InPawn);
	EquippedWeapon = EquippedWeapon == nullptr ? Cast<AWeapon>(FCharacter->GetEquippedWeapon()) : EquippedWeapon;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->WeaponTypeText;
	if (bIsHUDValid && FCharacter && EquippedWeapon)
	{
		FString WeaponTypeName = GetWeaponTypeDisplayName(EquippedWeapon->GetWeaponType());
		FillainHUD->CharacterOverlay->WeaponTypeText->SetText(FText::FromString(WeaponTypeName));
	}
}

void AFillainPlayerController::SetHUDEliminationMessage(AFillainPlayerController* KillerController, AFillainPlayerController* VictimController)
{
	FillainHUD = FillainHUD == nullptr ? Cast<AFillainHUD>(GetHUD()) : FillainHUD;
	bool bIsHUDValid = FillainHUD && FillainHUD->CharacterOverlay && FillainHUD->CharacterOverlay->EliminationMessageText && FillainHUD->CharacterOverlay->VictimNameText && FillainHUD->CharacterOverlay->KillerNameText;

	if (FillainHUD && bIsHUDValid && VictimController && KillerController)
	{
		FString NameOfVictim = VictimController->PlayerState->GetPlayerName();
		FString NameOfKiller = KillerController->PlayerState->GetPlayerName();
		FString EliminationMessage = FString::Printf(TEXT("Was Eliminated By"));
		FString VictimName = FString::Printf(TEXT("%s"), *NameOfVictim);
		FString KillerName = FString::Printf(TEXT("%s"), *NameOfKiller);
		FillainHUD->CharacterOverlay->VictimNameText->SetText(FText::FromString(VictimName));
		FillainHUD->CharacterOverlay->KillerNameText->SetText(FText::FromString(KillerName));
		FillainHUD->CharacterOverlay->EliminationMessageText->SetText(FText::FromString(EliminationMessage));
		FTimerHandle TimerHandleVictim;
		FTimerHandle TimerHandleKiller;
		FTimerHandle TimerHandleElimination;
		GetWorldTimerManager().SetTimer(TimerHandleVictim, [this]() {
			FillainHUD->CharacterOverlay->VictimNameText->SetText(FText::GetEmpty()); },
			3.0f,
			false
			);
		GetWorldTimerManager().SetTimer(TimerHandleKiller, [this]() {
			FillainHUD->CharacterOverlay->KillerNameText->SetText(FText::GetEmpty()); },
			3.0f,
			false
			);
		GetWorldTimerManager().SetTimer(TimerHandleElimination, [this]() {
			FillainHUD->CharacterOverlay->EliminationMessageText->SetText(FText::GetEmpty()); },
			3.0f,
			false
			);
	}
}
	