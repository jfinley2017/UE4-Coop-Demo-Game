#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

DECLARE_DELEGATE(FOnWeaponReload);
DECLARE_DELEGATE(FOnWeaponFire);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponHit, AActor*, HitActor);

class UDamageType;
class UParticleSystem;
class USoundCue;
class UTexture2D;
class UImage;
class USHitIndicatorWidget;
class UAnimMontage;
class UBehaviorTree;

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ASWeapon();

    virtual void BeginPlay() override;

    ///////////////////////////////////
    /** Event Delegates */
	FOnWeaponReload OnReload;
	FOnWeaponFire OnWeaponFire;

    UPROPERTY(BlueprintAssignable, Category = "Weapon")
    FOnWeaponHit OnWeaponHit;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    UTexture2D* GetWeaponIcon() { return WeaponIcon; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetAmmoInClip() { return AmmoInClip; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetClipSize() { return ClipSize; }

    ///////////////////////////////////
    /** Current Weapon State Variables */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
    bool bIsReloading;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponFiringData")
    float TimeToReload = 1.5f;

    /** Should be overridden if a particular weapon requires any special cases to be true in order for it to begin firing */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    bool CanFire();

    /** Checks to see if AmmoInClip <= AmmoRequiredToFire, returns false if so. Optionally reloads the weapon if there is not enough ammo */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    bool HasAmmoRequiredToFire(bool bReloadIfFase = false);

    ///////////////////////////////////
    /** Weapon Actions */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Reload();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void CancelReload();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void StopFire();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void StartFire();

    /** Normally called when the weapon has become active (through changing weapon, etc)*/
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void WeaponActivated();

    /** Normally called when the weapon */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void WeaponDeactivated();

    // primary used to dispay hit markers, should be bound to a delegate somewhere perhaps if it needs to be accessed outside of
    // this class, rather than being public
    virtual void OnHit(AActor* HitActor, bool bSkipCheck = false);

protected:

    ///////////////////////////////////
    /** Weapon Actions */

	/*UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UAnimMontage* HipFireAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UAnimMontage* ADSFireAnimation;*/

    /** Handles replication of firing, calls OnFire. Rather than overriding this, one should override OnFire */
    virtual void Fire();
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerFire();

    /** Blueprint hook for Fire */
    UFUNCTION(BlueprintImplementableEvent, Category = "Firing")
    void OnFire();
	
    /** Blueprint-based hook to allow AI to know how to use this weapon */
    UFUNCTION(BlueprintNativeEvent, Category = "Firing")
    void AIFire();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerReload();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerCancelReload();

    /** Ammo mutator, use negative values to give ammo */
    UFUNCTION(Server, Reliable, WithValidation)
    void ConsumeAmmo(float AmmoToConsume);

    /** Blueprint hook for WeaponActivated */
    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
    void OnWeaponActivated();

    /** Blueprint hook for WeaponDeactivated */
    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
    void OnWeaponDeactivated();
	
    ///////////////////////////////////
    /** General Weapon Data */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponFiringData")
    UBehaviorTree* AIFiringBehavior = nullptr;

    ///////////////////////////////////
    /** General Weapon Data */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* MeshComp = nullptr;

    /** If the weapon has less than this amount in its clip, HasAmmoRequiredToFire will fail */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponFiringData")
    float AmmoRequiredToFire = 1;

    /** The damage type that this weapon causes */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponFiringData")
    TSubclassOf<UDamageType> DamageType;

    /** The cooldown period between firing events */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponFiringData")
    float TimeBetweenShots = 0.5f;

    /** Ammount of ammo that is consumed when fire is successfully called */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponFiringData")
    float AmmoConsumedPerFire = 1;

    /** The maximum amount of ammo carried in a clip */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponFiringData")
    float ClipSize = 20.0f;

    /** Sound played when weapon is swapped/otherwise becomes active */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponEffectData")
    USoundCue* WeaponActivatedSound = nullptr;

    /** Sound played during reload */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponEffectData")
	USoundCue* ReloadSound = nullptr;

    /** Specifies the socket on @MeshComp where things like muzzle flashes should play */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "WeaponEffectData")
    FName MuzzleSocketName = "MuzzleSocket";

    /** Effect to be played on fire at @MuzzleSocketName's location */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponEffectData")
    UParticleSystem* MuzzleEffect = nullptr;

    /** The sound a weapon makes when fired */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponEffectData")
    USoundCue* FireSound = nullptr;

    /** The image used to represent this weapon */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponEffectData")
    UTexture2D* WeaponIcon = nullptr;

    /** Widget reference displayed when this weapon hits something */
    USHitIndicatorWidget* HitIndicatorWidget = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "WeaponEffectData")
    TSubclassOf<USHitIndicatorWidget> HitIndicatorWidgetClass;   

    ///////////////////////////////////
    /** Instance variables */
    /** The current amount of ammo in our clip, if this number is less than AmmoRequiredToFire, HasAmmoRequiredToFire will fail */
    UPROPERTY(Replicated)
    float AmmoInClip = ClipSize;

    /** Timer used repeat firing events (eg when mouse is held down) */
    FTimerHandle TimerHandle_TimeBetweenShots;

    /** The last time this weapon has fired */
    UPROPERTY(BlueprintReadWrite, Category = "WeaponInstanceVariabes")
    float LastFireTime = -9999999;

    /** The timer tracking how much time is remaining for a successful reload operation */
	FTimerHandle TimerHandle_ReloadTimer;


};
