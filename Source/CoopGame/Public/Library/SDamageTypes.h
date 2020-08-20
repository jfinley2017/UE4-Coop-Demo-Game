// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/PlayerState.h"
#include "SDamageTypes.generated.h"

/**
 * Represents an instance of damage.
 */
USTRUCT(BlueprintType)
struct FSDamageInstance
{

    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(BlueprintReadWrite)
    float Damage;

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AActor> Instigator;

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<APlayerState> Instigator_PlayerState;

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AActor> Receiver;

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<APlayerState> Receiver_PlayerState;

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AActor> DamageDealer;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> ContextTags;

};