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
	void ResetSword();

protected:
	virtual void OnEquipped() override;
	virtual void OnDropped() override;
	virtual void BeginPlay() override;

private: 
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SwordMesh;

	FTransform InitialTransform;

public:
	FORCEINLINE FTransform GetInitialTransform() const { return InitialTransform; }
	
};
