// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/DamageNumbers/SFloatingText.h"
#include "Services/DamageNumbers/SDamageNumberService.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"

// Sets default values
ASFloatingText::ASFloatingText()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    bIsActive = false;
}

// Called when the game starts or when spawned
void ASFloatingText::BeginPlay()
{
	Super::BeginPlay();
    Setup(ASDamageNumberService::GetDamageNumberService(GetWorld()));
    Deactivate();
}

// Called every frame
void ASFloatingText::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASFloatingText::Activate(const AActor* DamagedActor, const AActor* DamageInstigator, float Amount, const TArray<FString>& DamageTags)
{
    bIsActive = true;
    SetActorTickEnabled(true);
    ReceiveActivation(DamagedActor, DamageInstigator, Amount, DamageTags);
}

void ASFloatingText::Deactivate()
{
    bIsActive = false;
    SetActorTickEnabled(false);
    ReceiveDeactivation();
    CachedDamageNumberService->ReturnToPool(this);
}

void ASFloatingText::Setup(ASDamageNumberService* DamageNumberService)
{
    CachedDamageNumberService = DamageNumberService;
}




