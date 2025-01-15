// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Projectile.h"
#include "Sound/SoundCue.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "Weapons/Weapon.h"
#include "Characters/FillainCharacter.h"



AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* RocketFiringPawn = GetInstigator();
	if (RocketFiringPawn)
	{
		AController* RocketFiringController = RocketFiringPawn->GetController();
		if (RocketFiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, //WorldContextObject
				Damage, //BaseDamage
				10.f, //Minimum Damage
				GetActorLocation(), // Origin
				300.f, //DamageInnerRadius
				600.f, //DamageOuterRadius
				1.f, // DamageFalloff
				UDamageType::StaticClass(), //DamageType
				TArray<AActor*>(), //IgnoreActors
				this, //DamageCauser
				RocketFiringController //InstigatedBy
			);
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileRocket::Destroyed()
{
	Super::Destroyed();
	APawn* FiringPawn = GetInstigator();
	AFillainCharacter* FiringFillain = Cast<AFillainCharacter>(FiringPawn);
	AWeapon* FiredWeapon = FiringFillain->GetEquippedWeapon();

	if (bHitPlayerCharacter == true)
	{
		if (FiringFillain && FiredWeapon && FiredWeapon->GetWeaponType() == EWeaponType::EWT_RocketLauncher) // Fix the assignment operator to comparison operator
		{
			if (ImpactPlayerCharacterParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactPlayerCharacterParticles, GetActorTransform());
			}
			if (ImpactPlayerCharacterSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ImpactPlayerCharacterSound, GetActorLocation());
			}
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
			}
			if (ImpactNiagaraParticles)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagaraParticles, GetActorLocation(), GetActorRotation());
			}
			if (ImpactSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
			}
		}
		else if ((FiredWeapon && FiredWeapon->GetWeaponType() != EWeaponType::EWT_RocketLauncher))
		{
			if (ImpactPlayerCharacterParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactPlayerCharacterParticles, GetActorTransform());
			}
			if (ImpactPlayerCharacterSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ImpactPlayerCharacterSound, GetActorLocation());
			}
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
			}
		}
	}
	else if (bHitPlayerCharacter == false)
	{
		if (FiringFillain && FiredWeapon && FiredWeapon->GetWeaponType() == EWeaponType::EWT_RocketLauncher) // Fix the assignment operator to comparison operator
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
			}
			if (ImpactNiagaraParticles)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagaraParticles, GetActorLocation(), GetActorRotation());
			}
			if (ImpactSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
			}
		}
		else if ((FiredWeapon && FiredWeapon->GetWeaponType() != EWeaponType::EWT_RocketLauncher))
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
			}
			if (ImpactSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
			}
		}
	}
}