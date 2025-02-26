// Fill out your copyright notice in the Description page of Project Settings.


#include "SeizeTheSword/SwordZone.h"
#include "Components/SphereComponent.h"
#include "Weapons/Sword.h"
#include "GameMode/SeizeTheSwordGameMode.h"

ASwordZone::ASwordZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	SetRootComponent(ZoneSphere);

}

void ASwordZone::BeginPlay()
{
	Super::BeginPlay();

	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &ASwordZone::OnSphereOverlap);
	
}

void ASwordZone::OnSphereOverlap(UPrimitiveComponent* OverlappedCOmponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASword* OverlappingSword = Cast<ASword>(OtherActor);
	if (OverlappingSword && OverlappingSword->GetTeam() != Team)
	{
		ASeizeTheSwordGameMode* GameMode = GetWorld()->GetAuthGameMode<ASeizeTheSwordGameMode>();
		if (GameMode)
		{
			GameMode->SwordSeized(OverlappingSword, this);
		}
		OverlappingSword->ResetSword();
	}
}



