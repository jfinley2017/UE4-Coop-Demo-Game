// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGameMode.h"
#include "SHordeGameMode.generated.h"


enum class EWaveState : uint8;

/**
 * 
 */
UCLASS()
class COOPGAME_API ASHordeGameMode : public ASGameMode
{
	GENERATED_BODY()
	


public:
    ASHordeGameMode();
    virtual void StartPlay() override;

protected:

    // Wave spawning
    UPROPERTY(EditDefaultsOnly, Category = "WaveMode")
    int32 NumberWaves = 1;

    // Preparation time between waves
    UPROPERTY(EditDefaultsOnly, Category = "WaveMode")
    float TimeBetweenWaves = 0.0f;

    int32 CurrentWaveCount = 0;

    // Integer used to control how many bots we are spawning in the current wave
    int32 NumberBotsToSpawn = 0;

    FTimerHandle TimerHandle_BotSpawner;
    FTimerHandle TimerHandle_NextWaveStart;

    // Spawns a bot using the blueprint override
    UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
    void SpawnNewBot();

    void SpawnBotTimerElapsed();

    void StartWave();

    void EndWave();

    void PrepareForNextWave();

    void SetWaveState(EWaveState State);

    void CheckWaveState();
	
    //UFUNCTION(BlueprintNativeEvent, Category = "GameMode")
    //void OnActorKilled(AActor* KilledActor, AActor* KillerActor, AController* KillerController);
    virtual void OnActorKilled_Implementation(AActor* KilledActor, AActor* KillerActor, AController* KillerController) override;
	
};
