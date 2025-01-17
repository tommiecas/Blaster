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
#include "Components/CombatComponent.h" // Add this include to resolve the incomplete type error#inc
#include "PlayerController/FillainPLayerController.h"

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

void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectile, bHitPlayerCharacter);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
		AFillainCharacter* FillainCharacter = Cast<AFillainCharacter>(OtherActor);
		if (FillainCharacter && OtherActor && OtherActor->Implements<UInteractWithCrosshairsInterface>())
		{
			bHitPlayerCharacter = true;
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactPlayerCharacterParticles, GetActorTransform());
			AFillainPlayerController* InstigatorPlayerController = Cast<AFillainPlayerController>(GetInstigatorController());
			DamagedPawn = FillainCharacter;
			InstigatorFillainCharacter = Cast<AFillainCharacter>(GetInstigator());
			DidRocketLauncherKillFillain(InstigatorFillainCharacter, FillainCharacter);
		}
		Destroy();
}

void AProjectile::MulticastDestroy_Implementation()
{
	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AProjectile::DidRocketLauncherKillFillain(AFillainCharacter* MurderingPawn, AFillainCharacter* DeadPawn)
{
	AFillainCharacter* MurderingCharacter = Cast<AFillainCharacter>(MurderingPawn);
	DeadPawn = Cast<AFillainCharacter>(DamagedPawn); // Fix the class name here
	AFillainCharacter* MurderingFillain = Cast<AFillainCharacter>(MurderingPawn);
	if (MurderingFillain == nullptr) return false; // Add this check

	if (MurderingFillain->GetFillainPlayerController() == MurderingPawn->GetInstigatorController())
	{
		AWeapon* FiredWeapon = MurderingFillain->GetCombatComponent()->EquippedWeapon;
		if (bHitPlayerCharacter && MurderingFillain && FiredWeapon && FiredWeapon->GetWeaponType() == EWeaponType::EWT_RocketLauncher) // Fix the assignment operator to comparison operator
		{
			HandleRocketKilledOrMissedFillainSFX(MurderingFillain, DamagedPawn, MurderingPawn->GetInstigatorController());
			return bDeadByRocketLauncher = true;
		}
		else if (!bHitPlayerCharacter && FiredWeapon && FiredWeapon->GetWeaponType() != EWeaponType::EWT_RocketLauncher)
		{
			HandleOtherProjectileKilledOrMissedFillainSFX(MurderingFillain, DamagedPawn, MurderingPawn->GetInstigatorController());
			return bDeadByRocketLauncher = false;
		}
	}
	return false; // Ensure a return value for all control paths
}


void AProjectile::Destroyed()
{
	Super::Destroyed();
}

void AProjectile::HandleRocketKilledOrMissedFillainSFX(AFillainCharacter* KillerFillain, AFillainCharacter* VictimFillain, AController* )
{
	if (!bHitPlayerCharacter) return; // Add this check
	if (bHitPlayerCharacter == true)
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
	else if (bHitPlayerCharacter != true)
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
}

void AProjectile::HandleOtherProjectileKilledOrMissedFillainSFX(AFillainCharacter* KillerFillain, AFillainCharacter* VictimFillain, AController* InstigatorController)
{
	if (bHitPlayerCharacter == true)
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
	else if (bHitPlayerCharacter != true)
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
}
	