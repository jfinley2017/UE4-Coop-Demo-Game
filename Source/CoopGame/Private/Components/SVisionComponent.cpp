// Fill out your copyright notice in the Description page of Project Settings.


#include "SVisionComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameState.h"

// Sets default values for this component's properties
USVisionComponent::USVisionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USVisionComponent::BeginPlay()
{
	Super::BeginPlay();

    bIsVisible = false;
    
}


// Called every frame
void USVisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // The owner is a player, players are always visible.
    APawn* OwnerAsPawn = Cast<APawn>(GetOwner());
    if (OwnerAsPawn && OwnerAsPawn->GetPlayerState() && !OwnerAsPawn->GetPlayerState()->bIsABot)
    {
        bIsVisible = true;
        OnBecameVisible.Broadcast();
        SetComponentTickEnabled(false);
        return;
    }

    // We are an AI, test to see if any players in the game has LoS on us
    for (APlayerState* Player : GetWorld()->GetGameState()->PlayerArray)
    {
        APawn* PlayerPawn = Player->GetPawn();
        if (!PlayerPawn || Player->bIsABot || Player->bIsSpectator)
        {
            continue;
        }

        FHitResult LineOfSightTraceHitResult;
        FCollisionQueryParams LineOfSightQueryParams;
        LineOfSightQueryParams.AddIgnoredActor(GetOwner());
        LineOfSightQueryParams.AddIgnoredActor(PlayerPawn);
        GetWorld()->LineTraceSingleByChannel(LineOfSightTraceHitResult, GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation(), ECollisionChannel::ECC_Visibility, LineOfSightQueryParams);

        if (!LineOfSightTraceHitResult.bBlockingHit)
        {
            if (!bIsVisible)
            {
                bIsVisible = true;
                OnBecameVisible.Broadcast();
            }   
            return;
        }
    }

    // No players have LoS on us.
    if (bIsVisible)
    {
        bIsVisible = false;
        OnBecameHidden.Broadcast();
    }
}

bool USVisionComponent::IsVisible()
{
    return bIsVisible;
}

