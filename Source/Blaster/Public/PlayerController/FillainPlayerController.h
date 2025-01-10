// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Weapons/WeaponTypes.h"
#include "FillainPlayerController.generated.h"

struct FInputActionValue;
class AFillainCharacter;

/**
 * 
 */
UCLASS()
class BLASTER_API AFillainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 WeaponAmmo);
	void SetHUDCarriedAmmo(int32 CarriedAmmo);
	void SetHUDWeaponType(APawn* InPawn);
	void SetHUDEliminationMessage(AFillainPlayerController* ConstKillerController, AFillainPlayerController* ConstVictimController);
	void SetHUDMatchCountdown(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual float GetServerTime(); // Synced with server World clock
	virtual void ReceivedPlayer() override;

	UPROPERTY()
	AFillainCharacter* VictimCharacter;

	UPROPERTY()
	AFillainCharacter* KillerCharacter;

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputMappingContext* FillainMappingContext;

	/**************************************
	* Sync Time Between Clinet And Server *
	***************************************/

	//Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	//Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // Difference between Client and Server time
	
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;


	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

private:
	FString GetWeaponTypeDisplayName(EWeaponType WeaponType);

	UPROPERTY()
	class AFillainHUD* FillainHUD;

    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    class AFillainCharacter* Fillain;

    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    class AHAFPlayerState* State;

	UPROPERTY()
	class AWeapon* EquippedWeapon;

	float MatchTime = 120.f;
	uint32 CountdownInt;

	UPROPERTY()
	bool bIsMessageSet = true;

public:
	AFillainCharacter* GetFillain() const { return Fillain; }
	AHAFPlayerState* GetHAFPlayerState() const { return State; }

	



	
};
