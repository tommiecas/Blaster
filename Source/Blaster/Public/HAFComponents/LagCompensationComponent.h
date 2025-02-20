// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};


USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	AFillainCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AFillainCharacter*, uint32> HeadShots;

	UPROPERTY()
	TMap<AFillainCharacter*, uint32> BodyShots;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class AFillainCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	/**************************
	****    HITSCAN SSR    ****
	**************************/
	FServerSideRewindResult ServerSideRewind(AFillainCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLoccation, float HitTime);
	
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(AFillainCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);


	/*****************************
	****    PROJECTILE SSR    ****
	*****************************/
	FServerSideRewindResult ProjectileServerSideRewind(AFillainCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(AFillainCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	

	/**************************
	****    SHOTGUN SSR    ****
	**************************/
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<AFillainCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(const TArray<AFillainCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

	


	

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage GetFrameToCheck(AFillainCharacter* HitCharacter, float HitTime);

	/*****************************
	***   HITSCAN CONFIRMHIT   ***
	*****************************/
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, AFillainCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	
	/********************************
	***   PROJECTILE CONFIRMHIT   ***
	*********************************/
	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, AFillainCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	
	/*****************************
	***   SHOTGUN CONFIRMHIT   ***
	*****************************/
	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);
	
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	void CacheBoxPositions(AFillainCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AFillainCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AFillainCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(AFillainCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();

private:
	UPROPERTY()
	AFillainCharacter* Character;;

	UPROPERTY()
	class AFillainPlayerController* Controller;


	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
		
};
