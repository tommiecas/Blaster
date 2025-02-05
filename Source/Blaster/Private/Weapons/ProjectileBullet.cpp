// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Projectile.h"
#include "Sound/SoundCue.h"

#include "GameFramework/Character.h"
#include "Weapons/Weapon.h"
#include "Characters/FillainCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Weapons/BulletMovementComponent.h"
#include "HAFComponents/CombatComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Public/NiagaraSystemInstance.h"



AProjectileBullet::AProjectileBullet()
{
	BulletMovementComponent = CreateDefaultSubobject<UBulletMovementComponent>(TEXT("BulletMovementComponent"));
	BulletMovementComponent->bRotationFollowsVelocity = true;
	BulletMovementComponent->SetIsReplicated(true);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}

	SpawnTrailSystem();

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	ExplodeDamage();
	StartDestroyTimer();
	if (ImpactParticles && (Hit.GetActor() && !Hit.GetActor()->Implements<UInteractWithCrosshairsInterface>()))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	else if (ImpactPlayerCharacterParticles && (Hit.GetActor() && Hit.GetActor()->Implements<UInteractWithCrosshairsInterface>()))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactPlayerCharacterParticles, GetActorTransform());
	}
	if (ImpactNiagaraSystem && (Hit.GetActor() && !Hit.GetActor()->Implements<UInteractWithCrosshairsInterface>()))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagaraSystem, GetActorLocation(), GetActorRotation());
	}
	else if (ImpactPlayerCharacterNiagaraSystem && (Hit.GetActor() && Hit.GetActor()->Implements<UInteractWithCrosshairsInterface>()))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactPlayerCharacterNiagaraSystem, GetActorLocation(), GetActorRotation());
	}
	if (ImpactSound && (Hit.GetActor() && !Hit.GetActor()->Implements<UInteractWithCrosshairsInterface>()))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	else if (ImpactPlayerCharacterSound && (Hit.GetActor() && Hit.GetActor()->Implements<UInteractWithCrosshairsInterface>()))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactPlayerCharacterSound, GetActorLocation());
	}
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate();
	}
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}

	/*
	APawn* FiringPawn = GetInstigator();
		AFillainCharacter* FiringFillain = Cast<AFillainCharacter>(FiringPawn);
		if (FiringFillain == nullptr) return; // Add this check
		AWeapon* FiredWeapon = FiringFillain->GetCombatComponent()->EquippedWeapon;

		if (bHitPlayerCharacter == true)
		{
			HandlePostHitSFXDamagingPlayer();
		}
		else
		{
			HandlePostHitSFXDamagingPlayer();
		}
		if (ProjectileMesh)
		{
			ProjectileMesh->SetVisibility(false);
		}
		if (AmmoMesh)
		{
			AmmoMesh->SetVisibility(false);
		}
		if (CollisionBox)
		{
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
		{
			TrailSystemComponent->GetSystemInstance()->Deactivate();;
		}

		if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
		{
			ProjectileLoopComponent->Stop();
		}
		*/
}

void AProjectileBullet::Destroyed()
{

}


