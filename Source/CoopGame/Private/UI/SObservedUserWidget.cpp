// Fill out your copyright notice in the Description page of Project Settings.


#include "SObservedUserWidget.h"

void USObservedUserWidget::SetObserved(AActor* NewObserved)
{
    AActor* OldObservedActor = ObservedActor;
    ObservedActor = NewObserved;
    NotifyObservedChanged(OldObservedActor, NewObserved);
}

void USObservedUserWidget::NotifyObservedChanged(AActor* OldObserved, AActor* NewObserved)
{
    ReceiveObservedChanged(OldObserved, NewObserved);
}
