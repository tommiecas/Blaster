// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/FillainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerState.h"
#include "HUD/OverheadWidget.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/Weapon.h"
#include "HAFComponents/CombatComponent.h"
#include "HAFComponents/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include <Kismet/KismetMathLibrary.h>
#include "Characters/FillainAnimInstance.h"
#include "Blaster/Blaster.h"
#include "PlayerController/FillainPlayerController.h"
#include "GameMode/HAFGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "PlayerState/HAFPlayerState.h"
#include "Weapons/WeaponTypes.h"
#include "GameMode/LobbyGameMode.h"
#include "Weapons/Projectile.h"


AFillainCharacter::AFillainCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
// 	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 1000.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AFillainCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AFillainCharacter, OverlappingWeapon, COND_OwnerOnly);
	// DOREPLIFETIME(AFillainCharacter, HitReactMontage);
	DOREPLIFETIME(AFillainCharacter, Health);
	DOREPLIFETIME(AFillainCharacter, bDisableGameplay);
}


void AFillainCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AFillainCharacter::Eliminate()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->DropWeapon();
	}
	MulticastEliminate();
	GetWorldTimerManager().SetTimer(
		EliminationTimer,
		this,
		&AFillainCharacter::EliminationTimerFinished,
		EliminationDelay
	);
}

void AFillainCharacter::MulticastEliminate_Implementation()
{
	if (VictimController)
	{
		VictimController->SetHUDWeaponAmmo(0);
	}
	bIsEliminated = true;
	PlayEliminatedMontage();

	// Start Dissolve Effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 300.f);
	}
	StartDissolve();

	// Disable Character Movement
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	// Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//Spawn Elimination-Bot
	if (EliminationBotEffect)
	{
		FVector EliminationBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		EliminationBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			EliminationBotEffect,
			EliminationBotSpawnPoint,
			GetActorRotation()
		);
	}
	if (EliminationBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			EliminationBotSound,
			GetActorLocation()
		);
	}
	bool bHideSniperSscope = IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperSscope)
	{
		ShowSniperScopeWidget(false);
	}
}

void AFillainCharacter::EliminationTimerFinished()
{
	AHAFGameMode* HAFGameMode = GetWorld()->GetAuthGameMode<AHAFGameMode>();
	if (HAFGameMode)
	{
		HAFGameMode->RequestRespawn(this, Controller);

	}
}
void AFillainCharacter::Destroyed()
{
	Super::Destroyed();
	if (EliminationBotComponent)
	{
		EliminationBotComponent->DestroyComponent();
	}

	AHAFGameMode* HAFGameMode = Cast<AHAFGameMode>(UGameplayStatics::GetGameMode(this));
	bool bIsMatchNotInProgress = HAFGameMode && HAFGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat && Combat->EquippedWeapon && bIsMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void AFillainCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AFillainPlayerController* FillainController = Cast<AFillainPlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(FillainController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(HAFMappingContext, 0);
		}
	}
	/*if (FillainPlayerController == nullptr)
	{
		FillainPlayerController = Cast<AFillainPlayerController>(GetController());
	}
	if (FillainPlayerController)
	{
		// Set the player state or any other necessary properties here
		FillainPlayerController->InitPlayerState();
		HAFPlayerState = FillainPlayerController->GetPlayerState<AHAFPlayerState>();

		// Log the player controller name for debugging
		UE_LOG(LogTemp, Log, TEXT("FillainPlayerController initialized: %s"), *FillainPlayerController->GetName());
	}*/
	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AFillainCharacter::ReceiveDamage);
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void AFillainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RotateInPlace(DeltaTime);
	HideCharacterIfCameraClose();
	PollInit();
}

void AFillainCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void AFillainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFillainCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFillainCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AFillainCharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &AFillainCharacter::EquipButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AFillainCharacter::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AFillainCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AFillainCharacter::AimButtonReleased);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AFillainCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AFillainCharacter::FireButtonReleased);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AFillainCharacter::ReloadButtonPressed);
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Triggered, this, &AFillainCharacter::GrenadeButtonPressed);
	}
}

void AFillainCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
	}
}

void AFillainCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AFillainCharacter::PlayReloadingMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadingMontage)
	{
		AnimInstance->Montage_Play(ReloadingMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;

		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AFillainCharacter::PlayEliminatedMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EliminatedMontage)
	{
		AnimInstance->Montage_Play(EliminatedMontage);
	}
}

void AFillainCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void AFillainCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
	// ReceiveDamage(CachedDamagedPawn, CachedDamage, CachedDamageType, CachedInstigatorController, CachedDamageCauser);
}

void AFillainCharacter::GrenadeButtonPressed()
{
	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}

void AFillainCharacter::ReceiveDamage(AActor* DamagedPawn, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (bIsEliminated) return;

	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();
	CacheDamageParameters(DamagedPawn, Damage, DamageType, InstigatorController, DamageCauser);

	if (Health == 0.f)
	{
		AHAFGameMode* HAFGameMode = GetWorld()->GetAuthGameMode<AHAFGameMode>();
				
		if (HAFGameMode)
		{
			FillainPlayerController = FillainPlayerController == nullptr ? Cast<AFillainPlayerController>(Controller) : FillainPlayerController;
			AFillainPlayerController* KillerController = Cast<AFillainPlayerController>(InstigatorController);
			AFillainCharacter* KillerFillain = Cast<AFillainCharacter>(InstigatorController->GetPawn());
			AFillainCharacter* VictimFillain = Cast<AFillainCharacter>(DamagedPawn);
			AFillainPlayerController* ControllerOfVictim = Cast<AFillainPlayerController>(VictimFillain->GetController());
			HAFGameMode->PlayerEliminated(this, ControllerOfVictim, KillerController);
			ControllerOfVictim->SetHUDEliminationMessage(KillerController, ControllerOfVictim);
			KillerController->SetHUDEliminationMessage(KillerController, ControllerOfVictim);
		}
	}
	ResetCachedDamageParameters();
}

void AFillainCharacter::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay)
	{
		bDisableGameplay = false;
	}
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AFillainCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);

}

void AFillainCharacter::EquipButtonPressed()
{
	if (bDisableGameplay)
	{
		bDisableGameplay = false;
	}

	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void AFillainCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void AFillainCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay)
	{
		bDisableGameplay = false;
	}
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AFillainCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay)
	{
		bDisableGameplay = false;
	}

	if (Combat)
	{
		Combat->Reloading();
	}
}

void AFillainCharacter::AimButtonPressed()
{
	if (bDisableGameplay)
	{
		bDisableGameplay = false;
	}

	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void AFillainCharacter::AimButtonReleased()
{
	if (bDisableGameplay)
	{
		bDisableGameplay = false;
	}

	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

float AFillainCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AFillainCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) //standing still and not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) //moving or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	CalculateAO_Pitch();
}

void AFillainCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AFillainCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}


	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void AFillainCharacter::Jump()
{
	if (bDisableGameplay)
	{
		bDisableGameplay = false;
	}
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void AFillainCharacter::FireButtonPressed()
{
	if (bDisableGameplay)
	{
		bDisableGameplay = false;
	}

	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AFillainCharacter::FireButtonReleased()
{
	if (bDisableGameplay)
	{
		bDisableGameplay = false;
	}


	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AFillainCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AFillainCharacter::HideCharacterIfCameraClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AFillainCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void AFillainCharacter::UpdateHUDHealth()
{
	FillainPlayerController = FillainPlayerController == nullptr ? Cast<AFillainPlayerController>(Controller) : FillainPlayerController;

	if (FillainPlayerController)
	{
		FillainPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AFillainCharacter::PollInit()
{
	if (HAFPlayerState == nullptr)
	{
		HAFPlayerState = GetPlayerState<AHAFPlayerState>();
		if (HAFPlayerState)
		{
			HAFPlayerState->AddToScore(0.f);
			HAFPlayerState->AddToDefeats(0);
		}
	}
}

void AFillainCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AFillainCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AFillainCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void AFillainCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidgets(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidgets(true);
		}
	}
}

void AFillainCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidgets(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidgets(false);
	}
}

bool AFillainCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AFillainCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* AFillainCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AFillainCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState AFillainCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}






/************************************************************************
**   I added the following functions to complete optional challenges   **
**   in the course, and they're proven to work correctly.			   **
************************************************************************/

void AFillainCharacter::SwitchWeapon(AWeapon* NewWeapon)
{
	if (NewWeapon && Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon = NewWeapon;
		// Update the HUD with the new weapon type
		AFillainPlayerController* PC = Cast<AFillainPlayerController>(GetFillainPlayerController());
		if (PC)
		{
			PC->SetHUDWeaponType(this);
		}
	}
}

AWeapon* AFillainCharacter::GetOverlappingWeapon()
{
	return OverlappingWeapon;
}


AFillainPlayerController* AFillainCharacter::GetFillainPlayerController()
{
	AFillainPlayerController* FillainController = Cast<AFillainPlayerController>(GetController());
	return FillainController;
}


void AFillainCharacter::CacheDamageParameters(AActor* DamagedPawn, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (CachedDamagedPawn == nullptr && CachedDamage == 0.0f && CachedDamageType == nullptr && CachedInstigatorController == nullptr && CachedDamageCauser == nullptr)
	{
		CachedDamagedPawn = DamagedPawn;
		CachedDamage = Damage;
		CachedDamageType = DamageType;
		CachedInstigatorController = InstigatorController;
		CachedDamageCauser = DamageCauser;
	}
}

void AFillainCharacter::ResetCachedDamageParameters()
{
	CachedDamagedPawn = nullptr;
	CachedDamage = 0.0f;
	CachedDamageType = nullptr;
	CachedInstigatorController = nullptr;
	CachedDamageCauser = nullptr;
}

void AFillainCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	ShowPlayerName();
}

void AFillainCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	// Ensure FillainPlayerController is initialized

	if (AFillainPlayerController* NewFillainController = Cast<AFillainPlayerController>(NewController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(NewFillainController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(HAFMappingContext, 0);
		}
		if (NewFillainController)
		{
			UE_LOG(LogTemp, Warning, TEXT("Successfully initialized FillainPlayerController"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to initialize FillainPlayerController"));
		}
		ShowPlayerName();
	}
}

 /******************************************************\
| **   The following were also added for challenges.  ** |
| **   They didn't do squat.			              ** |
 \******************************************************/

/* void AFillainCharacter::Restart()
{
	Super::Restart();
} 

void AFillainCharacter::OnFillainDying(AFillainCharacter* KillerFillain, AFillainCharacter* VictimFillain, AFillainPlayerController* InstigatorController)
{
	AHAFGameMode* HAFGameMode = GetWorld()->GetAuthGameMode<AHAFGameMode>();
	VictimCharacter = Cast<AFillainCharacter>(VictimFillain);
	VictimController = Cast<AFillainPlayerController>(VictimCharacter->GetController());
	AFillainPlayerController* KillerController = Cast<AFillainPlayerController>(InstigatorController);
	if (VictimCharacter && HAFGameMode && VictimController && KillerController)
	{
		HAFGameMode->PlayerEliminated(this, VictimController, KillerController);
		VictimController->SetHUDEliminationMessage(KillerController, VictimController);
		KillerController->SetHUDEliminationMessage(KillerController, VictimController);
	}
} */