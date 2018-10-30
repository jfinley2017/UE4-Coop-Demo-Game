// Fill out your copyright notice in the Description page of Project Settings.

#include "SwarmComponent.h"
#include "CoopGame.h"
#include "Components/SphereComponent.h"

USwarmComponent::USwarmComponent()
{
    ProximitySphere = CreateDefaultSubobject<USphereComponent>(TEXT("ProximitySphere"));
    if (ProximitySphere)
    {
        ProximitySphere->OnComponentBeginOverlap.AddDynamic(this, &USwarmComponent::OnProximityOverlap);
        ProximitySphere->OnComponentEndOverlap.AddDynamic(this, &USwarmComponent::OnProximityEndOverlap);
        //ProximitySphere->SetCollisionProfileName("Sensor");
        //ProximitySphere->SetCollisionObjectType(COLLISION_SENSOR);
        ProximitySphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        ProximitySphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap);
    }
}

// Called when the game starts
void USwarmComponent::BeginPlay()
{
    Super::BeginPlay();
    ProximitySphere->SetSphereRadius(ProximityRadius, true);
    ProximitySphere->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

}

//TODO: Find a  way to avoid ever calling this
void USwarmComponent::InitializeOverlappingStuff()
{
    TArray<UPrimitiveComponent*> OverlappingComponents;
    ProximitySphere->GetOverlappingComponents(OverlappingComponents);

    for (int32 i = 0; i < OverlappingComponents.Num(); i++)
    {
        for (int32 i = 0; i < PowerUpActors.Num(); i++)
        {
            if (OverlappingComponents[i]->GetOwner()->IsA(PowerUpActors[i]))
            {
                NumOverlappingActors++;
                PowerLevel = NumOverlappingActors * PowerGainedPerActor;
                OnPowerLevelChanged.Broadcast(NumOverlappingActors, PowerLevel);
            }
        }
    }
}

void USwarmComponent::OnProximityOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
        
    if (!OtherActor || OtherActor == GetOwner()) { return; }

    for (int32 i = 0; i < PowerUpActors.Num(); i++)
    {

        if (OtherActor->IsA(PowerUpActors[i]))
        {
            NumOverlappingActors++;
            PowerLevel = NumOverlappingActors * PowerGainedPerActor;
            TRACE("SwarmIncrease. Actor: %s, Overlapped: %s, PowerLevel: %f", *GetOwner()->GetName(), *OtherActor->GetName(), PowerLevel)
            OnPowerLevelChanged.Broadcast(NumOverlappingActors, PowerLevel);
        }
    }
}


void USwarmComponent::OnProximityEndOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor  || OtherActor == GetOwner()) { return; }

    for (int32 i = 0; i < PowerUpActors.Num(); i++)
    {
        if (OtherActor->IsA(PowerUpActors[i]))
        {
            NumOverlappingActors--;
            // Prevent going below zero for some god-unknown reason
            if (NumOverlappingActors < 0)
            {
                NumOverlappingActors = 0;
            }
            PowerLevel = NumOverlappingActors * PowerGainedPerActor;
            TRACE("SwarmDecrease. Actor: %s, Overlapped: %s, PowerLevel: %f", *GetOwner()->GetName(), *OtherActor->GetName(), PowerLevel);
            OnPowerLevelChanged.Broadcast(NumOverlappingActors, PowerLevel);
        }
    }
}   

// broken

//#if WITH_EDITOR
//void USwarmComponent::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
//{
//    UE_LOG(LogTemp, Warning, TEXT("Changed"))
//    Super::PostEditChangeProperty(PropertyChangedEvent);
//
//    FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
//
//    if ((PropertyName == GET_MEMBER_NAME_CHECKED(USwarmComponent, ProximityRadius)))
//    {
//        UFloatProperty* property = static_cast<UFloatProperty*>(PropertyChangedEvent.Property);
//        ProximityRadius = property->GetPropertyValue(&ProximityRadius);
//        ProximitySphere->SetSphereRadius(ProximityRadius);
//    }
//}
//#endif

