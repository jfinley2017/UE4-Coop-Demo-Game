#include "SProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Public/TimerManager.h"
#include "SProjectileWeapon.h"
#include "Sound/SoundCue.h"
#include "DamageDealer.h"
#include "Net/UnrealNetwork.h"
#include "SWeapon.h"
#include "TimerManager.h"

ASProjectile::ASProjectile()
{

    SetReplicates(true);
    SetReplicateMovement(true);

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComp->SetNotifyRigidBodyCollision(true);
    MeshComp->SetVisibility(true);
    SetRootComponent(MeshComp);

    MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ExplosionStatus.bExploded = false;
	ExplosionStatus.bWasDirectPawnHit = false;

    
}

void ASProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASProjectile, ExplosionStatus);
    DOREPLIFETIME(ASProjectile, bIsServerProjectile);
}

void ASProjectile::Initialize(const FProjectileWeaponData &Data, bool bIsServer, TArray<AActor*> ActorsToIgnore)
{
    WeaponData = Data;
    bIsServerProjectile = bIsServer;
    bWasInitialized = true;
    MovementComp->Velocity = GetActorRotation().Vector() * ProjectileSpeed;
    MovementComp->MaxSpeed = ProjectileSpeed;
    MovementComp->InitialSpeed = ProjectileSpeed;
    MeshComp->MoveIgnoreActors.Append(ActorsToIgnore);
    MovementComp->UpdateComponentVelocity();
    
}

void ASProjectile::OnRep_bIsServerProjectile()
{
    if (bIsServerProjectile && !HasAuthority() && GetInstigator()->IsLocallyControlled())
    {
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
    }
}

void ASProjectile::Launch()
{
    if (!bWasInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Projectile fired despite not being Initialized. Please Initialze projectile. Undefined behavior incoming."))
    }

    OnActorHit.AddDynamic(this, &ASProjectile::OnProjectileHit);

    if (WeaponData.DoesExpire())
    {
        GetWorld()->GetTimerManager().SetTimer(FuseTimerHandle, this, &ASProjectile::OnProjectileExpire, WeaponData.ProjectileLifeTime, false);
    }
}

void ASProjectile::OnProjectileExpire()
{
    ExplosionStatus.bExploded = true;

	OnRep_Exploded();
	
}

void ASProjectile::OnProjectileHit(AActor * SelfActor, AActor * OtherActor, FVector NormalImpulse, const FHitResult & Hit)
{
	if (OtherActor == Instigator || (OtherActor && OtherActor->GetOwner() == Instigator))
	{
		return;
	}

	FProjectileExplosion NewStatus;
	DirectHitActor = OtherActor;
	
	if (Cast<APawn>(OtherActor))
	{
		NewStatus.bWasDirectPawnHit = true;
	}

	NewStatus.bExploded = true;
	ExplosionStatus = NewStatus;
    
	GetWorld()->GetTimerManager().ClearTimer(FuseTimerHandle);

	if (Role == ROLE_Authority)
	{
	    OnRep_Exploded();
	}
}

void ASProjectile::DirectHit()
{
	AController* InstContr = GetInstigatorController();

	// directly hitting an explosive barrel, for example, doesn't warrant a special sound
	if (ExplosionStatus.bWasDirectPawnHit && DirectHitSoundEffect)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), DirectHitSoundEffect);
	}

	if (Role == ROLE_Authority && DirectHitActor)
	{
		float DirectDamage = ApplyDamageModifier(WeaponData.ProjectileDamageDirectHit);

		AController* InstigatorController = Instigator ? Instigator->GetController() : nullptr;

		UGameplayStatics::ApplyDamage(DirectHitActor, DirectDamage,
			InstigatorController, GetOwner(), WeaponData.ProjectileDamageType);
	}
}

float ASProjectile::ApplyDamageModifier(float Damage)
{
	IDamageDealer* DamageDealer = Cast<IDamageDealer>(GetInstigator());
	
	return DamageDealer ? Damage += (DamageDealer->GetDamageModifier() / 100) * Damage : Damage;
}

void ASProjectile::Explode()
{
	GetWorld()->GetTimerManager().ClearTimer(FuseTimerHandle);

    if (ExplosionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation(), FVector::ZeroVector.Rotation());
    }

    if (ExplosionSoundEffect)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ExplosionSoundEffect, GetActorLocation());
    }

	DirectHit();

    TArray<AActor*> IgnoredActors = { this, GetOwner(), Instigator };
    if (Role == ROLE_Authority)
    {
        float ActualDamageRadial = ApplyDamageModifier(WeaponData.ProjectileDamage);
		
		AController* InstigatorController = Instigator ? Instigator->GetController() : nullptr;

		bool HitSomething = UGameplayStatics::ApplyRadialDamage(GetWorld(), ActualDamageRadial,
			GetActorLocation(), WeaponData.ProjectileRadius, WeaponData.ProjectileDamageType,
			IgnoredActors, GetOwner(), InstigatorController, true);

		ASWeapon* MyOwner = Cast<ASWeapon>(GetOwner());

		if (HitSomething && MyOwner)
		{
			MyOwner->OnHit(nullptr, true);
		}

		SetLifeSpan(0.5f);
	}

	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    SetActorHiddenInGame(true);
}

void ASProjectile::OnRep_Exploded()
{
    if (bIsServerProjectile && !HasAuthority())
    {
        return;
    }
    Explode();
}