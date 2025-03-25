// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Sword.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Characters/FillainCharacter.h"
#include "HUD/PickupWidget.h"


#include "Components/WidgetComponent.h"  

ASword::ASword()  
{  
   SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));  
   SetRootComponent(SwordMesh);  

   GetAreaSphere()->SetupAttachment(SwordMesh);  
   // Assuming GetPickupWidgetA() returns a UUserWidget or a subclass of it  
   if (UWidgetComponent* WidgetComponent = GetPickupWidgetA())  
   {  
       if (UPickupWidget* PickupWidget = Cast<UPickupWidget>(WidgetComponent->GetUserWidgetObject()))  
       {  
           PickupWidget->AddToViewport();  
       }  
   }  

   SwordMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);  
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

void ASword::ResetSword()
{
	AFillainCharacter* SwordWielder = Cast<AFillainCharacter>(GetOwner());
	if (SwordWielder)
	{
		SwordWielder->SetWieldingTheSword(false);
		SwordWielder->SetOverlappingWeapon(nullptr);
		SwordWielder->UnCrouch();
	}

	if (!HasAuthority()) return;
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	SwordMesh->DetachFromComponent(DetachRules);
	SetWeaponState(EWeaponState::EWS_Initial);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SetOwner(nullptr);
	FillainOwnerCharacter = nullptr;
	FillainOwnerPlayerController = nullptr;

	SetActorTransform(InitialTransform);
}



void ASword::OnEquipped()
{
	ShowPickupWidgets();
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SwordMesh->SetSimulatePhysics(false);
	SwordMesh->SetEnableGravity(false);
	SwordMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SwordMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
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

void ASword::BeginPlay()
{
	Super::BeginPlay();
	InitialTransform = GetActorTransform();
}
