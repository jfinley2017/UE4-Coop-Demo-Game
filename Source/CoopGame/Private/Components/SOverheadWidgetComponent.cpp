// Fill out your copyright notice in the Description page of Project Settings.


#include "SOverheadWidgetComponent.h"
#include "SObservedUserWidget.h"
#include "SimpleLoggingLibrary.h"

void USOverheadWidgetComponent::SetWidgetVisibility(bool bIsVisible)
{
    if (!Widget)
    {
        return;
    }

    if (bIsVisible)
    {
        Widget->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        Widget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void USOverheadWidgetComponent::BeginPlay()
{
    Super::BeginPlay();

    USObservedUserWidget* WidgetAsObservedUserWidget = Cast<USObservedUserWidget>(Widget);

    if (!WidgetAsObservedUserWidget)
    {
        SimpleLog(LogTemp, Error, "SOverheadWidgetComponent attached to %s has a widget type which is not of type USObservedUserWidget. Use USObservedUserWidgets to automatically pass in observed actor to widget.", *GetNameSafe(GetOwner()));
        return;
    }

    WidgetAsObservedUserWidget->SetObserved(GetOwner());
}
