// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionsSubsystem.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Add player to lobby
	// LobbyPlayers.Add(NewPlayer);
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);

		if (NumberOfPlayers == Subsystem->DesiredNumPublicConnections)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;

				FString MatchType = Subsystem->DesiredMatchType;
				if (MatchType == "BattleRoyale")
				{
					World->ServerTravel(FString("/Game/Maps/HAFBattleMap?listen"));

				}
				else if (MatchType == "Teams")
				{
					World->ServerTravel(FString("/Game/Maps/HAFTeamsMap?listen"));
				}
				else if (MatchType == "SeizeTheSword")
				{
					World->ServerTravel(FString("/Game/Maps/HAFSeizeTheSwordMap?listen"));
				}
			}
		}

	}
}

