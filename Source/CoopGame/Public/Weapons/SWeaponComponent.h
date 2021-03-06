#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponChange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponInventoryUpdatedSignature);

class USWeaponWidget;
class ASWeapon;
class UImage;
class UTexture2D;

//////////////////////////////////////////////////////////////////////////
// Support
// 1.) Firing, effects, anims. TryStartFire -> [Server] ServerStartFire -> Weapon->Fire() -> FireLoop
//                                             [Local] SimulateStartFire -> Weapon->SimulateStartFire() -> PlayFireEffects
//
// 2.) Reloading, effects, anims. TryStartReload -> [Server] ServerStartReload -> Animation playing, OnRep set to trigger on remotes, ignore on owner -> AnimNotify -> Weapon->Reload
//                                               -> [Local] SimulateStartReload -> 
//
// 3.) Changing Weapons, effects, anims
//////////////////////////////////////////////////////////////////////////

/**
 * Struct to drive replication of weapon animations, runs gameplay logic from (as an example)
 * an OnRep function associated with this variable.
 */
USTRUCT()
struct FSReplicatedAnimMontageInfo
{
    GENERATED_BODY()

    /**
     * Montage to play, or stop playing if @bStopPlaying is true
     */
    UPROPERTY()
    UAnimMontage* Montage = nullptr;

    /**
     * Whether or not to start or stop the montage. Should be used to either stop playing @Montage or start playing @Montage
     */
    UPROPERTY()
    bool bStopPlaying = false;

    /**
     * Forces a replication update on this struct, useful for replaying an animation.
     */
    UPROPERTY()
    uint8 ForceReplicationByte = 1;

    void ForceReplication()
    {
        ForceReplicationByte = (ForceReplicationByte + 1) % 255;
    }

};

/**
 * 
 */
UCLASS(ClassGroup=(COOP), meta=(BlueprintSpawnableComponent))
class COOPGAME_API USWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
    /**
     * Broadcasted when a weapon is successfully changed.
     */
	UPROPERTY(BlueprintAssignable, Category = "WeaponComponent")
	FOnWeaponChange  OnWeaponChange;

    /**
     * Broadcasted when the weapon inventory is updated for any reason.
     */
    UPROPERTY(BlueprintAssignable, Category = "WeaponComponent")
    FOnWeaponInventoryUpdatedSignature OnWeaponInventoryUpdated;

    USWeaponComponent();

    /**
     * Attempts to fire the currently equipped weapon. Can fail for a variety of reasons. 
     * In multiplayer games where the executor is not the authority, this may result in a false positive.
     * @param ErrorMessage - context as to why we couldn't begin firing, empty if successfully fired. 
     * @ret - true if the weapon was successfully fired.
     */
    UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
    virtual bool TryStartFire(FString& ErrorMessage);

    /**
     * Attempts to stop fire of the currently equipped weapon. Can fail for a variety of reasons.
     * In multiplayer games where the executor is not the authority, this may result in a false positive.
     * @param ErrorMessage - context as to why we couldnt stop firing, empty if successfully fired. 
     * @ret - true if we successfully stopped firing.
     */
    UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
    virtual bool TryStopFire(FString& ErrorMessage);

    /**
     * Attempts to reload the currently equipped weapon. Can fail for a variety of reasons.
     * In multiplayer games where the executor is not the authority, this may result in a false positive.
     * @param ErrorMessage - context as to why we couldn't reload, empty if successfully reloading.
     * @ret - true if we successfully reloaded.
     */
    UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
    virtual bool TryReload(FString& ErrorMessage);

    /**
     * Attempts to cancel reloading the currently equipped weapon. Can silently fail, safe to call even 
     * if the current weapon is not reloading.
     */
    UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
    virtual void CancelReload();

    /**
     * Attempts to change the currently equipped weapon. Can fail for a variety of reasons.
     * In multiplayer games where the executor is not the authority, this may result in a false positive.
     * @param ErrorMessage - context as to why we couldn't change weapons, empty if successfully changing weapons.
     * @ret - true if we successfully began changing weapons.
     */
    UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
    virtual bool TryChangeWeapon(FString& OutErrorMessage);

    /**
     * Plays a montage, handling replication. 
     */
    UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
    virtual void PlayMontage(UAnimMontage* MontageToPlay);

    /**
     * Retrieves the current weapon.
     */
    UFUNCTION(BlueprintPure, Category = "WeaponComponent")
    ASWeapon* GetCurrentWeapon();

    /**
     * Retreives the weapon inventory.
     */
    UFUNCTION(BlueprintPure, Category = "WeaponComponent")
    TArray<ASWeapon*> GetWeaponInventory();

protected:

	// AActor
	virtual void BeginPlay() override;
    // ~AActor

    /**
     * Spawns the default weapons for the owner, adding them to inventory.
     */
    void SpawnDefaultWeaponInventory();

    /**
     * Returns true if can fire, returns false if cannot.
     * @param OutErrorReason - The reason why we couldn't fire, empty if returning trues
     */
    UFUNCTION()
    virtual bool CanFire(FString& OutErrorReason);

    /**
     * Returns true if can change weapon, returns false if cannot.
     * @param OutErrorReason - The reason why we couldn't change weapon, empty if returning trues
     */
    UFUNCTION()
    virtual bool CanChangeWeapon(FString& OutErrorReason);

    /**
     * Sets up @Weapon to be the currently equipped weapon
     */
    UFUNCTION()
    virtual void SetupWeapon(ASWeapon* Weapon);

    /**
     * Removes @Weapon, making it no longer equipped
     */
    UFUNCTION()
    virtual void UnsetupWeapon(ASWeapon* Weapon);

    /**
     * Returns whether or not we are in the middle of changing weapons
     */
    UFUNCTION()
    bool IsChangingWeapons();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerFire();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerStopFire();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerReload();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerCancelReload();

    UFUNCTION()
    void EquipWeapon(ASWeapon* Weapon);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerChangeWeapon();

    UFUNCTION()
    void ChangeWeapon();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerPlayMontage(UAnimMontage* Animation);
    
    UFUNCTION()
    void HandleWeaponSwapAnimNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
     
    /**
     * Where to equip weapons on our owner's skeleton
     */
    UPROPERTY(VisibleDefaultsOnly, Category = "Player")
    FName WeaponSocket = "WeaponSocket";

    /**
     * Montage to use when swapping weapons, potentially to be moved into ASWeapon
     */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponComponent|Data")
    UAnimMontage* WeaponSwapMontage = nullptr;

    /**
     * Default weapons to be spawned for the owner on BeginPlay.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponComponent|Data")
    TArray<TSubclassOf<ASWeapon>> DefaultWeapons;

    /**
     * Replication of animations for weapons, see definition of FSReplicatedAnimMontageInfo
     */
    UPROPERTY(ReplicatedUsing = OnRep_ReplicatedAnimationInfo)
    FSReplicatedAnimMontageInfo ReplicatedAnimationInfo;

    /**
    * Currently equipped/activated weapons.
    */
    UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
    ASWeapon* CurrentWeapon;
    
    /**
     * Currently owned weapons.
     */
    UPROPERTY(ReplicatedUsing = OnRep_WeaponInventory)
    TArray<ASWeapon*> WeaponInventory;
    
    /**
     * Runs weapon swapping
     */
    FTimerHandle TimerHandle_WeaponSwapTimer;

    UFUNCTION()
    void OnRep_CurrentWeapon(ASWeapon* LastEquippedWeapon);

    UFUNCTION()
    void OnRep_WeaponInventory();

    UFUNCTION()
    void OnRep_ReplicatedAnimationInfo();

};
