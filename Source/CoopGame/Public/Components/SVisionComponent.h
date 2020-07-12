// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SVisionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBecameVisibleSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBecameHiddenSignature);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USVisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
    
    UPROPERTY(BlueprintAssignable, Category = "SVisionComponent")
    FBecameVisibleSignature OnBecameVisible;

    UPROPERTY(BlueprintAssignable, Category = "SVisionComponent")
    FBecameHiddenSignature OnBecameHidden;

	// Sets default values for this component's properties
	USVisionComponent();

    // UActorComponent
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    // ~UActorComponent

    /**
     * Returns whether or not the owner is visible to the local player.
     */
    UFUNCTION(BlueprintPure, Category="SVisionComponent")
    virtual bool IsVisible();

protected:

    // UActorComponent
	virtual void BeginPlay() override;
    // ~UActorComponent

    bool bIsVisible;

};
