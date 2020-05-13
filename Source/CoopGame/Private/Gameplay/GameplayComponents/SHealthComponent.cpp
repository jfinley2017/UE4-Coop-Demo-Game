// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "SGameMode.h"
#include "SGameState.h"
#include "CoopGame.h"
#include "SPlayerState.h"
#include "Engine/Engine.h"
#include "Gameplay/GameplayComponents/TeamComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
    SetIsReplicated(true);
	// ...
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USHealthComponent, Health);
    DOREPLIFETIME(USHealthComponent, MaxHealth);
}

// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

    // Only perform if we are the server
    if (GetOwnerRole() == ROLE_Authority)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            Owner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeDamage);
        }
        Health = MaxHealth;
        OnHealthChanged.Broadcast(this, Health, 0, nullptr, nullptr, nullptr);
    }
}

// This only runs on server because we are only binding to this delegate on server
void USHealthComponent::HandleTakeDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
    ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());

    // Find the ACTOR responsible for the damage which was dealt
    // Why all the trouble? Controllers don't stay attached to pawns on death, thus on occasion InstigatedBy->GetPawn is invalid
    AActor* DamageInstigatorActor = DetermineDamageInstigatorActor(InstigatedBy, DamageCauser);

    if ( Damage <= 0 
        || bIsDead 
        || (!(DamageCauser == DamageInstigatorActor) && !GM->bIsFriendlyFireEnabled && UTeamComponent::IsActorFriendly(DamagedActor, DamageInstigatorActor))
        || (!bDamageSelf && DamageCauser == DamageInstigatorActor))
    {
        return;
    }

    Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

    // @DEBUGLOG Damage 
    FString InstigatedByS = DamageInstigatorActor ? *DamageInstigatorActor->GetName() : NULLSTRING;
    FString DamageCauserS = DamageCauser ? *DamageCauser->GetName() : NULLSTRING;
    TRACE("%s inflicts %f damage on %s using %s . Current Health: %f",
        *InstigatedByS, 
        Damage, 
        *GetOwner()->GetName(), 
        *DamageCauserS,
        Health);


    BroadcastRelevantDamageEvents(DamagedActor, Damage, DamageType, InstigatedBy, DamageInstigatorActor, DamageCauser);

    if (Health <= 0 && !bIsDead)
    {
        OnKilled.Broadcast(GetOwner());
        if (GM && DamageInstigatorActor && GetOwner())
        {
            TRACE("%s", *DamageCauser->GetName());
            GM->OnActorKilled(GetOwner(), DamageInstigatorActor, DamageCauser);
        }
        bIsDead = true;
    }

}
// Run on owning client here i guess to displlay damage numbers?

void USHealthComponent::BroadcastRelevantDamageEvents(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor* DamageInstigatorActor, AActor * DamageCauser)
{
    OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

    // Tell the other actor that it successfully applied damage
    if (DamageInstigatorActor)
    {
        USHealthComponent* OtherHealthComponent = DamageInstigatorActor->FindComponentByClass<USHealthComponent>();
        if (OtherHealthComponent)
        {
            OtherHealthComponent->OnDamageDealt.Broadcast(DamageInstigatorActor, GetOwner(), DamageCauser, Damage);
        }
    }

    // Update damage and damage taken stats
    APawn* Damaged = Cast<APawn>(GetOwner());
    APawn* Damager = Cast<APawn>(DamageInstigatorActor);
    if (Damaged && Damaged->GetPlayerState())
    {
        ASPlayerState* PS = Cast<ASPlayerState>(Damaged->GetPlayerState());
        PS->AddDamageTaken(Damage);
    }

    if (Damager && Damager->GetPlayerState())
    {
        ASPlayerState* PS = Cast<ASPlayerState>(Damager->GetPlayerState());
        PS->AddDamage(Damage);
    }
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
    float Damage = OldHealth - Health;
    TRACE("%s Health changed. Current Health: %f. Damage: %f", *GetOwner()->GetName(), Health, Damage);
    OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::OnRep_MaxHealth()
{
    OnHealthChanged_Minimal.Broadcast(this);
}

void USHealthComponent::Heal(float HealAmount)
{
    if (HealAmount <= 0.0f || Health <= 0.0f)
    {
        return;
    }

    TRACE("%s healed for %f hp.", *GetOwner()->GetName(), HealAmount);
    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

// Attempts to find an actor responsible for the damage caused
AActor* USHealthComponent::DetermineDamageInstigatorActor(AController* DamageInstigator, AActor* DamageCauser)
{
    if (DamageInstigator && DamageInstigator->GetPawn())
    {
        return DamageInstigator->GetPawn();
    }
    else if (DamageCauser && DamageCauser->GetOwner())
    {
         return DamageCauser->GetOwner();
    }
    else
    {
        // Occasionally the actor responsible is the actor itself (the case in any suicidal units)
        return DamageCauser;
    }
}
