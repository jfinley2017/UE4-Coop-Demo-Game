// Copyright 2020 Joe Finley. All Rights Reserved.

#include "SimpleCheatManager.h"
#include "SimpleCheatPlugin.h"
#include "SimpleCheatComponent.h"
#include "GameFramework/PlayerController.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void USimpleCheatManager::SimpleCheatTest()
{
    if (!HasAuthority())
    {
        ServerCheat(FString::Printf(TEXT("%s"), *FUNC_NAME));
        // Do not return early, allow client to also run this code. 
    }

    FString AuthorityPrefix = HasAuthority() ? "Authority: " : "Client: ";
    FColor Color = HasAuthority() ? FColor::Cyan : FColor::Green;
    SendScreenMessage(Color, FString::Printf(TEXT("%sSimpleCheatTest"), *AuthorityPrefix));
}

void USimpleCheatManager::SimpleCheatTest_Server()
{
    if (!HasAuthority())
    {
        ServerCheat(FString::Printf(TEXT("%s"), *FUNC_NAME));
        // Return early so that client doesn't run this. This will be the most common behavior in most games.
        return;
    }

    FString AuthorityPrefix = HasAuthority() ? "Authority: " : "Client: ";
    FColor Color = HasAuthority() ? FColor::Cyan : FColor::Green;
    SendScreenMessage(Color, FString::Printf(TEXT("%sSimpleCheatTest"), *AuthorityPrefix));
}

void USimpleCheatManager::SimpleCheatTest_Client()
{
    FString AuthorityPrefix = HasAuthority() ? "Authority: " : "Client: ";
    FColor Color = HasAuthority() ? FColor::Cyan : FColor::Green;
    SendScreenMessage(Color, FString::Printf(TEXT("%sSimpleCheatTest"), *AuthorityPrefix));
}

void USimpleCheatManager::ServerCheat(const FString& CheatMsg)
{
    if (HasAuthority())
    {
        return;
    }

    // Route through component
    GetOuterSimpleCheatComponent()->ServerCheat(CheatMsg);

}

bool USimpleCheatManager::HasAuthority()
{
    return GetOuterAPlayerController()->HasAuthority();
}

UWorld* USimpleCheatManager::GetWorld() const
{
    return GetOuterAPlayerController()->GetWorld();
}

void USimpleCheatManager::SendScreenMessage(FColor Color, FString Message)
{
    GEngine->AddOnScreenDebugMessage(-1, 1.0f, Color, Message);
}

USimpleCheatComponent* USimpleCheatManager::GetOuterSimpleCheatComponent()
{
    if (CachedOuterSimpleCheatComponent)
    {
        return CachedOuterSimpleCheatComponent;
    }

    APlayerController* OuterPC = GetOuterAPlayerController();
    if (!OuterPC)
    {
        UE_LOG(LogSimpleCheatPlugin, Error, TEXT("Outer PlayerController not found. Cannot retrieve SimpleCheatComponent. This should never happen."));
        return nullptr;
    }

    USimpleCheatComponent* OuterSimpleCheatComponent = OuterPC->FindComponentByClass<USimpleCheatComponent>();
    if (!OuterSimpleCheatComponent)
    {
        UE_LOG(LogSimpleCheatPlugin, Error, TEXT("Outer SimpleCheatComponent not found. Add a USimpleCheatComponent to the outer PlayerController."));
        return nullptr;
    }

    CachedOuterSimpleCheatComponent = OuterSimpleCheatComponent;
    return CachedOuterSimpleCheatComponent;
}
