// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/EliminationAnnouncement.h"
#include "Components/TextBlock.h"

void UEliminationAnnouncement::SetEliminationAnnouncementText(FString KillerName, FString VictimmName)
{
	FString EliminationAnnouncementText = FString::Printf(TEXT("%s Eliminated %s!"), *KillerName, *VictimmName);
	if (AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(EliminationAnnouncementText));
	}
}
