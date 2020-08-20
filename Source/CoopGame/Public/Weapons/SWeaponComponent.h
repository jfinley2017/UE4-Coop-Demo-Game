#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponChange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponCompFire);

class USWeaponWidget;
class ASWeapon;
class UImage;
class UTexture2D;

UCLASS(ClassGroup=(COOP), meta=(BlueprintSpawnableComponent))
class COOPGAME_API USWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USWeaponComponent();

	UPROPERTY(BlueprintAssignable, Category = "PlayerWeapon")
	FOnReload OnReload;

	UPROPERTY(BlueprintAssignable, Category = "PlayerWeapon")
	FOnWeaponCompFire OnWeaponFire;

	UPROPERTY(BlueprintAssignable, Category = "PlayerWeapon")
	FOnWeaponChange  OnWeaponChange;
	
	// WeaponComp manages this, because there are things outside the weapon's control
	// which cause it to not be allowed to fire - like switching weapons
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Weapon")
	bool bCanFire = true;

	USWeaponWidget* WeaponWidget = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    USWeaponWidget* DrawWeaponWidget(APlayerController* OwningController, int32 NumberWeaponSlots);

    UFUNCTION(BlueprintPure, Category = "WeaponData")
    ASWeapon* GetCurrentWeapon() { return CurrentWeapon; }

    UFUNCTION(BlueprintPure, Category = "WeaponData")
    TArray<ASWeapon*> GetWeaponInventory() { return WeaponInventory; }

    UFUNCTION(BlueprintCallable, Category = "PlayerWeapon")
    void StartFire();

    UFUNCTION(BlueprintCallable, Category = "PlayerWeapon")
    void StopFire();

    void ChangeWeapon();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    UPROPERTY(VisibleDefaultsOnly, Category = "Player")
    FName WeaponSocket = "WeaponSocket";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ExposeOnSpawn = true))
    TArray<TSubclassOf<ASWeapon>> DefaultWeapons;

    UPROPERTY(ReplicatedUsing = OnRep_WeaponInventory)
    TArray<ASWeapon*> WeaponInventory;

	// Delay before the weapon is swapped, this should match the animation
	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Weapon")
	float ChangeWeaponDelay = 0.17f;

    UPROPERTY(BlueprintReadonly, ReplicatedUsing = OnRep_CurrentWeapon)
    ASWeapon* CurrentWeapon;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<USWeaponWidget> WeaponsWidgetClass = nullptr;

    /** [Server] **/
    void SpawnDefaultWeaponInventory();

    /** [Server] **/
    void EquipWeapon(ASWeapon* Weapon);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetCanFire(bool bCan);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerEquipWeapon(ASWeapon* Weapon);

    UFUNCTION()
    void SetCurrentWeapon(ASWeapon* NewWeapon);

    UFUNCTION()
    void OnRep_CurrentWeapon();

    UFUNCTION()
    void OnRep_WeaponInventory();

private:
	bool IsLocallyControlled();

	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerChangeWeaponAnim();

	UFUNCTION(NetMulticast, Unreliable, WithValidation)
	void MulticastChangeWeaponAnim();

	UFUNCTION(NetMulticast, Unreliable, WithValidation)
	void MulticastFireAnim();

	UFUNCTION(NetMulticast, Unreliable, WithValidation)
	void MulticastReloadAnim();
};
