// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Library/SDamageTypes.h"
#include "SDamageLibrary.generated.h"


/**
 * 
 */
UCLASS()
class COOPGAME_API USDamageLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Damage")
	static bool DealDamage(FSDamageInstance& DamageInstance);
	
    UFUNCTION(BlueprintCallable, Category = "Damage")
    static void ApplyRadialDamage(UObject* WorldContextObject, const FVector& CenterLocation, float Radius, const TArray<AActor*>& IgnoredActors, float Damage, AActor* Instigator, APlayerState* Instigator_PlayerState, const TArray<FString>& ContextTags);
};
