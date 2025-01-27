// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScanWeaponAmmo.h"
#include "Weapons/ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Projectile.h"
#include "Sound/SoundCue.h"

#include "GameFramework/Character.h"
#include "Weapons/Weapon.h"
#include "Characters/FillainCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Weapons/RocketMovementComponent.h"
#include "HAFComponents/CombatComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Public/NiagaraSystemInstance.h"


AHitScanWeaponAmmo::AHitScanWeaponAmmo()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
}

void AHitScanWeaponAmmo::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}



void AHitScanWeaponAmmo::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	/*
	APawn* GunFiringPawn = GetInstigator();
	if (GunFiringPawn && HasAuthority())
	{
		AController* GunFiringController = GunFiringPawn->GetController();
		if (GunFiringController)
		{
			ACharacter* KillerCharacter = Cast<ACharacter>(GetInstigator());
			if (KillerCharacter)
			{
				AFillainCharacter* HitFillain = Cast<AFillainCharacter>(DamagedActor);
				AFillainCharacter* KillerFillain = Cast<AFillainCharacter>(KillerCharacter);
				TArray<AActor*> IgnoreActors;
				IgnoreActors.Add(KillerFillain);
				AWeapon* FiredWeapon = KillerFillain->GetCombatComponent()->EquippedWeapon;
				if (KillerFillain && HitFillain && HitFillain->Implements<UInteractWithCrosshairsInterface>())
				{
					bHitPlayerCharacter = true;
					AController* KillerController = Cast<AController>(GetInstigatorController());
					HandlePostHitSFXDamagingPlayer();
					UDamageType const* const DamageType = UDamageType::StaticClass()->GetDefaultObject<UDamageType>();
					HitFillain->ReceiveDamage(HitFillain, Damage, DamageType, KillerController, this);
				}
				else
				{
					bHitPlayerCharacter = false;
					bHitByRocketLauncher = false;
					bMissedByRocketLauncher = true;
					HandlePostHitSFXDamagingEnvironment();
					return;
				}
				if (AmmoMesh)
				{
					AmmoMesh->SetVisibility(false);
				}
				if (CollisionBox)
				{
					CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
			}
		}
	}
	*/
	if (OtherActor == GetOwner())
	{
		return;
	}
	ExplodeDamage();
	StartDestroyTimer();
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactPlayerCharacterParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactPlayerCharacterParticles, GetActorTransform());
	}
	if (ImpactNiagaraSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagaraSystem, GetActorLocation(), GetActorRotation());
	}
	if (ImpactPlayerCharacterNiagaraSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactPlayerCharacterNiagaraSystem, GetActorLocation(), GetActorRotation());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (ImpactPlayerCharacterSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactPlayerCharacterSound, GetActorLocation());
	}
}

void AHitScanWeaponAmmo::Destroyed()
{

}




