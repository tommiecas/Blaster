// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"

#include "Characters/FillainCharacter.h"
#include "Blaster/Blaster.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "Net/UnrealNetwork.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Weapons/Weapon.h" // Add this include to resolve the incomplete type error
#include "Weapons/WeaponTypes.h"
#include "HAFComponents/CombatComponent.h" // Add this include to resolve the incomplete type error#inc
#include "PlayerController/FillainPLayerController.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/DamageType.h"


AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);	
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);

	AmmoMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AmmoMesh"));
	AmmoMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AmmoMesh->SetupAttachment(CollisionBox);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (CascadeTracer && TracerCascadeComponent)
	{
		TracerCascadeComponent = UGameplayStatics::SpawnEmitterAttached(
			CascadeTracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}	
	else if (NiagaraTracer && TracerNiagaraComponent)
	{
        TracerNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            NiagaraTracer,
            CollisionBox,
            FName(),
            GetActorLocation(),
            GetActorRotation(),
            FVector(1.f),
            EAttachLocation::KeepWorldPosition,
            false,
            ENCPoolMethod::None
        );
	}

	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);	
	}
}


void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* DamagedActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	/* /Player was hit, but by what?
	ACharacter* KillerCharacter = Cast<ACharacter>(GetInstigator());
	if (KillerCharacter)
	{
		AFillainCharacter* KillerFillain = Cast<AFillainCharacter>(KillerCharacter);
		AWeapon* FiredWeapon = KillerFillain->GetCombatComponent()->EquippedWeapon;
		if (KillerFillain && DamagedActor && DamagedActor->Implements<UInteractWithCrosshairsInterface>() && FiredWeapon && FiredWeapon->GetWeaponType() == EWeaponType::EWT_RocketLauncher)
		{
			bHitPlayerCharacter = true;
			AFillainPlayerController* KillerController = Cast<AFillainPlayerController>(GetInstigatorController());
			AFillainCharacter* HitFillain = Cast<AFillainCharacter>(DamagedActor);
			HandlePostHitSFXDamagingPlayer();
			UDamageType const* const DamageType = UDamageType::StaticClass()->GetDefaultObject<UDamageType>();
			HitFillain->ReceiveDamage(KillerFillain, Damage, DamageType, KillerController, this);
		}
		else
		{
			bHitPlayerCharacter = false;
			bHitByRocketLauncher = false;
			bMissedByRocketLauncher = true;
			HandlePostHitSFXDamagingEnvironment();
			return;
		}
	}*/
	Destroy();
}

void AProjectile::SpawnTrailSystem()
{
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
}

void AProjectile::ExplodeDamage()
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
				DamageInnerRadius, //DamageInnerRadius
				DamageOuterRadius, //DamageOuterRadius
				1.f, // DamageFalloff
				UDamageType::StaticClass(), //DamageType
				IgnoreActors, //IgnoreActors
				this, //DamageCauser
				RocketFiringController //InstigatedBy
			);
		}
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::Destroyed()
{
	Super::Destroyed();
	FHitResult HitResult;
	if (ImpactParticles && (HitResult.GetActor() && !HitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	else if (ImpactPlayerCharacterParticles && (HitResult.GetActor() && HitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactPlayerCharacterParticles, GetActorTransform());
	}
	if (ImpactNiagaraSystem && (HitResult.GetActor() && !HitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagaraSystem, GetActorLocation(), GetActorRotation());
	}
	else if (ImpactPlayerCharacterNiagaraSystem && (HitResult.GetActor() && HitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactPlayerCharacterNiagaraSystem, GetActorLocation(), GetActorRotation());
	}
	if (ImpactSound && (HitResult.GetActor() && !HitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	else if (ImpactPlayerCharacterSound && (HitResult.GetActor() && HitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactPlayerCharacterSound, GetActorLocation());
	}
}

/*
void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectile, bHitPlayerCharacter);
}


void AProjectile::MulticastDestroy_Implementation()
{
	Destroy();
}



void AProjectile::HandlePostHitSFXDamagingPlayer()
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

void AProjectile::HandlePostHitSFXDamagingEnvironment()
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
*/