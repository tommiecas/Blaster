// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Characters/FillainCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Weapons/Casing.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PlayerController/FillainPlayerController.h"
#include "HAFComponents/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "HUD/PickupWidget.h"
#include "Components/WidgetComponent.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	Component = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(Component);	

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Component);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(Component);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidgetA = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidgetA"));
	PickupWidgetA->SetupAttachment(Component);
	PickupWidgetA->SetWidgetClass(UPickupWidget::StaticClass()); // Set the widget class to UPickupWidget

	PickupWidgetB = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidgetB"));
	PickupWidgetB->SetupAttachment(Component);
	PickupWidgetB->SetWidgetClass(UPickupWidget::StaticClass()); // Set the widget class to UPickupWidget
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	if (PickupWidgetA)
	{
		PickupWidgetA->SetVisibility(false);
	}
	if (PickupWidgetB)
	{
		PickupWidgetB->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedCOmponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFillainCharacter* FillainCharacter = Cast<AFillainCharacter>(OtherActor);
	if (FillainCharacter)
	{
		if (WeaponType == EWeaponType::EWT_Sword && FillainCharacter->GetTeam() == Team) return;
		if (FillainCharacter->IsWieldingTheSword()) return;
		FillainCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappingCOmponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AFillainCharacter* FillainCharacter = Cast<AFillainCharacter>(OtherActor);
	if (FillainCharacter)
	{
		if (WeaponType == EWeaponType::EWT_Sword && FillainCharacter->GetTeam() != Team) return;
		if (FillainCharacter->IsWieldingTheSword()) return;
		FillainCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetHUDAmmo()
{
	FillainOwnerCharacter = FillainOwnerCharacter == nullptr ? Cast<AFillainCharacter>(GetOwner()) : FillainOwnerCharacter;
	if (FillainOwnerCharacter)
	{
		FillainOwnerPlayerController = FillainOwnerPlayerController == nullptr ? Cast<AFillainPlayerController>(FillainOwnerCharacter->Controller) : FillainOwnerPlayerController;
		if (FillainOwnerPlayerController)
		{
			FillainOwnerPlayerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::FireSingleRoundOfAmmo()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	FillainOwnerCharacter = FillainOwnerCharacter == nullptr ? Cast<AFillainCharacter>(GetOwner()) : FillainOwnerCharacter;
	if (FillainOwnerCharacter && FillainOwnerCharacter->GetCombatComponent() && IsWeaponFull())
	{
		FillainOwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		FillainOwnerCharacter = nullptr;
		FillainOwnerPlayerController = nullptr;
	}
	else
	{
		FillainOwnerCharacter = FillainOwnerCharacter == nullptr ? Cast<AFillainCharacter>(Owner) : FillainOwnerCharacter;
		if (FillainOwnerCharacter && FillainOwnerCharacter->GetEquippedWeapon() && FillainOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

void AWeapon::OnEquipped()
{
	HidePickupWidgets();
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);

	FillainOwnerCharacter = FillainOwnerCharacter == nullptr ? Cast<AFillainCharacter>(GetOwner()) : FillainOwnerCharacter;
	if (FillainOwnerCharacter && bUseServerSideRewind)
	{
		FillainOwnerPlayerController = FillainOwnerPlayerController == nullptr ? Cast<AFillainPlayerController>(FillainOwnerCharacter->Controller) : FillainOwnerPlayerController;
		if (FillainOwnerPlayerController && HasAuthority() && !FillainOwnerPlayerController->HighPingDelegate.IsBound())
		{
			FillainOwnerPlayerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	FillainOwnerCharacter = FillainOwnerCharacter == nullptr ? Cast<AFillainCharacter>(GetOwner()) : FillainOwnerCharacter;
	if (FillainOwnerCharacter)
	{
		FillainOwnerPlayerController = FillainOwnerPlayerController == nullptr ? Cast<AFillainPlayerController>(FillainOwnerCharacter->Controller) : FillainOwnerPlayerController;
		if (FillainOwnerPlayerController && HasAuthority() && FillainOwnerPlayerController->HighPingDelegate.IsBound())
		{
			FillainOwnerPlayerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnEquippedSecondary()
{
	HidePickupWidgets();
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();
	}

	FillainOwnerCharacter = FillainOwnerCharacter == nullptr ? Cast<AFillainCharacter>(GetOwner()) : FillainOwnerCharacter;
	if (FillainOwnerCharacter)
	{
		FillainOwnerPlayerController = FillainOwnerPlayerController == nullptr ? Cast<AFillainPlayerController>(FillainOwnerCharacter->Controller) : FillainOwnerPlayerController;
		if (FillainOwnerPlayerController && HasAuthority() && FillainOwnerPlayerController->HighPingDelegate.IsBound())
		{
			FillainOwnerPlayerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::ShowPickupWidgets()
{
	if (PickupWidgetA)
	{
		PickupWidgetA->SetVisibility(true);
	}
	if (PickupWidgetB)
	{
		PickupWidgetB->SetVisibility(true);
	}
}

void AWeapon::HidePickupWidgets()
{
	if (PickupWidgetA)
	{
		PickupWidgetA->SetVisibility(false);
	}
	if (PickupWidgetB)
	{
		PickupWidgetB->SetVisibility(false);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEjectSocket"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}
	FireSingleRoundOfAmmo();
}

void AWeapon::DropWeapon()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	FillainOwnerCharacter = nullptr;
	FillainOwnerPlayerController = nullptr;
}

bool AWeapon::IsWeaponEmpty()
{
	return Ammo <= 0;
}

bool AWeapon::IsWeaponFull()
{
	return Ammo == MagCapacity;
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlashSocket");
	if (MuzzleFlashSocket == nullptr) return FVector();
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	/* DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan,
		true);*/

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}



