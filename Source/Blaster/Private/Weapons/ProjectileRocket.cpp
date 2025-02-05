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
#include "HAFComponents/CombatComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Public/NiagaraSystemInstance.h"



AProjectileRocket::AProjectileRocket()
{
	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
	RocketMovementComponent->InitialSpeed = InitialSpeed;
	RocketMovementComponent->MaxSpeed = InitialSpeed;
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	PathParams.MaxSimTime = 4.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.SimFrequency = 30.f;
	PathParams.StartLocation = GetActorLocation();
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);

	FPredictProjectilePathResult PathResult;

	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);


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

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
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
		if (Hit.GetActor() && Hit.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactPlayerCharacterNiagaraSystem, GetActorLocation(), GetActorRotation());
		}
		else if (Hit.GetActor() && !Hit.GetActor()->Implements<UInteractWithCrosshairsInterface>()) return;
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (ImpactPlayerCharacterSound)
	{
		if (Hit.GetActor() && Hit.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{ 
			UGameplayStatics::PlaySoundAtLocation(this, ImpactPlayerCharacterSound, GetActorLocation());
		}
		else if (Hit.GetActor() && !Hit.GetActor()->Implements<UInteractWithCrosshairsInterface>()) return;
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

void AProjectileRocket::Destroyed()
{
	
}

#if WITH_EDITOR
void AProjectileRocket::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileRocket, InitialSpeed))
	{
		if (RocketMovementComponent)
		{
			RocketMovementComponent->InitialSpeed = InitialSpeed;
			RocketMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif


