// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/PlayerChat.h"
#include "HUD/PlayerChatTextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void UPlayerChat::SetChatText(const FString& Text, const FString& PlayerName)
{
    const FString Chat = PlayerName + " : " + Text;

    if (ChatBoxClass)
    {
        OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayer() : OwningPlayer;
        if (OwningPlayer)
        {
            UPlayerChatTextBlock* ChatBoxWidget = CreateWidget<UPlayerChatTextBlock>(OwningPlayer, ChatBoxClass);
            if (InputScrollBox && ChatBoxWidget && ChatBoxWidget->ChatTextBlock)
            {
                ChatBoxWidget->ChatTextBlock->SetText(FText::FromString(Chat));
                ChatBoxWidget->ChatTextBlock->SetAutoWrapText(true);
                InputScrollBox->AddChild(ChatBoxWidget->ChatTextBlock); // This will add the TextBlock from WBP_ChatBox to the ScrollBox from top to bottom
                InputScrollBox->ScrollToEnd();
                InputScrollBox->bAnimateWheelScrolling = true;
            }
        }
    }
}