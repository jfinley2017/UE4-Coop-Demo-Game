// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SFloatingText.generated.h"

class UWidgetComponent;
class ASDamageNumberService;

UCLASS()
class COOPGAME_API ASFloatingText : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASFloatingText();

    // AActor
	virtual void Tick(float DeltaTime) override;
    // ~AActor

    UFUNCTION()
    void Activate(const AActor* DamagedActor, const AActor* DamageInstigator, float Amount, const TArray<FString>& DamageTags);

    UFUNCTION(BlueprintCallable)
    void Deactivate();

    UFUNCTION()
    void Setup(ASDamageNumberService* DamageNumberService);

protected:

    // AActor
    virtual void BeginPlay() override;
    // ~AActor

    UPROPERTY(BlueprintReadOnly)
    ASDamageNumberService* CachedDamageNumberService;

    UPROPERTY(BlueprintReadOnly)
    bool bIsActive;

    /**
     * Blueprint hook for activation. Can do things like toggling visibility, formatting/setting text, etc.
     */
    UFUNCTION(BlueprintImplementableEvent)
    void ReceiveActivation(const AActor* DamagedActor, const AActor* DamageInstigator, float Amount, const TArray<FString>& DamageTags);

    /**
     * Blueprint hook for deactivation. Mostly used for resetting state so that this actor can be used again.
     */
    UFUNCTION(BlueprintImplementableEvent)
    void ReceiveDeactivation();

};
