// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, USHealthComponent*, HealthComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKilled, AActor*, KilledActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDealtDamage, AActor*, DamageDealer, AActor*, DamageReciever, AActor*, DamageCauser, float, Damage);

USTRUCT(BlueprintType)
struct FDamageContext
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly)
    float Timestamp;
    
    UPROPERTY(BlueprintReadOnly)
    float Damage;

    UPROPERTY(BlueprintReadOnly)
    AActor* DamageInstigator;

    UPROPERTY(BlueprintReadOnly)
    AActor* DamageReceiver;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> DamageTags;
    
};

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

    UPROPERTY(BlueprintAssignable, Category = "HealthComponent")
    FOnHealthChanged OnHealthChanged_Minimal;

    UPROPERTY(BlueprintAssignable, Category = "HealthComponentEvents")
    FOnHealthChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "DamageEventDelegates")
    FOnKilled OnKilled;

    UPROPERTY(BlueprintAssignable, Category = "DamageEventDelegates")
    FOnDealtDamage OnDamageDealt;

	// Sets default values for this component's properties
	USHealthComponent();

    /** Should the actor which this component is attached to be allowed to damage itself? */
    UPROPERTY(EditDefaultsOnly, Replicated, BlueprintReadOnly, Category = "HealthComponent")
    bool bDamageSelf = true;

    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    float GetHealth() const { return Health; }

protected:

    // UActorComponent
	virtual void BeginPlay() override;
    // ~UActorComponent

    UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadWrite, Category = "HealthComponent")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_MaxHealth, Category = "HealthComponent")
    float MaxHealth = 100.0f;
    
    UPROPERTY(ReplicatedUsing=OnRep_LastDamageTaken, BlueprintReadWrite, Category = "HealthComponent")
    FDamageContext LastDamageTaken;

    UFUNCTION()
    void HandleTakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION()
    void BroadcastRelevantDamageEvents(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor* DamageInstigatorActor, AActor * DamageCauser);

    UFUNCTION()
    void OnRep_Health(float OldHealth);

    UFUNCTION()
    void OnRep_MaxHealth();

    UFUNCTION()
    void OnRep_LastDamageTaken();

    bool bIsDead = false;

    /** Used to determine who is responsible for damage. 
        Basically just picks one of DamageInstigator->GetPawn(), DamagerCauser->GetOwner(), DamageCauser based on which is valid
        trying each in that order. */
    AActor * DetermineDamageInstigatorActor(AController * DamageInstigator, AActor * DamageCauser);
    
};
