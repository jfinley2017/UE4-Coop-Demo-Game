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
}

void USWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

    SpawnDefaultWeaponInventory();
}

void USWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(USWeaponComponent, AmmoInventory);
    DOREPLIFETIME(USWeaponComponent, CurrentWeapon);
	DOREPLIFETIME(USWeaponComponent, WeaponInventory);
	DOREPLIFETIME(USWeaponComponent, bCanFire);
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

// Only want to SetCurrentWeapon on the server
void USWeaponComponent::EquipWeapon(ASWeapon* Weapon)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->OnReload.Unbind();
		CurrentWeapon->OnWeaponFire.Unbind();
		CurrentWeapon->SetActorHiddenInGame(true);
	}

	Weapon->OnWeaponFire.BindLambda([&]() {
		MulticastFireAnim();
		UE_LOG(LogTemp, Error, TEXT("bound fire"));
	});

	// Proxy our current weapon's reload delegate
	Weapon->OnReload.BindLambda([&]() {
		MulticastReloadAnim();
	});

    Weapon->WeaponActivated();

    if (!GetOwner()->HasAuthority())
    {
        ServerEquipWeapon(Weapon);
        return;
    }

    SetCurrentWeapon(Weapon);
}

void USWeaponComponent::ServerChangeWeaponAnim_Implementation()
{
    MulticastChangeWeaponAnim();
}

bool USWeaponComponent::ServerChangeWeaponAnim_Validate() { return true; }



void USWeaponComponent::ServerEquipWeapon_Implementation(ASWeapon* Weapon)
{
    EquipWeapon(Weapon);
}

bool USWeaponComponent::ServerEquipWeapon_Validate(ASWeapon* Weapon) { return true; }


void USWeaponComponent::ChangeWeapon()
{
    if (WeaponInventory.Num() == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Failed: WeaponInventoryEmpty"));
        return;
    }

    OnWeaponChange.Broadcast();
    ServerChangeWeaponAnim();

    bCanFire = false;
    ServerSetCanFire(false);

    FTimerHandle Handle;
    GetWorld()->GetTimerManager().SetTimer(Handle, [&]() {
        bCanFire = true;
        ServerSetCanFire(true);

        if (!CurrentWeapon)
        {
            UE_LOG(LogTemp, Log, TEXT("No Current Weapon, Equipping First"));
            EquipWeapon(WeaponInventory[0]);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("WeaponSwap Success"));
            GetCurrentWeapon()->CancelReload();
            StopFire();
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
    }, ChangeWeaponDelay, false);
}


void USWeaponComponent::ServerSetCanFire_Implementation(bool bCan)
{
	bCanFire = bCan;
}

bool USWeaponComponent::ServerSetCanFire_Validate(bool bCan) { return true; }

void USWeaponComponent::SetCurrentWeapon(ASWeapon* Weapon)
{
    if (Weapon)
    {
        Weapon->SetActorHiddenInGame(false);
        CurrentWeapon = Weapon;

        USkeletalMeshComponent* OwningMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        CurrentWeapon->AttachToComponent(OwningMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);

		if (Weapon->GetAmmoInClip() <= 0)
		{
			Weapon->Reload();
		}
    }
}

void USWeaponComponent::OnRep_CurrentWeapon()
{
    SetCurrentWeapon(CurrentWeapon);
}

void USWeaponComponent::OnRep_WeaponInventory()
{
	for (ASWeapon* W : WeaponInventory)
	{
		if (!W)
		{
			return;
		}
	}

	if (WeaponWidget)
	{
		WeaponWidget->RefreshWeapons();
	}
}


bool USWeaponComponent::IsLocallyControlled()
{
	ACharacter* MyCharacter = Cast<ACharacter>(GetOwner());
	return MyCharacter ? MyCharacter->IsLocallyControlled() : false;
}

void USWeaponComponent::MulticastFireAnim_Implementation()
{

	UE_LOG(LogTemp, Error, TEXT("onweaponfire"));
	OnWeaponFire.Broadcast();
}


bool USWeaponComponent::MulticastFireAnim_Validate() { return true; }

void USWeaponComponent::MulticastReloadAnim_Implementation()
{
	OnReload.Broadcast();
}

bool USWeaponComponent::MulticastReloadAnim_Validate()
{
	return true;
}

void USWeaponComponent::MulticastChangeWeaponAnim_Implementation()
{
	if (!IsLocallyControlled())
	{
		OnWeaponChange.Broadcast();
	}
}

bool USWeaponComponent::MulticastChangeWeaponAnim_Validate() { return true; }

void USWeaponComponent::StartFire()
{
    if (CurrentWeapon && bCanFire)
    {
        CurrentWeapon->StartFire();
    }
}

void USWeaponComponent::StopFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
    }
}

USWeaponWidget* USWeaponComponent::DrawWeaponWidget(APlayerController* OwningController, int32 NumberWeaponSlots)
{
    if (OwningController)
    {
        WeaponWidget = CreateWidget<USWeaponWidget>(OwningController,WeaponsWidgetClass);
    
		if (WeaponWidget)
		{
			WeaponWidget->InitializeWeaponWidget(this, NumberWeaponSlots);
			return WeaponWidget;
		}
    }

	UE_LOG(LogTemp, Error, TEXT("DrawWeaponWidget called without owning controller, this is confusing"));
	return nullptr;
}