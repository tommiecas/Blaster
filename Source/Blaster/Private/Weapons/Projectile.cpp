// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"

#include "Characters/FillainCharacter.h"
#include "Blaster/Blaster.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "Net/UnrealNetwork.h"

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
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);


	AmmoMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AmmoMesh"));
	AmmoMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	AmmoMesh->SetupAttachment(CollisionBox);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;

}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
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
		}
		MulticastDestroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::Destroyed()
{
	Super::Destroyed();
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
		else if (bHitPlayerCharacter == false)
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

	void AProjectile::MulticastDestroy_Implementation()
	{
		Destroy();
	}

