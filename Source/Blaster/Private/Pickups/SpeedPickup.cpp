// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/SpeedPickup.h"
#include "Characters/FillainCharacter.h"
#include "HAFComponents/BuffComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AFillainCharacter* FillainCharacter = Cast<AFillainCharacter>(OtherActor);
	if (FillainCharacter)
	{
		UBuffComponent* BuffComponent = FillainCharacter->GetBuffComponent();
		if (BuffComponent)
		{
			BuffComponent->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}
	Destroy();
}
