// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/GameplayComponents/TeamComponent.h"
#include "STeam.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UTeamComponent::UTeamComponent()
{
    SetIsReplicatedByDefault(true);
}

void UTeamComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UTeamComponent, Team);
}


// Called when the game starts
void UTeamComponent::BeginPlay()
{
	Super::BeginPlay();

}

uint8 UTeamComponent::GetTeamID() const
{
    return Team ? Team->GetTeamID() : INVALID_TEAM_NUM;
}

void UTeamComponent::SetTeam(ASTeam* NewTeam) 
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        return;
    }

    Team = NewTeam;
    OnRep_TeamChanged();
}

bool UTeamComponent::IsActorFriendly(AActor* ActorOne, AActor* ActorTwo)
{
    UTeamComponent* TeamCompOne = ActorOne->FindComponentByClass<UTeamComponent>();
    UTeamComponent* TeamCompTwo = ActorTwo->FindComponentByClass<UTeamComponent>();

    if (!TeamCompOne || !TeamCompTwo)
    {
        return false;
    }

    if (TeamCompOne->GetTeam() == TeamCompTwo->GetTeam())
    {
        return true;
    }
    return false;

}

bool UTeamComponent::IsActorTeamValid(AActor* AnActor)
{
	UTeamComponent* TeamComp = AnActor->FindComponentByClass<UTeamComponent>();

	if (!(TeamComp))
	{
		return false;
	}

	return TeamComp->GetTeamID() != INVALID_TEAM_NUM;
}

bool UTeamComponent::AreActorTeamsValid(AActor* ActorOne, AActor* ActorTwo)
{
	return IsActorTeamValid(ActorOne) && IsActorTeamValid(ActorTwo);
}

void UTeamComponent::OnRep_TeamChanged()
{
    OnTeamChanged.Broadcast(GetOwner(), Team);
}