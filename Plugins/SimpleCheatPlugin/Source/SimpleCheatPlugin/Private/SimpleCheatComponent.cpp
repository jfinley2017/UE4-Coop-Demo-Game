// Copyright 2020 Joe Finley. All Rights Reserved.

#include "SimpleCheatComponent.h"
#include "SimpleCheatPlugin.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CheatManager.h"

// Sets default values for this component's properties
USimpleCheatComponent::USimpleCheatComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void USimpleCheatComponent::BeginPlay()
{
	Super::BeginPlay();

#if(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    EndCheatComponent();
    return;
#endif

    SetupDefaults();
	
}


void USimpleCheatComponent::SetupDefaults()
{
    APlayerController* OwnerAsPC = Cast<APlayerController>(GetOwner());
    if (!OwnerAsPC)
    {
        UE_LOG(LogSimpleCheatPlugin, Error, TEXT("Owner %s is not a Player Controller. SimpleCheatComponent should only be attached to Player Controllers."), *GetNameSafe(GetOwner()))
        return;
    }

    CachedOwningPlayerController = OwnerAsPC;
    OwnerAsPC->EnableCheats();

    UCheatManager* OwningCheatManager = OwnerAsPC->CheatManager;
    if (!OwningCheatManager)
    {
        UE_LOG(LogSimpleCheatPlugin, Error, TEXT("Owner %s does not have an instantiated CheatManager. Cheats cannot be ran without an instantiated CheatManager."), *GetNameSafe(GetOwner()))
        return;
    }

    CachedOwningCheatManager = OwningCheatManager;

}

void USimpleCheatComponent::EndCheatComponent()
{
    DestroyComponent();
}

void USimpleCheatComponent::ServerCheat_Implementation(const FString& CheatMsg)
{
    if (!CachedOwningCheatManager)
    {
        UE_LOG(LogSimpleCheatPlugin, Error, TEXT("Tried to run cheat %s but no CheatManager was found. Cheat could not be ran."));
    }

    if (!CachedOwningPlayerController)
    {
        UE_LOG(LogSimpleCheatPlugin, Error, TEXT("Tried to run cheat %s but no owning PlayerController was found. Cheat could not be ran."));
        return;
    }

    CachedOwningPlayerController->ConsoleCommand(CheatMsg, true);

}

bool USimpleCheatComponent::ServerCheat_Validate(const FString& CheatMsg)
{
    return true;
}

