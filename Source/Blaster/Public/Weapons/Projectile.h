// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/WeaponTypes.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	// virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Destroyed() override;
	// void HandlePostHitSFXDamagingPlayer();
	// void HandlePostHitSFXDamagingEnvironment();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactPlayerCharacterParticles;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactNiagaraSystem;


	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactPlayerCharacterNiagaraSystem;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactPlayerCharacterSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* AmmoMesh;

	// bool bHitByRocketLauncher = false;
	// bool bMissedByRocketLauncher = true;

	UPROPERTY()
	class AFillainCharacter* DamagedPawn;

	UPROPERTY()
	AFillainCharacter* InstigatorFillainCharacter;

	// Only set this for Grenades and Rockets
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	//Doesn't matter for Grenades and Rockets
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	/*******************************************
	****    Used with Server Side Rewind    ****
	*******************************************/

	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

protected:
	virtual void BeginPlay() override;

	/*
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDestroy();

	
	UPROPERTY(Replicated)
	bool bHitPlayerCharacter = false;
	*/

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;



	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	void SpawnTrailSystem();

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

	void StartDestroyTimer();

	void DestroyTimerFinished();

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	void ExplodeDamage();

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 300.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 600.f;

private:	
	

	UPROPERTY(EditAnywhere)
	class UParticleSystem* CascadeTracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerCascadeComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* NiagaraTracer;

	UPROPERTY()
	class UNiagaraComponent* TracerNiagaraComponent;

	UPROPERTY()
	EWeaponType WeaponType;;

public:
	FORCEINLINE EWeaponType GetWeaponType() const{ return WeaponType; }
	FORCEINLINE float GetDamage() const { return Damage; }
	
	

};
