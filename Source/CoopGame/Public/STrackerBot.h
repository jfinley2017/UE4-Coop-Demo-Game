#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DamageDealer.h"
#include "SPawn.h"
#include "STrackerBot.generated.h"

class USHealthComponent;
class UMaterialInstanceDynamic;
class USphereComponent;
class UTeamComponent;
class USoundCue;

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn, public ISPawn, public IDamageDealer
{
	GENERATED_BODY()

public:
	ASTrackerBot();

protected:

    // AActor
	virtual void BeginPlay() override;
    // ~AActor

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComp = nullptr;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    USHealthComponent* HealthComp = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UTeamComponent* TeamComp = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USphereComponent* ProximityExplosionRadius = nullptr;

    // Pulse material
    UMaterialInstanceDynamic* MatInstance = nullptr;

    UFUNCTION()
    void NotifyHealthChanged(USHealthComponent * ChangedHealthComp, float Health, float MaxHealth);

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float ExplosionRadius = 200;
    
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float ProximityRadius = 150;
    
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float ExplosionDamage = 100;
    
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float SelfDestructTime = 3.0f;
    
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float MovementForce = 5000;
	
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MaxSpeed = 1000;

	// For ISPawn IncreaseSpeed interface method
	float OriginalMovementForce = MovementForce;
	float OriginalMaxSpeed = MaxSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    bool bUseVelocityChange = false;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float RequiredDistanceToTarget = 100;
	// Recalc the navigation path every x seconds
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RefreshInterval = 0.5f;
    
    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    USoundCue* SelfDestructTickSound = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    USoundCue* ExplodeSound = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    USoundCue* TriggeredSound = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    UParticleSystem* ExplosionEffect = nullptr;

	// If true, bot will teleport on to player's back once in range
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bBotAttachesToPlayer = false;

	AActor* FindBestTarget();

    FVector GetNextPathPoint();
    FVector NextPathPoint;
    void MoveTowardsTarget(float DeltaTime);

    bool bTriggered = false;

    UPROPERTY(ReplicatedUsing=OnRep_Exploded)
    bool bExploded = false;

    UPROPERTY(ReplicatedUsing=OnRep_TrackerBotAttached)
    bool bSelfDestructionAttached = false;

    UPROPERTY(ReplicatedUsing = OnRep_ExplodeTime)
    float ExplodeTime = 0.0f;

    UFUNCTION()
    void OnRep_TrackerBotAttached();

    UFUNCTION()
    void OnRep_ExplodeTime();

    UFUNCTION()
    void OnRep_Exploded();

	/**
	 * @param Pawn to attach to. Must have a "Back" socket
	 * @returns False if bot did not attach for any reason
	 */
	UFUNCTION()
	bool AttachBotToActor(APawn* OtherPawn);

    FTimerHandle SelfDestructCountdownTimer;
    FTimerHandle SelfDestructionTickTimer;
    FTimerHandle TimerHandle_RefreshPath;
    
    UFUNCTION()
    void SelfDestruct();
    UFUNCTION()
    void SelfDestructTick();

    UPROPERTY(BlueprintReadWrite, Category = "Power")
    float DamageModifier = 1.0f;

    UFUNCTION()
    void OnProximityRadiusOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

    UFUNCTION()
    void RefreshPath();

    UPROPERTY(EditDefaultsOnly, Category = "Scoring")
    float OnKillPoints = 0.0f;

public:	
	UFUNCTION(BlueprintImplementableEvent, Category = "TrackerBot")
	void Exploded();

	UFUNCTION(BlueprintImplementableEvent, Category = "TrackerBot")
	void SelfDestructTicked(float TimeTillDestruct);

	virtual void IncreaseMovespeed(float PercentIncrease);

	virtual void Tick(float DeltaTime) override;

    // IDamageDealer
    virtual float GetDamageModifier() override { return DamageModifier; }

    // ISPawn
    virtual FVector GetSize() override;

    virtual float GetOnKillScore() override { return OnKillPoints; }
};
