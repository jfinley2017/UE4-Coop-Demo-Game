// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SOverheadWidgetComponent.h"
#include "SOverheadHealthbarComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class COOPGAME_API USOverheadHealthbarComponent : public USOverheadWidgetComponent
{
	GENERATED_BODY()
	
	
public:

    UFUNCTION()
    virtual void NotifyBecameVisible();

    UFUNCTION()
    virtual void NotifyBecameHidden();
	
protected:

    // UActorComponent
    virtual void BeginPlay() override;
    // ~UActorComponent

};
