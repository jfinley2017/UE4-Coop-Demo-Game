// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SDamageNumberService.generated.h"

class ASFloatingText;

/**
 * Provides the primary interface for accessing floating damage numbers.
 * Manages the pool for floating damage number actors.
 */
UCLASS(Blueprintable)
class COOPGAME_API ASDamageNumberService : public AActor
{
	GENERATED_BODY()
	
public:	

	// Sets default values for this actor's properties
	ASDamageNumberService();

    // AActor
	virtual void Tick(float DeltaTime) override;
    // ~AActor

    /**
     * Primary API call to show a damage number at @DamagedActor's location. 
     */
    UFUNCTION(BlueprintCallable, Category = "DamageNumberService")
    void DisplayDamageNumber(const AActor* DamagedActor, const AActor* DamageInstigator, float DamageAmount, const TArray<FString> DamageTags);

    /**
     * Retrieves the world's current DamageNumberService. 
     */
    UFUNCTION(BlueprintPure, meta=(WorldContext="WorldContextObject"), Category = "DamageNumberService")
    static ASDamageNumberService* GetDamageNumberService(UObject* WorldContextObject);

   
protected:
    
    // AActor
    virtual void BeginPlay() override;
    // ~AActor

    /**
     * Returns a ASFloatingText actor to the pool.
     */
    UFUNCTION()
    void ReturnToPool(ASFloatingText* FloatingTextObject);

    /**
     * Determine the location at which the floating damage number should be spawned. Could be used to prevent damage number overlaps via slight offsets.
     * Override by overriding FindDisplayLocation_Implementation.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "DamageNumberService")
    FVector FindDamageNumberActivationLocation(const AActor* DamagedActor, const AActor* DamageInstigator, float DamageAmount, const TArray<FString>& DamageTags);

    /**
     * Grabs a ASFloatingText actor from the pool, if the pool is not empty. Does nothing if the pool is empty.
     */
    UFUNCTION()
    ASFloatingText* RetrieveFromPool();


    /**
     * Returns whether or not the damage number service should be ran, considering things like netmode.
     */
    UFUNCTION()
    bool ShouldRunDamageService();

    /**
     * ASFloatingText pool, spawned in BeginPlay and maintained by ReturnToPool and RetrieveFromPool
     */
    UPROPERTY()
    TArray<ASFloatingText*> FloatingTextPool;

    /**
     * Num pooled actors to spawn at BeginPlay
     */
    UPROPERTY(EditAnywhere, Category = "CoopGame")
    int32 InitialPooledTextActors;

    /**
     * Class of ASFloatingText to spawn.
     */
    UPROPERTY(EditAnywhere, Category = "CoopGame")
    TSubclassOf<ASFloatingText> DamageNumberClass;

    friend class ASFloatingText;
};
