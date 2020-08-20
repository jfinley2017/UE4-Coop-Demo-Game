// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Library/SDamageTypes.h"
#include "SHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageTakenSignature, const FSDamageInstance&, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageDealtSignature, const FSDamageInstance&, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiedSignature, const FSDamageInstance&, Damage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChangedSignature, USHealthComponent*, HealthComp, float, Health, float, MaxHealth);

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

    /**
     * Fired whenever the health value of this component changes.
     */
    UPROPERTY(BlueprintAssignable, Category = "HealthComponentEvents")
    FOnHealthChangedSignature OnHealthChanged;

    /**
     * Fired whenever this component has died, typically health <= 0.
     */
    UPROPERTY(BlueprintAssignable, Category = "HealthComponent")
    FOnDiedSignature OnDied;
   
    /**
     * Fired whenever this component applied damage.
     */
    UPROPERTY(BlueprintAssignable, Category = "HealthComponent")
    FOnDamageDealtSignature OnDamageDealt;

    /**
     * Fired whenever this component took damage.s
     */
    UPROPERTY(BlueprintAssignable, Category = "HealthComponent")
    FOnDamageTakenSignature OnDamageTaken;

	// Sets default values for this component's properties
	USHealthComponent();

    /**
     * Heals this HealthComponent, HealAmount must be > 0
     */
    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    void Heal(float HealAmount);
    
    UFUNCTION()
    bool ApplyDamage(FSDamageInstance Damage);

    UFUNCTION(BlueprintPure, Category = "HealthComponent")
    float GetHealth() const { return Health; }

    
protected:

    // UActorComponent
	virtual void BeginPlay() override;
    // ~UActorComponent

    /**
     * Determines whether or not we should apply the specified damage instance to ourselves.
     */
    UFUNCTION()
    bool ShouldApplyDamage(const FSDamageInstance& Damage);

    UFUNCTION()
    void BroadcastDamageEvents(const FSDamageInstance& DamageTaken);

    UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadWrite, Category = "HealthComponent")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_MaxHealth, Category = "HealthComponent")
    float MaxHealth = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_LastDamageTaken, BlueprintReadWrite, Category = "HealthComponent")
    FSDamageInstance LastDamageTaken;
    
    bool bIsDead = false;

    /**
     * Should the actor which this component is attached to be allowed to damage itself?
     */
    UPROPERTY(EditDefaultsOnly, Replicated, BlueprintReadOnly, Category = "HealthComponent")
    bool bDamageSelf = true;

    UFUNCTION()
    void OnRep_Health();

    UFUNCTION()
    void OnRep_MaxHealth();

    UFUNCTION()
    void OnRep_LastDamageTaken();
};
