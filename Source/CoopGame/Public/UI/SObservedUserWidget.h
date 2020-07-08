// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SObservedUserWidget.generated.h"

/**
 * Widget which is intended to observed state on a passed in actor.
 */
UCLASS()
class COOPGAME_API USObservedUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

    /**
     * Sets the currently observed actor to be @NewObserved.
     */
    UFUNCTION(BlueprintCallable, Category = "ObservedUserWidget")
    void SetObserved(AActor* NewObserved);
	
protected:
    
    /**
     * Hook for observed changed events.
     */
    UFUNCTION()
    virtual void NotifyObservedChanged(AActor* OldObserved, AActor* NewObserved);

    UPROPERTY()
    AActor* ObservedActor;

    /**
     * Blueprint hook for observed changed events.
     */
    UFUNCTION(BlueprintImplementableEvent)
    void ReceiveObservedChanged(AActor* OldObserved, AActor* NewObserved);
};
