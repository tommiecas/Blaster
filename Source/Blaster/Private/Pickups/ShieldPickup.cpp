// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/ShieldPickup.h"
#include "Characters/FillainCharacter.h"
#include "HAFComponents/BuffComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AShieldPickup::AShieldPickup()
{
	bReplicates = true;

}



void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AFillainCharacter* FillainCharacter = Cast<AFillainCharacter>(OtherActor);
	if (FillainCharacter)
	{
		UBuffComponent* BuffComponent = FillainCharacter->GetBuffComponent();
		if (BuffComponent)
		{
			BuffComponent->ReplenishShield(ShieldReplenishAmount, ShieldReplenishingTime);
		}
	}
	Destroy();
}
