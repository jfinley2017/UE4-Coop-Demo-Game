// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TeamComponent.generated.h"

#define INVALID_TEAM_NUM 255

class ASTeam;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTeamComponentOnTeamChanged, AActor*, Actor, ASTeam*, NewTeam);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API UTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTeamComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_TeamChanged)
    ASTeam* Team;

    UFUNCTION()
    void OnRep_TeamChanged();

public:	
	
    UPROPERTY(BlueprintAssignable, Category = "Team")
    FTeamComponentOnTeamChanged OnTeamChanged;

    UFUNCTION(BlueprintCallable, Category = "Team")
    ASTeam* GetTeam() const { return Team; }

    UFUNCTION(BlueprintCallable, Category = "Team")
    uint8 GetTeamID() const;

    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeam(ASTeam* NewTeam);

    UFUNCTION(BlueprintCallable, Category = "Team")
    static bool IsActorFriendly(AActor* ActorOne, AActor* ActorTwo);

	static bool IsActorTeamValid(AActor* AnActor);

	// consider extending this to work for N actors
	static bool AreActorTeamsValid(AActor* ActorOne, AActor* ActorTwo);
};
