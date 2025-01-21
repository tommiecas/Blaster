// Fill out your copyright notice in the Description page of Project Settings.


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
#include "Components/CombatComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Public/NiagaraSystemInstance.h"



AProjectileRocket::AProjectileRocket()
{
	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
	if (TrailSystem)
	{
	SpawnTrailSystem();
	}
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

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	ExplodeDamage();
	StartDestroyTimer();

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
}

void AProjectileRocket::Destroyed()
{
	
}


