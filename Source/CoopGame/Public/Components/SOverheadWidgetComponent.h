// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "SOverheadWidgetComponent.generated.h"

/**
 * A component which passes information about its owner into a USObserbedUserWidget. 
 */
UCLASS(Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class COOPGAME_API USOverheadWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:

    /**
     * Sets @Widget's visibility to be either collapsed (bIsVisible = false) or visible (bIsVisible = true)
     */
    UFUNCTION(BlueprintCallable, Category = "SOverheadWidgetComponent")
    virtual void SetWidgetVisibility(bool bIsVisible);
	
protected:

    // UWidgetComponent
    virtual void BeginPlay() override;
	// ~UWidgetComponent
};
