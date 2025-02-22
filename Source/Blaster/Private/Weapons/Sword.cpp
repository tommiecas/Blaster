// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Sword.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

ASword::ASword()
{
	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
	SetRootComponent(SwordMesh);

	GetAreaSphere()->SetupAttachment(SwordMesh);
	GetPickupWidgetA()->SetupAttachment(SwordMesh);

	SwordMesh ->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SwordMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASword::DropWeapon()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	SwordMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	FillainOwnerCharacter = nullptr;
	FillainOwnerPlayerController = nullptr;
}

void ASword::OnEquipped()
{
	ShowPickupWidgets(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SwordMesh->SetSimulatePhysics(false);
	SwordMesh->SetEnableGravity(false);
	SwordMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnableCustomDepth(false);
}

void ASword::OnDropped()
{
	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	SwordMesh->SetSimulatePhysics(true);
	SwordMesh->SetEnableGravity(true);
	SwordMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SwordMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	SwordMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	SwordMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	SwordMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	SwordMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}
