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

}

USWeaponWidget* USWeaponComponent::DrawWeaponWidget(APlayerController* OwningController, int32 NumberWeaponSlots)
{
    if (OwningController)
    {
        WeaponWidget = CreateWidget<USWeaponWidget>(OwningController, WeaponsWidgetClass);

        if (WeaponWidget)
        {
            WeaponWidget->InitializeWeaponWidget(this, NumberWeaponSlots);
            return WeaponWidget;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("DrawWeaponWidget called without owning controller, this is confusing"));
    return nullptr;
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
            ServerEquipWeapon(WeaponInventory[0]);
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
    ServerReload();
    return true;
}

bool USWeaponComponent::TryChangeWeapon(FString& OutErrorMessage)
{
    if (!CanChangeWeapon(OutErrorMessage))
    {
        return false;
    }

    int32 CurrentWeaponIndex = WeaponInventory.Find(CurrentWeapon);
    if (CurrentWeaponIndex == WeaponInventory.Num() - 1)
    {
        ServerEquipWeapon(WeaponInventory[0]);
    }
    else if (CurrentWeaponIndex != INDEX_NONE)
    {
        CurrentWeaponIndex++;
        ServerEquipWeapon(WeaponInventory[CurrentWeaponIndex]);
    }

    OutErrorMessage = "";
    return true;
}

bool USWeaponComponent::CanFire(FString& OutErrorMessage)
{
    ASWeapon* CachedCurrentWeapon = GetCurrentWeapon();
    if (CachedCurrentWeapon)
    {
        return GetCurrentWeapon()->CanFire(OutErrorMessage);
    }

    OutErrorMessage = "NoCurrentWeapon";
    return false;
}

bool USWeaponComponent::CanChangeWeapon(FString& OutErrorReason)
{
    if (WeaponInventory.Num() == 0)
    {
        OutErrorReason = "InventoryEmpty";
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

void USWeaponComponent::ServerEquipWeapon_Implementation(ASWeapon* Weapon)
{
    if (Weapon == nullptr)
    {
        return;
    }

    FString StopFireErrorMessage;
    TryStopFire(StopFireErrorMessage);

    ASWeapon* CachedCurrentWeapon = GetCurrentWeapon();
    CurrentWeapon = Weapon;
    OnRep_CurrentWeapon(CachedCurrentWeapon);

}

bool USWeaponComponent::ServerEquipWeapon_Validate(ASWeapon* Weapon)
{
    return true;
}

void USWeaponComponent::SetupWeapon(ASWeapon* Weapon)
{

    USkeletalMeshComponent* OwningMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    CurrentWeapon->AttachToComponent(OwningMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);

    if (Weapon)
    {
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
    for (ASWeapon* Weapon : WeaponInventory)
    {
        if (!Weapon)
        {
            return;
        }
    }

    OnWeaponInventoryUpdated.Broadcast();

    if (WeaponWidget)
    {
        WeaponWidget->RefreshWeapons();
    }
}
