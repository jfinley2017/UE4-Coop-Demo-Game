// Fill out your copyright notice in the Description page of Project Settings.

#include "STeam.h"
#include "Net/UnrealNetwork.h"
#include "TeamComponent.h"
#include "GameFramework/Controller.h"
#include "CoopGame.h"
#include "Engine/Engine.h"
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
    DOREPLIFETIME(ASTeam, MemberActors);
    DOREPLIFETIME(ASTeam, TeamID);
}

//////////////////////////////////
/** Getters/Setters */

void ASTeam::AddScore(float ScoreDelta)
{
    TeamScore += ScoreDelta;
    OnRep_TeamScore();
}

float ASTeam::GetScore()
{
    return TeamScore;
}

void ASTeam::SetTeamID(uint8 NewTeamID)
{
    TeamID = NewTeamID;
    OnTeamUpdated.Broadcast(this);
}

//////////////////////////////////
/** Team Add/Remove Operations */
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

    TRACE("PlayerJoinedTeam %d", TeamID);
    Members.Add(Controller);
    MemberStates.Add(Controller->PlayerState);
    PlayerJoinedTeam(Controller->PlayerState);
}

void ASTeam::AddActorToTeam(AActor* Actor)
{
    if (Role < ROLE_Authority) { return; }

    MemberActors.Add(Actor);
    ActorJoinedTeam(Actor);
}

void ASTeam::ActorJoinedTeam_Implementation(AActor* Actor)
{
    TRACE("ActorJoinedTeam %d", TeamID);
    OnActorJoined.Broadcast(this, Actor);
}

bool ASTeam::ActorJoinedTeam_Validate(AActor* Actor) { return true; }

void ASTeam::RemoveActorFromTeam(AActor * Actor)
{
    if (Role < ROLE_Authority) { return; }

    MemberActors.Remove(Actor);
    ActorLeftTeam(Actor);
}

void ASTeam::ActorLeftTeam_Implementation(AActor* Actor)
{
    TRACE("ActorLeftTeam %d", TeamID);
    OnActorLeft.Broadcast(this, Actor);
}

bool ASTeam::ActorLeftTeam_Validate(AActor* Actor) { return true; }

void ASTeam::PlayerJoinedTeam_Implementation(APlayerState* Player)
{
    OnMemberJoined.Broadcast(this, Player);
}

bool ASTeam::PlayerJoinedTeam_Validate(APlayerState* Player) { return true; }


void ASTeam::ClearTeam()
{
    MemberActors.Empty(); 
    MemberStates.Empty(); 
    Members.Empty(); 
}

//////////////////////////////////
/** Query Operations */
TArray<AActor*> ASTeam::GetActorsOfClass(TSubclassOf<AActor> ActorClass)
{
    TArray<AActor*> FoundActors;
    for (AActor* Actor : MemberActors)
    {
        if (Actor->IsA(ActorClass))
        {
            FoundActors.Add(Actor);
        }
    }
    return FoundActors;
}

TArray<AActor*> ASTeam::GetActorsOfClassMultiple(const TArray<TSubclassOf<AActor>>& Types)
{
    TArray<AActor*> FoundActors;

    if (Types.Num() == 0) { return FoundActors; }

    for (AActor* Actor : MemberActors)
    {
        if (Types.Contains(Actor->GetClass()))
        {
            FoundActors.Add(Actor);
        }
    }
    return FoundActors;
}

TArray<AActor*> ASTeam::GetActorsWithInterface(TSubclassOf<UInterface> Interface)
{
    TArray<AActor*> FoundActors;
    for (AActor* Actor : MemberActors)
    {
        if (Actor->GetClass()->ImplementsInterface(Interface))
        {
            FoundActors.Add(Actor);
        }
    }
    return FoundActors;
}

TArray<AActor*> ASTeam::GetActorsWithTag(FName Tag)
{
    TArray<AActor*> FoundActors;
    for (AActor* Actor : MemberActors)
    {
        if (Actor->ActorHasTag(Tag))
        {
            FoundActors.Add(Actor);
        }
    }
    return FoundActors;
}

void ASTeam::OnRep_TeamScore()
{
    OnTeamScoreChanged.Broadcast(this, TeamScore);
}
