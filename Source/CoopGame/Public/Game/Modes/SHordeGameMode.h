#pragma once

#include "CoreMinimal.h"
#include "SGameMode.h"
#include "SHordeGameMode.generated.h"

enum class EWaveState : uint8;
class ASHordeGameState;
class ASAffix;

/**
 * The base gamemode for Horde and other Wave-Spawning type GameModes. Should be used in conjunction with a SHordeGameState varient in order to
 * send information related to the number of enemies left in a wave/the current state of the wave/etc to clients.
 */
UCLASS()
class COOPGAME_API ASHordeGameMode : public ASGameMode
{
	GENERATED_BODY()
	
public:
    ASHordeGameMode();
    virtual void StartPlay() override;

protected:
	void SpawnPickup(FVector SpawnLoc);

    // Spawns a bot using the blueprint override
    UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
    void SpawnNewBot();

    /** Spawns a bot, called by @TimerHandle_BotSpawner. Will call EndWave if there are no more bots to spawn after this iteration */
    void SpawnBotTimerElapsed();

    /** Starts the spawning of the current wave. */
    void StartWave();
    
    /**Stops the spawning of the current wave. */
    void EndWave();

    /** Waits for the time designated by @TimeBetweenWaves to start the next wave */
	UFUNCTION(BlueprintCallable, Category = "GameMode")
    void PrepareForNextWave();

    /** Changes the WaveState, calls upon SHordeGameState which then shows the change to clients . */
    void SetWaveState(EWaveState State);

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	virtual bool ShouldSpawnRandomAffix();

    UFUNCTION(BlueprintCallable, Category ="GameMode")
    ASAffix* SpawnRandomAffix();

    /** Determines if there are any players or bots alive */
    UFUNCTION(BlueprintCallable, Category = "WaveState")
    void CheckWaveState();

    UFUNCTION(BlueprintCallable, Category = "WaveState")
    void CheckPlayerState();

    virtual void OnActorKilled_Implementation(AActor* KilledActor, AActor* KillerActor, AActor* DamageCauser) override;
	
    UFUNCTION(BlueprintCallable, Category = "HordeWaveStateSpawningWoWGreatCategory")
    void ApplyWaveAffixes();

    UFUNCTION(BlueprintCallable, Category = "HordeWaveStateSpawningWoWGreatCategory")
    void ApplyWaveAffixesToActor(AActor* Actor);

    // Height from ground
    UPROPERTY(EditDefaultsOnly, Category = "HordeMode")
    float PickupSpawnHeight = 50.0f;

    UPROPERTY(EditDefaultsOnly, Category = "HordeMode")
    TSubclassOf<AActor> PickupToSpawnOnDeath = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "HordeMode")
    ASHordeGameState* GameStateCache = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "HordeMode")
    TArray<TSubclassOf<ASAffix>> AllPossibleWaveAffixes;

    TArray<ASAffix*> SpawnedAffixes;

    /** The number of waves designated before the players win the game. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HordeMode")
    int32 NumberWaves = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HordeMode")
    int32 NumberBotsPerWave = 6;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HordeMode")
    uint8 PlayerTeamNumber = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HordeMode")
    uint8 HordeTeamNumber = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HordeMode")
    TArray<TSubclassOf<AActor>> HordeUnitTypes;

    UPROPERTY(EditDefaultsOnly, Category = "HordeMode")
    float TimeBetweenWaves = 0.0f;

    /** Counter which displays the current wave count, can be used to adjust wave strength or show players progress when compared to NumberWaves */
    int32 CurrentWaveCount = 0;

    // Integer used to control how many bots we are spawning in the current wave
    int32 NumberBotsToSpawn = 0;

    FTimerHandle TimerHandle_BotSpawner;
    FTimerHandle TimerHandle_NextWaveStart;

};
