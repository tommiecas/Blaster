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
#include "Components/BoxComponent.h"
#include "HAFComponents/LagCompensationComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameStates/HAFGameState.h"
#include "PlayerStart/TeamPlayerStart.h"


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

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

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

	/***********************************************
	****    Hit boxes for server-side rewind    ****
	***********************************************/

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);
	
	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AFillainCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AFillainCharacter, OverlappingWeapon, COND_OwnerOnly);
	// DOREPLIFETIME(AFillainCharacter, HitReactMontage);
	DOREPLIFETIME(AFillainCharacter, Health);
	DOREPLIFETIME(AFillainCharacter, Shield);
	DOREPLIFETIME(AFillainCharacter, bDisableGameplay);
}


void AFillainCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AFillainCharacter::Eliminate(bool bPlayerLeftGame)
{
	DropOrDestroyBothWeapons();
	MulticastEliminate(bPlayerLeftGame);
	
}

void AFillainCharacter::MulticastEliminate_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
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
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
	bool bHideSniperScope = IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	GetWorldTimerManager().SetTimer(
		EliminationTimer,
		this,
		&AFillainCharacter::EliminationTimerFinished,
		EliminationDelay
	);
}

void AFillainCharacter::EliminationTimerFinished()
{
	HAFGameMode = HAFGameMode == nullptr ? GetWorld()->GetAuthGameMode<AHAFGameMode>() : HAFGameMode;
	if (HAFGameMode && !bLeftGame)
	{
		HAFGameMode->RequestRespawn(this, Controller);

	}
	if (bLeftGame && IsLocallyControlled())
	{
		PlayerLeavesGame.Broadcast();
	}
}

void AFillainCharacter::ServerLeaveGame_Implementation()
{
	AHAFGameMode* GameMode = GetWorld()->GetAuthGameMode<AHAFGameMode>();
	HAFPlayerState = HAFPlayerState == nullptr ? GetPlayerState<AHAFPlayerState>() : HAFPlayerState;
	if (GameMode && HAFPlayerState)
	{
		GameMode->PlayerLeftGame(HAFPlayerState);

	}
}

void AFillainCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->DropWeapon();
	}
}

void AFillainCharacter::DropOrDestroyBothWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
	}
}

void AFillainCharacter::OnPlayerStateInitialized()
{
	HAFPlayerState->AddToScore(0.f);
	HAFPlayerState->AddToDefeats(0);
	SetTeamColor(HAFPlayerState->GetTeam());
	SetSpawnPoint();
}

void AFillainCharacter::SetSpawnPoint()
{
	if (HasAuthority() && HAFPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for (auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if (TeamStart && TeamStart->Team == HAFPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		if (TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(
				ChosenPlayerStart->GetActorLocation(), 
				ChosenPlayerStart->GetActorRotation()
			);
		}
	}
}

void AFillainCharacter::Destroyed()
{
	Super::Destroyed();
	if (EliminationBotComponent)
	{
		EliminationBotComponent->DestroyComponent();
	}

	HAFGameMode = HAFGameMode == nullptr ? GetWorld()->GetAuthGameMode<AHAFGameMode>() : HAFGameMode;
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

	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();
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
	if (Combat && Combat->bWieldingTheSword)
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (Combat && Combat->EquippedWeapon) GetCharacterMovement()->bOrientRotationToMovement = false;
	if (Combat && Combat->EquippedWeapon) bUseControllerRotationYaw = true;
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
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<AFillainPlayerController>(Controller);
		}
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

void AFillainCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
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
		if (Combat && Combat->bWieldingTheSword) return;
		Combat->ThrowGrenade();
	}
}

void AFillainCharacter::ReceiveDamage(AActor* DamagedPawn, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	HAFGameMode = HAFGameMode == nullptr ? GetWorld()->GetAuthGameMode<AHAFGameMode>() : HAFGameMode;
	if (bIsEliminated || HAFGameMode == nullptr) return;
	Damage = HAFGameMode->CalculateDamage(InstigatorController, Controller, Damage);

	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();
	CacheDamageParameters(DamagedPawn, Damage, DamageType, InstigatorController, DamageCauser);

	if (Health == 0.f)
	{
		HAFGameMode = HAFGameMode == nullptr ? GetWorld()->GetAuthGameMode<AHAFGameMode>() : HAFGameMode;

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

void AFillainCharacter::SpawnDefaultWeapon()
{
	HAFGameMode = HAFGameMode == nullptr ? GetWorld()->GetAuthGameMode<AHAFGameMode>() : HAFGameMode;
	UWorld* World = GetWorld();
	if (HAFGameMode && World && !bIsEliminated && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

void AFillainCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(CrownSystem, GetMesh(), FName(), GetActorLocation() + FVector(0.f, 0.f, 110.f), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void AFillainCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void AFillainCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr || OriginalMaterial == nullptr) return;
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, OriginalMaterial);
		DissolveMaterialInstance = OriginalDissolveMaterialInstance;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMaterialInstance;
		break;
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMaterialInstance;
		break;


	}
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
		if (Combat->bWieldingTheSword) return;
		if (Combat->CombatState == ECombatState::ECS_Unoccupied) ServerEquipButtonPressed();
		bool bSwap = Combat->ShouldSwapWeapons() &&
			!HasAuthority() &&
			Combat->CombatState == ECombatState::ECS_Unoccupied &&
			OverlappingWeapon == nullptr;

		if (bSwap)
		{
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}
	}
}

void AFillainCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if (Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}

void AFillainCharacter::CrouchButtonPressed()
{
	if (Combat && Combat->bWieldingTheSword) return;
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
	if (Combat && Combat->bWieldingTheSword) return;
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
	if (Combat && Combat->bWieldingTheSword) return;
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
	if (Combat && Combat->bWieldingTheSword) return;
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
	if (Combat && Combat->bWieldingTheSword) return;
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
	if (Combat && Combat->bWieldingTheSword) return;
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
	if (Combat && Combat->bWieldingTheSword) return;
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
	UpdateHUDShield();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void AFillainCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
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

void AFillainCharacter::UpdateHUDShield()
{
	FillainPlayerController = FillainPlayerController == nullptr ? Cast<AFillainPlayerController>(Controller) : FillainPlayerController;

	if (FillainPlayerController)
	{
		FillainPlayerController->SetHUDShield(Shield, MaxShield);
	}
}
void AFillainCharacter::UpdateHUDAmmo()
{
	FillainPlayerController = FillainPlayerController == nullptr ? Cast<AFillainPlayerController>(Controller) : FillainPlayerController;

	if (FillainPlayerController && Combat && Combat->EquippedWeapon)
	{
		FillainPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		FillainPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}
void AFillainCharacter::PollInit()
{
	if (HAFPlayerState == nullptr)
	{
		HAFPlayerState = GetPlayerState<AHAFPlayerState>();
		if (HAFPlayerState)
		{
			OnPlayerStateInitialized();
			
			AHAFGameState* HAFGameState = Cast<AHAFGameState>(UGameplayStatics::GetGameState(this));

			if (HAFGameState && HAFGameState->TopScoringPlayers.Contains(HAFPlayerState))
			{
				MulticastGainedTheLead();
			}
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
		OverlappingWeapon->HidePickupWidgets();
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidgets();
		}
	}
}

void AFillainCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidgets();
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidgets();
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

bool AFillainCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

bool AFillainCharacter::IsWieldingTheSword() const
{
	if (Combat == nullptr) return false;
	return Combat->bWieldingTheSword;

}

ETeam AFillainCharacter::GetTeam()
{
	HAFPlayerState = HAFPlayerState == nullptr ? GetPlayerState<AHAFPlayerState>() : HAFPlayerState;
	if (HAFPlayerState == nullptr) return ETeam::ET_NoTeam;
	return HAFPlayerState->GetTeam();
}

void AFillainCharacter::SetWieldingTheSword(bool bWielding)
{
	if (Combat == nullptr) return;
	Combat->bWieldingTheSword = bWielding;
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