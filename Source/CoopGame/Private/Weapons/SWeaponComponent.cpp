#include "SWeaponComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "CoopGame.h"
#include "SWeaponWidget.h"
#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

USWeaponComponent::USWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    CurrentWeapon = nullptr;
}

void USWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

    SpawnDefaultWeaponInventory();
}

void USWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(USWeaponComponent, CurrentWeapon);
	DOREPLIFETIME(USWeaponComponent, WeaponInventory);
    DOREPLIFETIME(USWeaponComponent, ReplicatedAnimationInfo);

}

void USWeaponComponent::SpawnDefaultWeaponInventory()
{
    // We want the server to own these actors
    if (!GetOwner()->HasAuthority())
    {
        return;
    }
    else
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.Owner = GetOwner();
        SpawnParams.Instigator = Cast<APawn>(GetOwner());

        for (int i = 0; i < DefaultWeapons.Num(); i++)
        {
            ASWeapon* NewWeapon = GetWorld()->SpawnActor<ASWeapon>(DefaultWeapons[i], SpawnParams);
            WeaponInventory.Add(NewWeapon);
            NewWeapon->SetActorHiddenInGame(true);
        }

        if (WeaponInventory.Num() > 0)
        {
            EquipWeapon(WeaponInventory[0]);
        }

        if (!IsRunningDedicatedServer())
        {
            OnRep_WeaponInventory();
        }
    }
}

bool USWeaponComponent::TryStartFire(FString& CanFireErrorMessage)
{
    if (!CanFire(CanFireErrorMessage))
    {
        return false;
    }

    ServerFire();
    CanFireErrorMessage = "";
    return true;
}

bool USWeaponComponent::TryStopFire(FString& CanStopFireErrorMessage)
{
    ServerStopFire();
    CanStopFireErrorMessage = "";
    return true;
}

bool USWeaponComponent::TryReload(FString& ErrorMessage)
{
    FString OutStopFireErrorMessage;
    TryStopFire(OutStopFireErrorMessage);

    ServerReload();
    return true;
}

bool USWeaponComponent::TryChangeWeapon(FString& OutErrorMessage)
{
    if (!CanChangeWeapon(OutErrorMessage))
    {
        return false;
    }

    ServerChangeWeapon();
    OutErrorMessage = "";
    return true;
}

void USWeaponComponent::CancelReload()
{
    ServerCancelReload();
}

bool USWeaponComponent::CanFire(FString& OutErrorMessage)
{
    ASWeapon* CachedCurrentWeapon = GetCurrentWeapon();
    if (!CachedCurrentWeapon)
    {
        OutErrorMessage = "NOCURRENTWEAPON";
        return false;
    }

    if (IsChangingWeapons())
    {
        OutErrorMessage = "CHANGINGWEAPON";
        return false;
    }

    bool bCanWeaponFire = CachedCurrentWeapon->CanFire(OutErrorMessage);
    if (bCanWeaponFire)
    {
        return true;
    }

    if (OutErrorMessage == "NOAMMO")
    {
        FString OutReloadErrorMessage;
        TryReload(OutReloadErrorMessage);
        return false;
    }
    return false;
}

bool USWeaponComponent::CanChangeWeapon(FString& OutErrorReason)
{
    if (WeaponInventory.Num() == 0)
    {
        OutErrorReason = "INVENTORYEMPTY";
        return false;
    }

    return true;
}

void USWeaponComponent::ServerFire_Implementation()
{
    FString CanFireErrorMessage;
    if (!CanFire(CanFireErrorMessage))
    {
        return;
    }

    ASWeapon* CachedCurrentWeapon = GetCurrentWeapon();
    if (CachedCurrentWeapon)
    {
        CachedCurrentWeapon->StartFire();
    }

    return;
}

bool USWeaponComponent::ServerFire_Validate()
{
    return true;
}

void USWeaponComponent::ServerStopFire_Implementation()
{
    ASWeapon* CachedCurrentWeapon = GetCurrentWeapon();
    if (CachedCurrentWeapon)
    {
        CachedCurrentWeapon->StopFire();
    }
}

bool USWeaponComponent::ServerStopFire_Validate()
{
    return true;
}

void USWeaponComponent::ServerReload_Implementation()
{
    ASWeapon* CachedCurrentWeapon = GetCurrentWeapon();
    if (CachedCurrentWeapon)
    {
        CachedCurrentWeapon->Reload();
    }
}

bool USWeaponComponent::ServerReload_Validate()
{
    return true;
}

void USWeaponComponent::ServerCancelReload_Implementation()
{
    ASWeapon* CachedCurrentWeapon = GetCurrentWeapon();
    if (CachedCurrentWeapon)
    {
        CachedCurrentWeapon->CancelReload();
    }
}

bool USWeaponComponent::ServerCancelReload_Validate()
{
    return true;
}

void USWeaponComponent::EquipWeapon(ASWeapon* Weapon)
{
    if (!GetOwner()->HasAuthority())
    {
        return;
    }

    if (Weapon == nullptr)
    {
        return;
    }

    ASWeapon* CachedCurrentWeapon = GetCurrentWeapon();
    CurrentWeapon = Weapon;
    OnRep_CurrentWeapon(CachedCurrentWeapon);
}

void USWeaponComponent::ServerChangeWeapon_Implementation()
{
    FString StopFireErrorMessage;
    TryStopFire(StopFireErrorMessage);
    CancelReload();
    
    float ChangeWeaponDuration = 0.1f;
    if (WeaponSwapMontage)
    {
        ChangeWeaponDuration = WeaponSwapMontage->GetPlayLength()-.1f;
        PlayMontage(WeaponSwapMontage);
    }

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwapTimer, this, &USWeaponComponent::ChangeWeapon, ChangeWeaponDuration, false);
}

bool USWeaponComponent::ServerChangeWeapon_Validate()
{
    return true;
}

void USWeaponComponent::ChangeWeapon()
{
    int32 CurrentWeaponIndex = WeaponInventory.Find(CurrentWeapon);
    if (CurrentWeaponIndex == WeaponInventory.Num() - 1)
    {
        EquipWeapon(WeaponInventory[0]);
    }
    else if (CurrentWeaponIndex != INDEX_NONE)
    {
        CurrentWeaponIndex++;
        EquipWeapon(WeaponInventory[CurrentWeaponIndex]);
    }
}

void USWeaponComponent::SetupWeapon(ASWeapon* Weapon)
{
    if (Weapon)
    {
        USkeletalMeshComponent* OwningMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        CurrentWeapon->AttachToComponent(OwningMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
        Weapon->WeaponActivated();
    }
}

void USWeaponComponent::UnsetupWeapon(ASWeapon* Weapon)
{
    if (Weapon)
    {
        Weapon->WeaponDeactivated();
    }
}

bool USWeaponComponent::IsChangingWeapons()
{
    USkeletalMeshComponent* OwnerSkelMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    UAnimInstance* OwnerAnimInstance = OwnerSkelMeshComponent ? OwnerSkelMeshComponent->GetAnimInstance() : nullptr;

    if (OwnerAnimInstance)
    {
        return OwnerAnimInstance->Montage_IsPlaying(WeaponSwapMontage);
    }
    return false;
}

void USWeaponComponent::PlayMontage(UAnimMontage* MontageToPlay)
{
    if (!GetOwner()->HasAuthority())
    {
        ServerPlayMontage(MontageToPlay);
        return;
    }

    ReplicatedAnimationInfo.bStopPlaying = false;
    ReplicatedAnimationInfo.Montage = MontageToPlay;
    ReplicatedAnimationInfo.ForceReplication();
    OnRep_ReplicatedAnimationInfo();
}

void USWeaponComponent::ServerPlayMontage_Implementation(UAnimMontage* Animation)
{
    PlayMontage(Animation);
}

bool USWeaponComponent::ServerPlayMontage_Validate(UAnimMontage* Animation)
{
    return true;
}

ASWeapon* USWeaponComponent::GetCurrentWeapon()
{
    return CurrentWeapon;
}

TArray<ASWeapon*> USWeaponComponent::GetWeaponInventory()
{
    return WeaponInventory;
}

void USWeaponComponent::OnRep_CurrentWeapon(ASWeapon* LastEquippedWeapon)
{
    UnsetupWeapon(LastEquippedWeapon);
    SetupWeapon(CurrentWeapon);
    OnWeaponChange.Broadcast();
}

void USWeaponComponent::OnRep_WeaponInventory()
{
    OnWeaponInventoryUpdated.Broadcast();
}

void USWeaponComponent::OnRep_ReplicatedAnimationInfo()
{
    USkeletalMeshComponent* OwnerSkelMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

    UAnimInstance* OwnerAnimInstance = OwnerSkelMeshComponent ? OwnerSkelMeshComponent->GetAnimInstance() : nullptr;
    if (!OwnerAnimInstance)
    {
        return;
    }

    if (ReplicatedAnimationInfo.bStopPlaying)
    {
        if (OwnerAnimInstance->Montage_IsPlaying(ReplicatedAnimationInfo.Montage))
        {
            OwnerAnimInstance->Montage_Stop(0.0f, ReplicatedAnimationInfo.Montage);
        }
        return;
    }

    OwnerAnimInstance->Montage_Play(ReplicatedAnimationInfo.Montage);

}
