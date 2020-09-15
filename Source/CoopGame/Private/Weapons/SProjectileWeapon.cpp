#include "SProjectileWeapon.h"
#include "SProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"   
#include "CoopGame.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"


void ASProjectileWeapon::SpawnProjectile(TSubclassOf<ASProjectile> Projectile, bool bAttemptPrediction, TArray<AActor*> IgnoredActors)
{
    if (!HasAuthority()) { return; }

    if (Projectile && GetOwner())
    {
        // Where our owner is currently looking
        FVector ActorEyesLocation;
        FRotator ActorEyesRotation;
        GetOwner()->GetActorEyesViewPoint(ActorEyesLocation, ActorEyesRotation);

        // A trace with an arbitrary range in order to find a location to send the projectile towards
        float TraceMaxRange = 20000.0f;
        FVector TraceEndPoint = (ActorEyesRotation.Vector() * TraceMaxRange) + ActorEyesLocation;
        FHitResult OwnerAimLocationHitResult;
        GetWorld()->LineTraceSingleByChannel(OwnerAimLocationHitResult, ActorEyesLocation, TraceEndPoint, COLLISION_WEAPON);

        // If we hit something, use that as the aim location. If not: use the max trace range we specified
        // Handles cases like firing into the air.
        FVector HitLocation = OwnerAimLocationHitResult.bBlockingHit ? OwnerAimLocationHitResult.ImpactPoint : TraceEndPoint;

        // Where the projectile will be spawned, where muzzle flash will be played
        FVector ProjectileSpawnLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
        FRotator ProjectileSpawnRotation = UKismetMathLibrary::FindLookAtRotation(ProjectileSpawnLocation, TraceEndPoint);

        DrawDebugLine(GetWorld(), ProjectileSpawnLocation, (ProjectileSpawnRotation.Vector() * 20000.0f) + ProjectileSpawnLocation,FColor::Red,true,1.5f);

        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleEffect, ProjectileSpawnLocation);
        UGameplayStatics::SpawnSoundAtLocation(GetWorld(), FireSound, GetActorLocation());

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.Instigator = Cast<APawn>(GetOwner());
        SpawnParams.Owner = this;
        ASProjectile* NewProjectile = GetWorld()->SpawnActor<ASProjectile>(ProjectileClass, ProjectileSpawnLocation, ProjectileSpawnRotation, SpawnParams);

        // This is a hack to hide server-spawned projectiles in a predicted environment
        // Without this, the client who instigated the projectile will see two projectiles rather than one.
        bool bHideFromClient = false;
         //bAttemptPrediction&& HasAuthority() && GetInstigator()->IsPlayerControlled();


        NewProjectile->Initialize(ProjectileWeaponConfigData, bHideFromClient, IgnoredActors);
        NewProjectile->Launch();


    }
}
