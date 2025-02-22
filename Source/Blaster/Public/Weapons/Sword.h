// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "Sword.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ASword : public AWeapon
{
	GENERATED_BODY()

public:
	ASword();
	virtual void DropWeapon() override;

protected:
	virtual void OnEquipped() override;
	virtual void OnDropped() override;

private: 
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SwordMesh;
	
};
