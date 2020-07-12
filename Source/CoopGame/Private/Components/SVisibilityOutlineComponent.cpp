// Fill out your copyright notice in the Description page of Project Settings.


#include "SVisibilityOutlineComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "SVisionComponent.h"

// Sets default values for this component's properties
USVisibilityOutlineComponent::USVisibilityOutlineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USVisibilityOutlineComponent::BeginPlay()
{
	Super::BeginPlay();

    CachedVisionComponent = GetOwner()->FindComponentByClass<USVisionComponent>();
    CachedSkeletalMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
}


bool USVisibilityOutlineComponent::LocalPlayerHasLineOfSight()
{
    APlayerController* LocalPlayer = UGameplayStatics::GetPlayerController(this, 0);
    if (LocalPlayer)
    {
        return LocalPlayer->LineOfSightTo(GetOwner());
    }
    return false;
}

bool USVisibilityOutlineComponent::IsOwnerPlayer()
{
    APawn* OwnerAsPawn = Cast<APawn>(GetOwner());
    if (OwnerAsPawn)
    {
        return OwnerAsPawn->GetPlayerState();
    }

    return false;
}

// Called every frame
void USVisibilityOutlineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CachedVisionComponent && CachedVisionComponent->IsVisible() && !LocalPlayerHasLineOfSight())
    {
        if (CachedSkeletalMeshComponent)
        {
            CachedSkeletalMeshComponent->SetRenderCustomDepth(true);
            int32 StencilValue = IsOwnerPlayer() ? 252 : 254;
            CachedSkeletalMeshComponent->SetCustomDepthStencilValue(StencilValue);
            return;
        }
    }

    if (CachedSkeletalMeshComponent && CachedSkeletalMeshComponent->bRenderCustomDepth)
    {
        CachedSkeletalMeshComponent->SetRenderCustomDepth(false);
    }
}

