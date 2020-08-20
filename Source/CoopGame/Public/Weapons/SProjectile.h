// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
// TODO: Why wont it let me forward declare the FProjectileWeaponData struct?
#include "SProjectileWeapon.h"
#include "SProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class USoundCue;

USTRUCT()
struct FProjectileExplosion
{
	GENERATED_BODY()

public:

	UPROPERTY()
	bool bWasDirectPawnHit = false;

	UPROPERTY()
	bool bExploded = false;
};

UCLASS()
class COOPGAME_API ASProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASProjectile();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UProjectileMovementComponent* MovementComp = nullptr;

protected:
	AActor* DirectHitActor = nullptr;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_bIsServerProjectile, Category = "Weapon")
    bool bIsServerProjectile = false;

    UFUNCTION()
    void OnRep_bIsServerProjectile();

  
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* MeshComp = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ProjectileSpeed;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    UParticleSystem* ExplosionEffect = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    USoundCue* ExplosionSoundEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundCue* DirectHitSoundEffect = nullptr;

	// HitActor will take direct damage if specified
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Explode();

    UFUNCTION()
    virtual void NotifyProjectileHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	/** Deal damage and play effects for a direct hit */
	virtual void DirectHit();

	/** Checks if instigator implements damage dealer and uses the modifier if available */
	float ApplyDamageModifier(float Damage);

	UPROPERTY(ReplicatedUsing = OnRep_Exploded)
	FProjectileExplosion ExplosionStatus;

    UFUNCTION()
    void OnRep_Exploded();

    FTimerHandle FuseTimerHandle;
    FProjectileWeaponData WeaponData;

    bool bWasInitialized = false;

public:	
    UFUNCTION()
    virtual void OnProjectileExpire();
	
	UFUNCTION()
    virtual void Launch();
	
    virtual void Initialize(const FProjectileWeaponData& Data, bool bIsServer, TArray<AActor*> ActorsToIgnore);
};
