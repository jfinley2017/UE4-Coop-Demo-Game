// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "SGameMode.h"
#include "SGameState.h"
#include "CoopGame.h"
#include "SPlayerState.h"
#include "Engine/Engine.h"
#include "TeamComponent.h"
#include "Services/DamageNumbers/SDamageNumberService.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
    SetIsReplicatedByDefault(true);
	// ...
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USHealthComponent, Health);
    DOREPLIFETIME(USHealthComponent, MaxHealth);
    DOREPLIFETIME(USHealthComponent, LastDamageTaken);
}

// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

    // Only perform if we are the server
    if (GetOwnerRole() == ROLE_Authority)
    {
        Health = MaxHealth;
        OnRep_Health();
    }
}

bool USHealthComponent::ShouldApplyDamage(const FSDamageInstance& Damage)
{
    ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
    bool bIsFriendlyFire = UTeamComponent::IsActorFriendly(Damage.Instigator.Get(), Damage.Receiver.Get());
    bool bIsLegalFriendlyFire =  !bIsFriendlyFire || (bIsFriendlyFire && GM->bIsFriendlyFireEnabled) || Damage.ContextTags.Contains("Suicide");
    bool bIsDamage = Damage.Damage >= 0.0f;
    return bIsLegalFriendlyFire && bIsDamage;
}

void USHealthComponent::BroadcastDamageEvents(const FSDamageInstance& DamageTaken)
{
    OnDamageTaken.Broadcast(DamageTaken);

    if (DamageTaken.ContextTags.Contains("Lethal"))
    {
        OnDied.Broadcast(DamageTaken);
    }

    // Tell the other actor that it successfully applied damage
    if (DamageTaken.Instigator.Get())
    {
        USHealthComponent* OtherHealthComponent = DamageTaken.Instigator.Get()->FindComponentByClass<USHealthComponent>();
        if (OtherHealthComponent)
        {
            OtherHealthComponent->OnDamageDealt.Broadcast(DamageTaken);
        }
    }

    ASDamageNumberService* DamageNumberService = ASDamageNumberService::GetDamageNumberService(GetWorld());
    if (DamageNumberService)
    {
        DamageNumberService->DisplayDamageNumber(DamageTaken.Receiver.Get(), DamageTaken.Instigator.Get(), DamageTaken.Damage, LastDamageTaken.ContextTags);
    }

    // Update damage and damage taken stats
    APawn* Damaged = Cast<APawn>(DamageTaken.Receiver.Get());
    APawn* Damager = Cast<APawn>(DamageTaken.Receiver.Get());
    if (Damaged && Damaged->GetPlayerState())
    {
        ASPlayerState* PS = Cast<ASPlayerState>(Damaged->GetPlayerState());
        PS->AddDamageTaken(DamageTaken.Damage);
    }

    if (Damager && Damager->GetPlayerState())
    {
        ASPlayerState* PS = Cast<ASPlayerState>(Damager->GetPlayerState());
        PS->AddDamage(DamageTaken.Damage);
    }
}

void USHealthComponent::Heal(float HealAmount)
{
    if (HealAmount <= 0.0f || Health <= 0.0f)
    {
        return;
    }

    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(this, Health, MaxHealth);
}

bool USHealthComponent::ApplyDamage(FSDamageInstance Damage)
{
    if (!ShouldApplyDamage(Damage))
    {
        return false;
    }

    ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());

    float DamageAmount = Damage.Damage;
    Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
    OnRep_Health();

    if (Health <= 0 && !bIsDead)
    {
        bIsDead = true;
        Damage.ContextTags.Add("Lethal");

        OnDied.Broadcast(Damage);
        if (GM && Damage.Instigator.Get() && GetOwner())
        {
            GM->OnActorKilled(GetOwner(), Damage.Instigator.Get(), nullptr);
        }
    }

    LastDamageTaken = Damage;
    OnRep_LastDamageTaken();

    return true;
}

void USHealthComponent::OnRep_Health()
{
    OnHealthChanged.Broadcast(this,Health, MaxHealth);
}

void USHealthComponent::OnRep_MaxHealth()
{
    OnHealthChanged.Broadcast(this, Health, MaxHealth);
}

void USHealthComponent::OnRep_LastDamageTaken()
{
    // Determine if damage is valid
    if (LastDamageTaken.Timestamp < 0.0f || GetWorld()->GetTimeSeconds() - LastDamageTaken.Timestamp >= 5.0f)
    {
        return;
    }

    BroadcastDamageEvents(LastDamageTaken);
}
