// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/SDamageLibrary.h"
#include "Components/SHealthComponent.h"
#include "Model.h"
#include "Kismet/KismetSystemLibrary.h"

bool USDamageLibrary::DealDamage(FSDamageInstance& DamageInstance)
{
    if (!DamageInstance.Receiver.Get() || !DamageInstance.Instigator.Get())
    {
        // Log here
        return false;
    }

    USHealthComponent* DamagedHealthComponent = DamageInstance.Receiver.Get()->FindComponentByClass<USHealthComponent>();
    if (DamagedHealthComponent)
    {
        return DamagedHealthComponent->ApplyDamage(DamageInstance);
    }

    return false;
}

void USDamageLibrary::ApplyRadialDamage(UObject* WorldContextObject, const FVector& CenterLocation, float Radius, const TArray<AActor*>& IgnoredActors, float Damage, AActor* Instigator, APlayerState* Instigator_PlayerState, const TArray<FString>& ContextTags)
{

    if (!WorldContextObject || !WorldContextObject->GetWorld())
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    TArray<AActor*> Overlaps;

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeQuery;
    UKismetSystemLibrary::SphereOverlapActors(WorldContextObject, CenterLocation, Radius, ObjectTypeQuery, nullptr, TArray<AActor*>(), Overlaps);

    for (AActor* Overlapped : Overlaps)
    {
        FSDamageInstance NewDamageInstance;
        NewDamageInstance.ContextTags = ContextTags;
        NewDamageInstance.Damage = Damage;
        NewDamageInstance.Instigator = Instigator;
        NewDamageInstance.Instigator_PlayerState = Instigator_PlayerState;
        NewDamageInstance.Timestamp = World->GetTimeSeconds();
        NewDamageInstance.Receiver = Overlapped;
        DealDamage(NewDamageInstance);
    }
    
    

}
