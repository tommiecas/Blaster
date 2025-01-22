// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/FillainHUD.h"
#include "Weapons/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AFillainCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reloading();
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

    UPROPERTY()
    EWeaponType WeaponType;

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	
	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReloading();

	void HandleReload();
	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	void DropEquippedWeapon();

	void AttachActorToRightHand(class AActor* ActorToAttach);
	void AttachActorToLeftHand(class AActor* ActorToAttach);

	void UpdateCarriedAmmo();

	void PlayWeaponEquipSound();

	void ReloadEmptyWeapon();

private:
	UPROPERTY()
	class AFillainCharacter* Character;
	
	UPROPERTY()
	class AFillainPlayerController* Controller;
	
	UPROPERTY()
	class AFillainHUD* PlayerHUD;

	

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	bool bIsFireButtonPressed;

	/*
	** HUD and Crosshairs
	*/
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	UPROPERTY()
	FHUDPackage HUDPackage;

	FVector HitTarget;

	/*
	** Aiming and FOV
	*/
	float DefaultFOV;
	//field of view when not aiming; set to the camera's base FOV in BeginPlay

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/*
	** Automatic Fire
	*/
	FTimerHandle FireTimer;

	bool bCanGunFire = true;
	void StartFireTimer();
	void FireTimerFinished();

	bool CanIFire();

	//carried ammo for the currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	void InitializeCarriedAmmo();

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();
	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();

public:	
	FORCEINLINE bool IsAiming() const { return bAiming; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	
		
};
