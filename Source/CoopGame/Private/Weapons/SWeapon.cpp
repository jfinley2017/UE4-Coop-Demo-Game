#include "SWeapon.h"
#include "CoopGame.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "SHealthComponent.h"
#include "TeamComponent.h"
#include "SHitIndicatorWidget.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BehaviorTreeComponent.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "ConstructorHelpers.h"

ASWeapon::ASWeapon()
{
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));

    RootComponent = MeshComp;

	MeshComp->SetCollisionResponseToChannel(COLLISION_HITSCAN_OVERLAP, ECR_Ignore);
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

    // Potentially move to sweaponcomp so that each weapon can specify a different crosshair
    if (HitIndicatorWidgetClass && MyPawn && MyPawn->IsPlayerControlled())
    {
        HitIndicatorWidget = CreateWidget<USHitIndicatorWidget>(GetWorld(), HitIndicatorWidgetClass);
        HitIndicatorWidget->AddToViewport();
    }
}

void ASWeapon::WeaponActivated()
{
    SetActorHiddenInGame(false);
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponActivatedSound, GetActorLocation());

	// Set the firing behavior in the AI Controller. This tree should be ran whenever one wishes for the AI to fire.
    // Allows for complex firing behavior
    // In the future, I'd really like to have firing behavior encapsulated in a task like setting which is different for each weapon
    // to remove dependency on the behavior tree
    // However as it is right now, BT and non-BT custom-made tasks don't play well together
    AAIController* AIController = Cast<AAIController>(GetInstigatorController());
    if (AIController)
    {
        UBehaviorTreeComponent* AITree = Cast<UBehaviorTreeComponent>(AIController->GetBrainComponent());
        if (AITree)
        {
            AITree->SetDynamicSubtree(FGameplayTag::RequestGameplayTag("SWeaponFiringLogic"), AIFiringBehavior);
        }
    }

    OnWeaponActivated();
}

void ASWeapon::WeaponDeactivated()
{
    SetActorHiddenInGame(true);

    OnWeaponDeactivated();
}

void ASWeapon::StartFire()
{
    FString CanFireErrorMessage;
    if (!CanFire(CanFireErrorMessage) || GetWorldTimerManager().IsTimerActive(TimerHandle_TimeBetweenShots)) { return; }

    // If this weapon has fired prior to us attempting to start fire
    float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	// Timer used with lambda to avoid creation of a function which is seemingly in the same scope as StartFire()
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, [&]()
	{
        FString CanFireErrorMessage;
        if (!CanFire(CanFireErrorMessage))
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
	LastFireTime = UGameplayStatics::GetTimeSeconds(GetWorld());

    if (HasAuthority())
    {
		OnWeaponFire.ExecuteIfBound();
        ConsumeAmmo(AmmoConsumedPerFire);
    }
}

void ASWeapon::AIFire_Implementation()
{
    Fire();
}

void ASWeapon::ConsumeAmmo_Implementation(float AmmoToConsume)
{
    if (HasAuthority())
    {
        AmmoInClip -= AmmoToConsume;
    }
}
bool ASWeapon::ConsumeAmmo_Validate(float AmmoToConsume) { return true; }

void ASWeapon::ServerFire_Implementation()
{
    Fire();
}
bool ASWeapon::ServerFire_Validate() { return true; }


bool ASWeapon::CanFire(FString& OutErrorMessage)
{
    // Reload the weapon if this check fails
    if (!HasAmmoRequiredToFire(true))
    {
        OutErrorMessage = "NO_AMMO";
        return false;
    }

    return true;
}

bool ASWeapon::HasAmmoRequiredToFire(bool bReloadIfFalse)
{
    if (AmmoInClip >= AmmoRequiredToFire)
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

void ASWeapon::ServerReload_Implementation()
{
    bIsReloading = true;

    OnReload.ExecuteIfBound();

    GetWorldTimerManager().SetTimer(TimerHandle_ReloadTimer, [&]() {
        ConsumeAmmo(-(ClipSize - AmmoInClip));
        bIsReloading = false;
    }, TimeToReload, false);
}
bool ASWeapon::ServerReload_Validate() { return true; }

void ASWeapon::CancelReload()
{
    bIsReloading = false;
    GetWorldTimerManager().ClearTimer(TimerHandle_ReloadTimer);

    if (!HasAuthority())
    {
        ServerCancelReload();
    }
}

void ASWeapon::ServerCancelReload_Implementation()
{
    CancelReload();
}
bool ASWeapon::ServerCancelReload_Validate() { return true; }

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

