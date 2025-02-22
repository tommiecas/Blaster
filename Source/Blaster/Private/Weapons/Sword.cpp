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
