#include "SHordeGameMode.h"
#include "CoopGame.h"
#include "SPlayerState.h"
#include "STeamManager.h"
#include "SFunctionLibrary.h"
#include "STeam.h"
#include "TeamComponent.h"
#include "SPawn.h"
#include "Gameplay/GameplayActors/SAffix.h"
#include "SHordeGameState.h"

ASHordeGameMode::ASHordeGameMode()
{
    GameStateClass = ASHordeGameState::StaticClass();
    PlayerStateClass = ASPlayerState::StaticClass();
}

void ASHordeGameMode::StartPlay()
{
    Super::StartPlay();

    GameStateCache = GetGameState<ASHordeGameState>();
    ensure(GameStateCache);

    GameStateCache->GetTeamManager()->CreateTeam(PlayerTeamNumber);
    GameStateCache->PlayerTeam = GameStateCache->GetTeamManager()->GetTeam(PlayerTeamNumber);
    
	GameStateCache->GetTeamManager()->CreateTeam(HordeTeamNumber);
    GameStateCache->HordeTeam = GameStateCache->GetTeamManager()->GetTeam(HordeTeamNumber);
    
	GameStateCache->OnRep_HordeTeam();
}

void ASHordeGameMode::StartWave()
{
    // Update the current wave count
    ensure(GameStateCache);

    CurrentWaveCount++;
    GameStateCache->SetCurrentWaveNumber(CurrentWaveCount);

    SetWaveState(EWaveState::WaitingToComplete);
    NumberBotsToSpawn = NumberBotsPerWave * CurrentWaveCount;
    GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASHordeGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);
}

void ASHordeGameMode::SpawnBotTimerElapsed()
{
    SpawnNewBot();
    NumberBotsToSpawn--;
    if (NumberBotsToSpawn <= 0)
    {
        EndWave();
    }
}

void ASHordeGameMode::EndWave()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
}

void ASHordeGameMode::PrepareForNextWave()
{
    ensure(GameStateCache);

    RestartDeadPlayers();

	if (ShouldSpawnRandomAffix())
	{
		SpawnRandomAffix();
	}

    if (CurrentWaveCount == NumberWaves && NumberWaves > 0)
    {
        GameOver(GameStateCache->PlayerTeam);
        return;
    }

    // Spawn next wave immediately
    if (TimeBetweenWaves == 0)
    {
        StartWave();
    }
    else
    {
        // Spawn the next wave on a delay
        GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASHordeGameMode::StartWave, TimeBetweenWaves, false);
        GameStateCache->SetNextWaveStartTime(GetWorld()->TimeSeconds + TimeBetweenWaves);
    }

    SetWaveState(EWaveState::WaitingToStart);
}

// Iterate over all of the pawns, if we find a bot and its still alive then carry on
// This should really be generalized more into a team format somehow? broadcast win/lose conditions based on teamID
void ASHordeGameMode::CheckWaveState()
{
    ensure(GameStateCache);

    if (NumberBotsToSpawn > 0 || GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart))
    {
        return;
    }

    ASTeam* HordeTeam = GameStateCache->GetTeamManager()->GetTeam(HordeTeamNumber);
    if (!HordeTeam) { return; }

    // Perhaps in the future its best to have an interface for mobs that can be wave mobs.
    TArray<AActor*> WaveMobsAlive = HordeTeam->GetActorsWithTag("WaveMob");
    if (WaveMobsAlive.Num() <= 0)
    {
        HordeTeam->ClearTeam();

        PrepareForNextWave();
        return;
    }
}

void ASHordeGameMode::CheckPlayerState()
{
    ensure(GameStateCache);

    TArray<AActor*> PlayerTeamActors = GameStateCache->PlayerTeam->GetActorsWithTag("WaveRelevant");
    if (PlayerTeamActors.Num() == 0)
    {
        GameStateCache->MulticastGameOver(GameStateCache->HordeTeam);
    }
}

// Changes the state of the wave spawner
void ASHordeGameMode::SetWaveState(EWaveState State)
{
    ensure(GameStateCache);

    GameStateCache->SetWaveState(State);
   
}

void ASHordeGameMode::SpawnPickup(FVector SpawnLoc)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AActor>(PickupToSpawnOnDeath, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
}

// Called when a player has died, used in order to avoid using tick to check player/wave state
void ASHordeGameMode::OnActorKilled_Implementation(AActor* KilledActor, AActor* KillerActor, AActor* DamageCauser)
{
    Super::OnActorKilled_Implementation(KilledActor, KillerActor, DamageCauser);
   
	if (PickupToSpawnOnDeath)
	{
		FVector FloorLoc;

		if (USFunctionLibrary::FindFloor(GetWorld(), KilledActor->GetActorLocation(), FloorLoc))
		{
			SpawnPickup(FloorLoc + FVector(0.0f, 0.0f, PickupSpawnHeight));
		}
	}
	
    // Update the involved players scores
    if (KilledActor != KillerActor)
    {
        // fix this
        APawn* Killer = Cast<APawn>(KillerActor);
        ISPawn* Killed = Cast<ISPawn>(KilledActor);
        if (Killer && Killed && Killer->GetPlayerState())
        {
            // Score for the player
            ASPlayerState* PS = Cast<ASPlayerState>(Killer->GetPlayerState());
            PS->AddScore(Killed->GetOnKillScore());
            
            // Score for the team
            UTeamComponent* PSTeamComp = PS->FindComponentByClass<UTeamComponent>();
            if (PSTeamComp && PSTeamComp->GetTeam())
            {
                PSTeamComp->GetTeam()->AddScore(Killed->GetOnKillScore());
            }
        }
    }
    
    // If the killed actor was controlled by something, increase death count on the playerstate
    APawn* Killed = Cast<APawn>(KilledActor);
    if (Killed && Killed->GetPlayerState())
    {
        ASPlayerState* PS = Cast<ASPlayerState>(Killed->GetPlayerState());
        if (PS)
        {
            PS->AddDeaths(1);
        }
    }

    // Determine if the game/wave should end by checking Horde/Player teams
    CheckWaveState();
    CheckPlayerState();
}


void ASHordeGameMode::ApplyWaveAffixes()
{
    ensure(GameStateCache);

    if (AllPossibleWaveAffixes.Num() <= 0)
    {
        return;
    }

    ASTeam* HordeTeam = GameStateCache->HordeTeam;
    TArray<AActor*> HordeTeamPawns = GameStateCache->HordeTeam->GetActorsWithTag("WaveMob");
    
    for (AActor* HordeWaveActor : HordeTeamPawns)
    {
        if (HordeWaveActor)
        {
            ApplyWaveAffixesToActor(HordeWaveActor);
        }
    }
}

void ASHordeGameMode::ApplyWaveAffixesToActor(AActor* Actor)
{
    if (!Actor) { return; }

    for (ASAffix* Affix : SpawnedAffixes)
    {
        Affix->ApplyToActor(Actor);
    }
}

// Could be overridden in BP to base off of a data table, the time of day, mic input..
bool ASHordeGameMode::ShouldSpawnRandomAffix()
{
	
	return CurrentWaveCount % 3 == 1;
}

ASAffix* ASHordeGameMode::SpawnRandomAffix()
{
    ensure(GameStateCache);

    if (AllPossibleWaveAffixes.Num() <= 0) { return nullptr; }

    TSubclassOf<ASAffix> Chosen = AllPossibleWaveAffixes[FMath::RandRange(0, AllPossibleWaveAffixes.Num() - 1)];
    ASAffix* Affix = GetWorld()->SpawnActor<ASAffix>(Chosen);
    SpawnedAffixes.Add(Affix);
    AllPossibleWaveAffixes.Remove(Chosen);

    GameStateCache->AddAffix(Affix);

    return Affix;
}
