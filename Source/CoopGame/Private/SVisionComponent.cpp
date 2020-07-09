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

    bIsVisible = true;
    
}


// Called every frame
void USVisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    for (APlayerState* Player : GetWorld()->GetGameState()->PlayerArray)
    {
        APawn* PlayerPawn = Player->GetPawn();
        if (!PlayerPawn)
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

