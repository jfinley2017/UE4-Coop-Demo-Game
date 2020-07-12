// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SwarmComponent.generated.h"

class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPowerLevelChanged, float, NumActorsOverlapping, float, CurrentPowerLevel);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USwarmComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USwarmComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    UPROPERTY()
    USphereComponent* ProximitySphere = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "SwarmComponent")
    TArray<TSubclassOf<AActor>> PowerUpActors;
    UPROPERTY(EditDefaultsOnly, Category = "SwarmComponent")
    float ProximityRadius = 64.0f;
    UPROPERTY(EditDefaultsOnly, Category = "SwarmComponent")
    float PowerGainedPerActor = 0.0f;

    int32 NumOverlappingActors = 0;
    float PowerLevel = 0.0f;

    void InitializeOverlappingStuff();

public:	

    UFUNCTION()
    void OnProximityOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
    UFUNCTION()
    void OnProximityEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UPROPERTY(BlueprintAssignable)
    FPowerLevelChanged OnPowerLevelChanged;

    UFUNCTION(BlueprintPure, Category = "SwarmComponent")
    float GetPowerLevel() { return PowerLevel; }
    UFUNCTION(BlueprintPure, Category = "SwarmComponent")
    float GetNumberOverlappingActors() { return NumOverlappingActors; }

 
};
