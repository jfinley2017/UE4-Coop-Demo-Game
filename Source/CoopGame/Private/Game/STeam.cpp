// Fill out your copyright notice in the Description page of Project Settings.

#include "STeam.h"
#include "Net/UnrealNetwork.h"
#include "Gameplay/GameplayComponents/TeamComponent.h"
#include "GameFramework/Controller.h"
#include "CoopGame.h"
#include "GameFramework/PlayerState.h"

ASTeam::ASTeam()
{
    SetReplicates(true);
    bAlwaysRelevant = true;
}


void ASTeam::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASTeam, MemberStates);
    DOREPLIFETIME(ASTeam, TeamID);
    DOREPLIFETIME(ASTeam, NumberEnemiesLeft);
}

void ASTeam::SetTeamID(uint8 NewTeamID)
{
    TeamID = NewTeamID;
    OnTeamUpdated.Broadcast(this);
}

void ASTeam::AddToTeam(AController* Controller)
{
    // Don't run this on clients. Only server functions are able to change teams.
    if (Role < ROLE_Authority) { return; }

    for (int i = 0; i < Members.Num(); i++)
    {
        if (Members[i] == Controller)
        {
            return;
        }
    }

    Members.Add(Controller);
    MemberStates.Add(Controller->PlayerState);
    PlayerJoinedTeam(Controller->PlayerState);
}

void ASTeam::AddActorToTeam(AActor* Actor)
{
    if (Role < ROLE_Authority) { return; }

    MemberActors.Add(Actor);
}

void ASTeam::RemoveActorFromTeam(AActor * Actor)
{
    if (Role < ROLE_Authority) { return; }

    MemberActors.Remove(Actor);
}

void ASTeam::PlayerJoinedTeam_Implementation(APlayerState* Player)
{
    OnMemberJoined.Broadcast(this, Player);
}

bool ASTeam::PlayerJoinedTeam_Validate(APlayerState* Player)
{
    return true;
}

void ASTeam::ActorJoinedTeam_Implementation(AActor* Actor)
{
    OnActorJoined.Broadcast(this, Actor);
}

bool ASTeam::ActorJoinedTeam_Validate(AActor* Actor)
{
    return true;
}