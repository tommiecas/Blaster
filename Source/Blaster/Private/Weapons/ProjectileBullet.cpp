// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Projectile.h"
#include "Sound/SoundCue.h"

#include "GameFramework/Character.h"
#include "Weapons/Weapon.h"
#include "Characters/FillainCharacter.h"
#include "HAFComponents/LagCompensationComponent.h"
#include "PlayerController/FillainPlayerController.h"
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
	BulletMovementComponent->InitialSpeed = InitialSpeed;
	BulletMovementComponent->MaxSpeed = InitialSpeed;
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
	/*
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
	*/
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
	AFillainCharacter* OwnerCharacter = Cast<AFillainCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		AFillainPlayerController* OwnerController = Cast<AFillainPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				const float DamageToCause = Hit.BoneName.ToString().Contains("head") ? HeadShotDamage : Damage;
				UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			AFillainCharacter* HitCharacter = Cast<AFillainCharacter>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensation()->ProjectileServerScoreRequest(
					HitCharacter,
					TraceStart,
					InitialVelocity,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime
				);
			}
		}
	}
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
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::Destroyed()
{

}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);
	
	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if (BulletMovementComponent)
		{
			BulletMovementComponent->InitialSpeed = InitialSpeed;
			BulletMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif


