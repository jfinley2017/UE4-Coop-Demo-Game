#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class UDamageType;
class UParticleSystem;
class USoundCue;
class UTexture2D;
class UImage;
class USHitIndicatorWidget;
class UAnimMontage;
class UBehaviorTree;
class USWeaponComponent;

USTRUCT()
struct FSWeaponOwnerInfo
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<APawn> OwnerAsPawn;

    UPROPERTY()
    TWeakObjectPtr<USkeletalMeshComponent> OwningSkeletalMeshComponent;

    UPROPERTY()
    TWeakObjectPtr<USWeaponComponent> OwningWeaponComponent;
};

/**
 * 
 */
UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	

	ASWeapon();

    // AActor
    virtual void BeginPlay() override;
    // ~AActor
    
    /**
     * Notifys the weapon that the owner wishes to begin firing.
     */
    UFUNCTION()
    virtual void StopFire();

    /**
     * Notifys the weapon that the owner wishes to stop firing.
     */
    UFUNCTION()
    virtual void StartFire();

    /**
     * Notifys the weapon that the owner wishes to reload.
     */
    UFUNCTION()
    virtual void Reload();
    
    /**
     * Notifys the weapon that the owner wishes to cancel the ongoing reload (if any).
     */
    UFUNCTION()
    virtual void CancelReload();

    /**
     * Determines whether or not the weapon can begin firing based on numerous weapon-specific conditions
     * @param OutErrorMessage - context as to why we couldn't fire, empty if we can currently fire.
     * @ret - true if we can currently fire
     */
    UFUNCTION()
    virtual bool CanFire(FString& OutErrorMessage);

    /**
     * Notifys the weapon that it has become active, eg the owner is actively wielding it.
     */
    UFUNCTION()
    virtual void WeaponActivated();

    /**
     * Notifys the weapon that it has become inactive, eg the owner is not actively wielding it.
     */
    UFUNCTION()
    virtual void WeaponDeactivated();

    UFUNCTION(BlueprintCallable)
    UTexture2D* GetWeaponIcon() { return WeaponIcon; }

    UFUNCTION(BlueprintCallable)
    float GetAmmoInClip() { return AmmoInClip; }

    UFUNCTION(BlueprintCallable)
    float GetClipSize() { return ClipSize; }

protected:

    /**
     * Loop body for firing the weapon. 
     */
    UFUNCTION()
    virtual void FireLoop();

    /**
     * Actually fires the weapon. 
     */
    UFUNCTION()
    virtual void Fire();
    
    /**
     * Consumes the amount of ammo specified by @AmmoToConsume. Negative values will instead give ammo.
     * @param AmmoToConsume - The ammo to take away, if negative it will instead give ammo.
     */
    UFUNCTION()
    void ConsumeAmmo(float AmmoToConsume);

    /** 
     * Checks to see if AmmoInClip <= AmmoRequiredToFire, returns false if so. Optionally reloads the weapon if there is not enough ammo. 
     */
    UFUNCTION()
    bool HasAmmoRequiredToFire(bool bReloadIfFase = false);

    /**
     * Returns whether or not this weapon is reloading.
     */
    UFUNCTION()
    bool IsReloading();

    /**
     * Hook invoked after a reload animation has finished.
     */
    UFUNCTION()
    virtual void FinishReload();

    /**
     * Refreshes the owner info associated with this weapon to be based off of @InOwner
     */
    UFUNCTION()
    void RefreshOwnerInfo(AActor* InOwner);

    /**
     * Information about our owner, helpful to prevent casting over and over for a 
     * reference to pawn/skeletalmesh/etc
     */
    UPROPERTY()
    FSWeaponOwnerInfo OwnerInfo;

    /** 
     * Blueprint-based hook to allow AI to know how to use this weapon 
     */
    UFUNCTION(BlueprintNativeEvent)
    void AIFire();

    //////////////////////////////////////////////////////////////////////////
    // Turn into data table?

    /** General Weapon Data */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponData")
    UBehaviorTree* AIFiringBehavior = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponData")
    UAnimMontage* ReloadAnimation = nullptr;

    /** If the weapon has less than this amount in its clip, HasAmmoRequiredToFire will fail */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponData")
    float AmmoRequiredToFire = 1;

    /** The damage type that this weapon causes */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponData")
    TSubclassOf<UDamageType> DamageType;

    /** The cooldown period between firing events */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponData")
    float TimeBetweenShots = 0.5f;

    /** Ammount of ammo that is consumed when fire is successfully called */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponData")
    float AmmoConsumedPerFire = 1;

    /** The maximum amount of ammo carried in a clip */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponData")
    float ClipSize = 20.0f;

    /** Sound played when weapon is swapped/otherwise becomes active */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponData")
    USoundCue* WeaponActivatedSound = nullptr;

    /** Sound played during reload */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponData")
	USoundCue* ReloadSound = nullptr;

    /** Specifies the socket on @MeshComp where things like muzzle flashes should play */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "WeaponData")
    FName MuzzleSocketName = "MuzzleSocket";

    /** Effect to be played on fire at @MuzzleSocketName's location */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponData")
    UParticleSystem* MuzzleEffect = nullptr;

    /** The sound a weapon makes when fired */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponData")
    USoundCue* FireSound = nullptr;

    /** The image used to represent this weapon */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponData")
    UTexture2D* WeaponIcon = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "WeaponData")
    TSubclassOf<USHitIndicatorWidget> HitIndicatorWidgetClass;   

    // ~ turn into data table
    //////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////
    /** Instance variables */

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USkeletalMeshComponent* MeshComp = nullptr;

     /** Widget reference displayed when this weapon hits something */
    USHitIndicatorWidget* HitIndicatorWidget = nullptr;

    /** 
     * The current amount of ammo in our clip, if this number is less than AmmoRequiredToFire, HasAmmoRequiredToFire will fail 
     */
    UPROPERTY(Replicated)
    float AmmoInClip = ClipSize;

    /** 
     * Timer used to run looping fire effects
     */
    FTimerHandle TimerHandle_FireLoop;

    /** 
     * The last time this weapon has fired. Clients will have a client-side version of this which
     * may be different than the servers.
     */
    UPROPERTY()
    float LastFireTime = -9999999;

    /** 
     * Timer driving a reload.
     */
	FTimerHandle TimerHandle_Reload;

    /** 
     * Blueprint hook for weapon firing.
     */
    UFUNCTION(BlueprintImplementableEvent)
    void ReceiveFire();

    /** 
     * Blueprint hook for WeaponActivated. 
     */
    UFUNCTION(BlueprintImplementableEvent)
    void ReceiveWeaponActivated();

    /** 
     * Blueprint hook for WeaponDeactivated. 
     */
    UFUNCTION(BlueprintImplementableEvent)
    void ReceiveWeaponDeactivated();

};
