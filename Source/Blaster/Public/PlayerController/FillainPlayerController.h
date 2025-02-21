// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Weapons/WeaponTypes.h"
#include "FillainPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

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
	AFillainPlayerController();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 WeaponAmmo);
	void SetHUDCarriedAmmo(int32 CarriedAmmo);
	void SetHUDWeaponType(APawn* InPawn);
	void SetHUDEliminationMessage(AFillainPlayerController* ConstKillerController, AFillainPlayerController* ConstVictimController);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);

	void HideTeamScores();
	void InitTeamScores();
	void SetHUDRedTeamScore(int32 RedScore);
	void SetHUDBlueTeamScore(int32 BlueScore);

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual float GetServerTime(); // Synced with server World clock
	virtual void ReceivedPlayer() override;

	float MatchTimeElapsedTime = 0.f;
	bool bIsMatchCountdownVisible = true;
	FLinearColor MatchCountdownColor;
	FLinearColor MatchCountdownBlinkingColor;

	void UpdateMatchCountdownColor();
	void ToggleMatchCountdownVisibility();

	UPROPERTY()
	AFillainCharacter* VictimCharacter;

	UPROPERTY()
	AFillainCharacter* KillerCharacter;

	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void HandleCooldown();

	float SingleTripTime = 0.f;

	UPROPERTY()
	class AWeapon* EquippedWeapon;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElimination(APlayerState* Killer, APlayerState* Victim);

	void AddPlayerChatTextBlock();

	UFUNCTION()
	void ToggleInputChatBox();

	UFUNCTION()
	void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION(Server, Reliable)
	void ServerSetText(const FString& Text, const FString& PlayerName);

	UFUNCTION(Client, Reliable)
	void ClientSetText(const FString& Text, const FString& PlayerName);

	UPROPERTY()
	class AHAFGameMode* GameMode;

private:
	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UPlayerChat> PlayerChatClass;

	UPROPERTY()
	UPlayerChat* PlayerChatWidget;

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputMappingContext* FillainMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* QuitAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* ChatAction;

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

	void PollInit();

	virtual void SetupInputComponent() override;
	void ShowReturnToMainMenu();

	void HandleMatchHasStarted(bool bTeamsMatch = false);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);

	UFUNCTION(Client, Reliable)
	void ClientEliminationAnnouncement(APlayerState* Killer, APlayerState* Victim);

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();
	FString GetInfoText(TArray<class AHAFPlayerState*>& Players);
	FString GetTeamsInfoText (class AHAFGameState* HAFGameState);

private:
	FString GetWeaponTypeDisplayName(EWeaponType WeaponType);

	UPROPERTY()
	class AFillainHUD* FillainHUD;

	/**********************************
	****    RETURN TO MAIN MENU    ****
	**********************************/

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;

    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    class AFillainCharacter* Fillain;

    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    class AHAFPlayerState* State;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	class AHAFGameMode* Mode;



	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float LevelStartingTime = 0.f;
	float CooldownTime = 0.f;
	int32 CountdownInt;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	bool bInitializeHealth = false;
	bool bInitializeScore = false;
	bool bInitializeDefeats = false;
	bool bInitializeGrenades = false;
	bool bInitializeShield = false;
	bool bInitializeWeaponAmmo = false;
	bool bInitializeCarriedAmmo = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;
	int32 HUDGrenades;
	float HUDShield;
	float HUDMaxShield;
	float HUDCarriedAmmo;
	float HUDWeaponAmmo;

	int32 ThirtySecondsOnTheClock = 30;

	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;

public:
	FORCEINLINE AFillainCharacter* GetFillain() const { return Fillain; }
	FORCEINLINE AHAFPlayerState* GetHAFPlayerState() const { return State; }
	FORCEINLINE float GetMatchTime() const { return MatchTime; }

	



	
};
