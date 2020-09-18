#include "SWeapon.h"
#include "CoopGame.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BehaviorTreeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "SWeaponComponent.h"

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

    RefreshOwnerInfo(GetOwner());
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

    ReceiveWeaponActivated();
}

void ASWeapon::WeaponDeactivated()
{
    SetActorHiddenInGame(true);
    ReceiveWeaponDeactivated();
}

void ASWeapon::StartFire()
{
    FString CanFireErrorMessage;
    if (!CanFire(CanFireErrorMessage)) 
    { 
        return; 
    }

    // If this weapon has fired prior to us attempting to start fire
    float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_FireLoop, this, &ASWeapon::FireLoop, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::FireLoop()
{
    FString OutFireErrorMessage;
    if (!CanFire(OutFireErrorMessage))
    {
        StopFire();
        return;
    }

    Fire();
}

void ASWeapon::Fire()
{
    // we only want to fire on the server at the moment
    if (!HasAuthority())
    {
        return;
    }

    ReceiveFire();	

    ConsumeAmmo(AmmoConsumedPerFire);
	LastFireTime = UGameplayStatics::GetTimeSeconds(GetWorld());
}

void ASWeapon::StopFire()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_FireLoop);
}

void ASWeapon::AIFire_Implementation()
{
    Fire();
}

void ASWeapon::ConsumeAmmo(float AmmoToConsume)
{
    if (!HasAuthority())
    {
        return;
    }

    AmmoInClip -= AmmoToConsume;
}

bool ASWeapon::CanFire(FString& OutErrorMessage)
{
    // Reload the weapon if this check fails
    if (!HasAmmoRequiredToFire(true))
    {
        OutErrorMessage = "NOAMMO";
        return false;
    }

    float CurrentTime = UGameplayStatics::GetTimeSeconds(GetWorld());
    bool bIsOnCooldown = (LastFireTime + TimeBetweenShots > CurrentTime 
                          && !UKismetMathLibrary::NearlyEqual_FloatFloat(LastFireTime + TimeBetweenShots, CurrentTime, 0.1f));

    if (LastFireTime > 0.0f && bIsOnCooldown)
    {
        OutErrorMessage = "FIRECOOLDOWNACTIVE";
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

bool ASWeapon::IsReloading()
{
    UAnimInstance* OwningAnimInstance = OwnerInfo.OwningSkeletalMeshComponent.IsValid()
        ? OwnerInfo.OwningSkeletalMeshComponent->GetAnimInstance() : nullptr;

    if (!OwningAnimInstance)
    {
        // I guess?
        return false;
    }

    return OwningAnimInstance->Montage_IsPlaying(ReloadAnimation);
}

void ASWeapon::Reload()
{
    if (IsReloading() || AmmoInClip == ClipSize) 
    { 
        return; 
    }

    // Currently this is server only (USWC server designated function calls this)
    // Sound won't be played 
    /*if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadSound, GetActorLocation());
    }*/

    
    if (!ReloadAnimation || !OwnerInfo.OwningWeaponComponent.IsValid())
    {
        return;
    }
        
    OwnerInfo.OwningWeaponComponent->PlayMontage(ReloadAnimation);
    float AnimationTime = ReloadAnimation->GetPlayLength();
    GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &ASWeapon::FinishReload, AnimationTime, false);
}

void ASWeapon::FinishReload()
{
    ConsumeAmmo(-(ClipSize - AmmoInClip));
    bIsReloading = false;
}

void ASWeapon::RefreshOwnerInfo(AActor* InOwner)
{
    OwnerInfo.OwnerAsPawn = Cast<APawn>(InOwner);
    OwnerInfo.OwningSkeletalMeshComponent = InOwner ? InOwner->FindComponentByClass<USkeletalMeshComponent>() : nullptr; 
    OwnerInfo.OwningWeaponComponent = InOwner ? InOwner->FindComponentByClass<USWeaponComponent>() : nullptr;
}

void ASWeapon::CancelReload()
{
    bIsReloading = false;
    GetWorldTimerManager().ClearTimer(TimerHandle_Reload);
}
