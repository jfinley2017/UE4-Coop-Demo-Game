// Fill out your copyright notice in the Description page of Project Settings.


#include "SOverheadHealthbarComponent.h"
#include "SimpleLoggingLibrary.h"
#include "SVisionComponent.h"

void USOverheadHealthbarComponent::NotifyBecameVisible()
{
    SetWidgetVisibility(true);
}

void USOverheadHealthbarComponent::NotifyBecameHidden()
{
    SetWidgetVisibility(false);
}

void USOverheadHealthbarComponent::BeginPlay()
{
    Super::BeginPlay();

    USVisionComponent* OwnerVisionComponent = GetOwner()->FindComponentByClass<USVisionComponent>();
    if (!OwnerVisionComponent)
    {
        SimpleLog(LogTemp, Error, "Owner %s did not have a vision component. Cannot toggle widget based on vision.", *GetNameSafe(GetOwner()));
        return;
    }

    OwnerVisionComponent->OnBecameVisible.AddDynamic(this, &USOverheadHealthbarComponent::NotifyBecameVisible);
    OwnerVisionComponent->OnBecameHidden.AddDynamic(this, &USOverheadHealthbarComponent::NotifyBecameHidden);

}
