#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "SHealthComponent.h"
#include "UserWidget.h"
#include "Gameplay/GameplayComponents/TeamComponent.h"
#include "SHitIndicatorWidget.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ASWeapon::ASWeapon()
{
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComp;
    SetReplicates(true);
    NetUpdateFrequency = 66.0f;
    MinNetUpdateFrequency = 33.0f;
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASWeapon, bIsReloading);
    DOREPLIFETIME(ASWeapon, AmmoInClip);
}

void ASWeapon::BeginPlay()
{
    Super::BeginPlay();

    APawn* MyPawn = Cast<APawn>(GetOwner());

    AmmoInClip = ClipSize;

    // Potentially move to sweaponcomp so that each weapon can specify a different crosshair
    if (HitIndicatorWidgetClass && MyPawn && MyPawn->IsPlayerControlled())
    {
        HitIndicatorWidget = CreateWidget<USHitIndicatorWidget>(GetWorld(), HitIndicatorWidgetClass);
        HitIndicatorWidget->AddToViewport();
    }
}

void ASWeapon::StartFire()
{
    if (!CanFire()) { return; }

    // If this weapon has fired prior to us attempting to start fire
    float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, [&]()
	{
        if (!CanFire())
        {
            StopFire();
            return;
        }
		CancelReload();
		Fire();
        
	}, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::Fire()
{
    if (!HasAuthority())
    {
        ServerFire();
    }
    OnFire();
}

void ASWeapon::ServerFire_Implementation()
{
    Fire();
}

bool ASWeapon::ServerFire_Validate() { return true; }


bool ASWeapon::CanFire()
{
    if (HasAmmoRequiredToFire(true))
    {
        return true;
    }

    return false;
}

bool ASWeapon::HasAmmoRequiredToFire(bool bReloadIfFase)
{
    if (AmmoInClip > AmmoRequiredToFire)
    {
        return true;
    }
    Reload();
    return false;
}

void ASWeapon::StopFire()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::Reload()
{
    if (bIsReloading || AmmoInClip == ClipSize) { return; }

    if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadSound, GetActorLocation());
    }

    OnReload.ExecuteIfBound();

    ServerReload();
}

void ASWeapon::ServerCancelReload_Implementation()
{
    CancelReload();
}

bool ASWeapon::ServerCancelReload_Validate() { return true; }


void ASWeapon::CancelReload()
{
    bIsReloading = false;
    GetWorldTimerManager().ClearTimer(TimerHandle_ReloadTimer);

    if (Role < ROLE_Authority)
    {
        ServerCancelReload();
    }
}

void ASWeapon::ServerReload_Implementation()
{
    bIsReloading = true;

    OnReload.ExecuteIfBound();

    GetWorldTimerManager().SetTimer(TimerHandle_ReloadTimer, [&]() {
        AmmoInClip = ClipSize;
        bIsReloading = false;
    }, TimeToReload, false);
}

bool ASWeapon::ServerReload_Validate()
{
    return true;
}

/**
 * @param HitActor If this actor is on the other team the weapon hit will be broadcast
 * @param bSkipCheck If true, broadcast weapon hit regardless
 */
void ASWeapon::OnHit(AActor* HitActor, bool bSkipCheck)
{
    if (!HitIndicatorWidget || (!HitActor && !bSkipCheck))
    {
        return;
    }

    if (bSkipCheck)
    {
        HitIndicatorWidget->PlayHitAnimation();
        OnWeaponHit.Broadcast(HitActor);
        return;
    }

    // Perhaps team checks should be abstracted to something like gamemode, unsure
    USHealthComponent* HitHealth = Cast<USHealthComponent>(HitActor->GetComponentByClass(USHealthComponent::StaticClass()));
    if (HitHealth && GetOwner())
    {
        if (HitIndicatorWidget && !UTeamComponent::IsActorFriendly(HitActor, GetOwner()))
        {
            HitIndicatorWidget->PlayHitAnimation();
            OnWeaponHit.Broadcast(HitActor);
        }
    }
}