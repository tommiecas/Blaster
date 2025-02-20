// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerChatTextBlock.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UPlayerChatTextBlock : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ChatTextBlock;
};
