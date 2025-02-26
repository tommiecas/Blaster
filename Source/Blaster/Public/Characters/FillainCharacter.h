// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HUD/OverheadWidget.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "GameMode/LobbyGameMode.h"
#include "Blaster/BlasterTypes/Team.h"
#include "FillainCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UWidgetComponent;
class UCombatComponent;
class UAnimMontage;
class UCurveFloat;
class UTimelineComponent;
class UCurveFloat;
class UCurveLinearColor;
class UCurveVector;
class UDamageType;
class AFillainPlayerController;
class AHAFPlayerState;
class ALobbyGameMode;
class AProjectile;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerLeavesGame);

UCLASS()
class BLASTER_API AFillainCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	AFillainCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	// virtual void Restart() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;
	void Eliminate(bool bPlayerLeftGame);
	// void FinishElimination();
	virtual void Destroyed() override;
	// void OnFillainDying(AFillainCharacter* InstigatorFillain, AFillainCharacter* DyingFillain, class AFillainPlayerController* InstigatorController);
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();
	void SwitchWeapon(AWeapon* NewWeapon);

	UPROPERTY()
	AFillainPlayerController* FillainPlayerController;

	UPROPERTY()
	AFillainPlayerController* VictimController;

	UPROPERTY()
	class AHAFPlayerState* HAFPlayerState;

	UPROPERTY()
	AFillainCharacter* VictimCharacter = nullptr;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate(bool bPlayerLeftGame);
	
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;


	/******************************
	****    HAF COMPONENTS    *****
	******************************/
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class ULagCompensationComponent* LagCompensation;

	void CacheDamageParameters(AActor* DamagedPawn, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);
	void ResetCachedDamageParameters();

	/************
	** Jumping **
	************/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;

	virtual void Jump() override;


	/******************
	** PLAY MONTAGES **
	******************/
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayEliminatedMontage();
	void PlayReloadingMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapMontage();

	UPROPERTY()
	AProjectile* Projectile;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedPawn, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void SpawnDefaultWeapon();

	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;

	bool bFinishedSwapping = false;

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	bool bLeftGame = false;

	FOnPlayerLeavesGame PlayerLeavesGame;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	void SetTeamColor(ETeam Team);


protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* HAFMappingContext;

	/****************** 
	** Moving Around **
	******************/ 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	void Move(const FInputActionValue& Value);

	/*******************
	** Looking Around **
	*******************/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	void Look(const FInputActionValue& Value);

	/**********************
	** Equipping Weapons **
	**********************/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* EquipAction;

	void EquipButtonPressed();

	/**************
	** Crouching **
	**************/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* CrouchAction;

	void CrouchButtonPressed();

	/**********************
	** Aiming the Weapon **
	**********************/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AimAction;

	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();

	/**********************
	** Firing the Weapon **
	**********************/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* FireAction;


	void FireButtonPressed();
	void FireButtonReleased();

	/*************************
	** Reloading the Weapon **
	*************************/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ReloadAction;

	void ReloadButtonPressed();

	/*************************
	** Throwing the Grenade **
	*************************/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ThrowAction;

	void GrenadeButtonPressed();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowPlayerName();
	

	// Poll for any relevant classes and initialize our HUD
	void PollInit();

	void RotateInPlace(float DeltaTime);

	/**
	* Hit boxes used for server-side rewind
	*/

	UPROPERTY(EditAnywhere)
	class UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;

	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyBothWeapons();

	void SetSpawnPoint();
	void OnPlayerStateInitialized();
	

private:	
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	/***********************
	** Animation Montages **
	***********************/

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(Replicated, EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;

	UPROPERTY(Replicated, EditAnywhere, Category = Combat)
	class UAnimMontage* EliminatedMontage;

	UPROPERTY(Replicated, EditAnywhere, Category = Combat)
	class UAnimMontage* ReloadingMontage;

	UPROPERTY(Replicated, EditAnywhere, Category = Combat)
	class UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapMontage;

	void HideCharacterIfCameraClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/******************
	** Player Health **
	******************/

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/******************
	** Player Shield **
	******************/

	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = "Player Stats")
	float Shield = 100.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);



	bool bIsEliminated = false;

	FTimerHandle EliminationTimer;

	void EliminationTimerFinished();

	UPROPERTY(EditDefaultsOnly)
	float EliminationDelay = 3.f;

	/********************
	** Dissolve Effect **
	********************/

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	//Dynamic Instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Eliminations)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	//Material Instance set ono the blueprint used with the dynamic material instance
	UPROPERTY(VisibleAnywhere, Category = Eliminations)
	UMaterialInstance* DissolveMaterialInstance;

	/****************************
	****      TEAM COLORS    ****
	****************************/
	UPROPERTY(EditAnywhere, Category = Eliminations)
	UMaterialInstance* RedMaterial;

	UPROPERTY(EditAnywhere, Category = Eliminations)
	UMaterialInstance* RedDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category = Eliminations)
	UMaterialInstance* BlueMaterial;

	UPROPERTY(EditAnywhere, Category = Eliminations)
	UMaterialInstance* BlueDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category = Eliminations)
	UMaterialInstance* OriginalMaterial;

	UPROPERTY(EditAnywhere, Category = Eliminations)
	UMaterialInstance* OriginalDissolveMaterialInstance;


	/****************************
	** Elimination-Bot/Effects **
	****************************/

	UPROPERTY(EditAnywhere)
	UParticleSystem* EliminationBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* EliminationBotComponent;
	
	UPROPERTY(EditAnywhere)
	class USoundCue* EliminationBotSound;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* CrownSystem;

	UPROPERTY(EditAnywhere)
	class UNiagaraComponent* CrownComponent;
	
	AActor* CachedDamagedPawn;
	float CachedDamage;
	const UDamageType* CachedDamageType;
	AController* CachedInstigatorController;
	AActor* CachedDamageCauser;

	/************
	** Grenade **
	************/
	UPROPERTY(VisibleAnywhere) 
	UStaticMeshComponent* AttachedGrenade;

	/*************************
	***   Default Weapon   ***
	*************************/
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY()
	class AHAFGameMode* HAFGameMode;

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetOverlappingWeapon();
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsEliminated() const { return bIsEliminated; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }	
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return Combat; }
	AHAFPlayerState* GetHAFPlayerState() const { return HAFPlayerState; }
	AFillainPlayerController* GetFillainPlayerController();
	FORCEINLINE AProjectile* GetProjectile() const { return Projectile; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadingMontage() const { return ReloadingMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return Buff; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	bool IsLocallyReloading();
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
	FORCEINLINE bool IsWieldingTheSword() const;
	ETeam GetTeam();
	void SetWieldingTheSword(bool bWielding);

};
