// Fill out your copyright notice in the Description page of Project Settings.

#include "SProjectile_Bounce.h"
#include "Engine/World.h"
#include "Engine/Public/TimerManager.h"
#include "GameFramework/ProjectileMovementComponent.h"


ASProjectile_Bounce::ASProjectile_Bounce()
    : ASProjectile()
{
    MovementComp->bShouldBounce = true;
    MovementComp->Bounciness = 0.6;
}

void ASProjectile_Bounce::NotifyProjectileHit(AActor * SelfActor, AActor * OtherActor, FVector NormalImpulse, const FHitResult & Hit)
{
	if (ExplosionStatus.bExploded)
	{
		return;
	}

    if (Cast<APawn>(OtherActor))
    {
		DirectHitActor = OtherActor;
        ExplosionStatus.bExploded = true;
		ExplosionStatus.bWasDirectPawnHit = true;

		if (Role == ROLE_Authority)
		{
			OnRep_Exploded();
		}
    }
	else if (bSetsTimerOnBounce)
	{
		bSetsTimerOnBounce = false;

		if (WeaponData.ProjectileLifeTime && WeaponData.ProjectileDamage < TimeToLiveAfterBounce)
		{
			UE_LOG(LogTemp, Error, TEXT("TimeToLiveAfterBounce should be shorter than ProjectileLifeTime if bSetsTimerOnBounce is specified in SProjectile_Bounce"));
		}

		GetWorld()->GetTimerManager().ClearTimer(FuseTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(FuseTimerHandle, this, &ASProjectile::OnProjectileExpire, TimeToLiveAfterBounce, false);
	}
}