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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Destroyed() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactNiagaraParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactPlayerCharacterParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactPlayerCharacterSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* AmmoMesh;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDestroy();

	UPROPERTY(Replicated)
	bool bHitPlayerCharacter = false;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

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
	
	

};
