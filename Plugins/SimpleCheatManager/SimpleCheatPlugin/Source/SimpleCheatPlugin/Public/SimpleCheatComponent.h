// Copyright 2020 Joe Finley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimpleCheatComponent.generated.h"

class APlayerController;
class UCheatManager;

/**
 * A component which handles common operations that the CheatManager itself cannot handle.
 *
 * It assumes that it is attached to a PlayerController and logs when it finds that it is not.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIMPLECHEATPLUGIN_API USimpleCheatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

    USimpleCheatComponent();

    /**
     * Passes a cheat message to the server. Handles executing the cheat message.
     */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerCheat(const FString& CheatMsg);

protected:

    // UActorComponent
    virtual void BeginPlay() override;
    // ~UActorComponent

    /**
     * Sets up the defaults for this component.
     * Typically will set cached values and verify assumptions.
     */
    virtual void SetupDefaults();

    /**
     * Ends the cheat component early, used if we currently don't want cheats enabled.
     */
    virtual void EndCheatComponent();

   UPROPERTY()
   APlayerController* CachedOwningPlayerController;

   UPROPERTY()
   UCheatManager* CachedOwningCheatManager;
};
