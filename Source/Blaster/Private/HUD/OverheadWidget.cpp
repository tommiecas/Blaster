// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetRoleText(FString RoleTextToDisplay)
{
	if (RoleText)
	{
		RoleText->SetText(FText::FromString(RoleTextToDisplay));
	}
}

void UOverheadWidget::SetNameText(FString NameTextToDisplay)
{
	if (NameText)
	{
		NameText->SetText(FText::FromString(NameTextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString Role;
	switch (RemoteRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("Autonomous Proxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}
	FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	SetRoleText(RemoteRoleString);
}

void UOverheadWidget::ShowPlayerName(APawn* InPawn)
{
	if (InPawn == nullptr || InPawn->GetPlayerState() == nullptr)
	{
		// Handle the null case, maybe log an error or set a default player name
		SetNameText(FString("Unknown Player"));
		return;
	}

	FString PlayerName = InPawn->GetPlayerState()->GetPlayerName();
	SetNameText(PlayerName);
}

void UOverheadWidget::NativeConstruct()
{
	RemoveFromParent();
	Super::NativeConstruct();

}
