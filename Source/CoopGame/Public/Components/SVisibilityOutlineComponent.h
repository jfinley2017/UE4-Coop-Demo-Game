// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SVisibilityOutlineComponent.generated.h"

class USVisionComponent;
class USkeletalMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USVisibilityOutlineComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USVisibilityOutlineComponent();

    // UActorComponent
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    // ~UActorComponent

protected:

    // UActorComponent
    virtual void BeginPlay() override;
	// ~UActorComponent

    UFUNCTION()
    virtual bool LocalPlayerHasLineOfSight();

    UFUNCTION()
    bool IsOwnerPlayer();

    UPROPERTY()
    USVisionComponent* CachedVisionComponent = nullptr;

    UPROPERTY()
    USkeletalMeshComponent* CachedSkeletalMeshComponent = nullptr;

};
