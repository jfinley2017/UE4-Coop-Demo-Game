// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/DamageNumbers/SDamageNumberService.h"
#include "Services/DamageNumbers/SFloatingText.h"
#include "Engine/World.h"
#include "EngineUtils.h"

// Sets default values
ASDamageNumberService::ASDamageNumberService()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    InitialPooledTextActors = 50;
    DamageNumberClass = ASFloatingText::StaticClass();
}

// Called when the game starts or when spawned
void ASDamageNumberService::BeginPlay()
{
	Super::BeginPlay();

    if (!ShouldRunDamageService())
    {
        Destroy(this);
        return;
    }

    for (int32 i = 0; i < InitialPooledTextActors; i++)
    {
        ASFloatingText* NewFloatingTextActor = GetWorld()->SpawnActor<ASFloatingText>(DamageNumberClass.Get(),FVector::ZeroVector, FRotator::ZeroRotator);
        FloatingTextPool.Add(NewFloatingTextActor);
    }
}

void ASDamageNumberService::ReturnToPool(ASFloatingText* FloatingTextObject)
{
    FloatingTextPool.Add(FloatingTextObject);
}

FVector ASDamageNumberService::FindDamageNumberActivationLocation_Implementation(const AActor* DamagedActor, const AActor* DamageInstigator, float DamageAmount, const TArray<FString>& DamageTags)
{
    return DamagedActor->GetActorLocation();
}

ASFloatingText* ASDamageNumberService::RetrieveFromPool()
{
    return FloatingTextPool.Pop();
}

bool ASDamageNumberService::ShouldRunDamageService()
{
    if (GetNetMode() == NM_DedicatedServer)
    {
        return false;
    }
    return true;
}

// Called every frame
void ASDamageNumberService::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASDamageNumberService::DisplayDamageNumber(const AActor* DamagedActor, const AActor* DamageInstigator, float DamageAmount, const TArray<FString> DamageTags)
{
    ASFloatingText* FloatingTextActor = RetrieveFromPool();
    FloatingTextActor->SetActorLocation(FindDamageNumberActivationLocation(DamagedActor, DamageInstigator, DamageAmount, DamageTags));
    FloatingTextActor->SetActorRotation(DamagedActor->GetActorRotation());
    FloatingTextActor->Activate(DamagedActor, DamageInstigator, DamageAmount, DamageTags);
}

ASDamageNumberService* ASDamageNumberService::GetDamageNumberService(UObject* WorldContextObject)
{
    // We need a WorldContextObject because this function is static
    UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    if (!World) { return nullptr; }

    TActorIterator<ASDamageNumberService> It(World);
    if (It)
    {
        return *It;
    }
    return nullptr;
}

