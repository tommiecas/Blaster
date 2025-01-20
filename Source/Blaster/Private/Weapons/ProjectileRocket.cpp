// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Projectile.h"
#include "Sound/SoundCue.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Public/NiagaraSystemInstance.h"
#include "GameFramework/Character.h"
#include "Weapons/Weapon.h"
#include "Characters/FillainCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Weapons/RocketMovementComponent.h"
#include "Components/CombatComponent.h"



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
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
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

void AProjectileRocket::DestroyTimerFinished()
{
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* RocketFiringPawn = GetInstigator();
	if (RocketFiringPawn && HasAuthority())
	{
		AController* RocketFiringController = RocketFiringPawn->GetController();
		if (RocketFiringController)
		{
			TArray<AActor*> IgnoreActors;
			IgnoreActors.Add(GetOwner());

			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, //WorldContextObject
				Damage, //BaseDamage
				10.f, //Minimum Damage
				GetActorLocation(), // Origin
				300.f, //DamageInnerRadius
				600.f, //DamageOuterRadius
				1.f, // DamageFalloff
				UDamageType::StaticClass(), //DamageType
				IgnoreActors, //IgnoreActors
				this, //DamageCauser
				RocketFiringController //InstigatedBy
			);
		}
	}
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectileRocket::DestroyTimerFinished,
		DestroyTime
	);
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


