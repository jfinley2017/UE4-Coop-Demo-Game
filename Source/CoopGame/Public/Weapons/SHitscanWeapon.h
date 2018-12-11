#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SHitscanWeapon.generated.h"

// Information for a sngle hitscan lnne trace
USTRUCT()
struct FHitScanTrace
{
    GENERATED_BODY()

public:

    /** The surface type encountered at the TraceTo location */
    UPROPERTY()
    TEnumAsByte<EPhysicalSurface> SurfaceType;

    /** The endpoint of our trace, either maxrange or whatever we hit before then */
    UPROPERTY()
    FVector_NetQuantize TraceTo;

    /** Variable used to force replication of this struct, as firing in the same spot would not cause 
        replication to occur otherwise */
    UPROPERTY()
    uint8 HitIndex = 0;
};


USTRUCT()
struct FRecoilPoint
{
    GENERATED_BODY()

    UPROPERTY()
    float DeltaPitch;

    UPROPERTY()    
    float DeltaYaw;

};

UCLASS()
class COOPGAME_API ASHitscanWeapon : public ASWeapon
{
	GENERATED_BODY()
	

protected:
    
    UPROPERTY(EditDefaultsOnly, Category = "WeaponFiringData")
    float HeadshotBonus = 4.0f;

    UPROPERTY(EditDefaultsOnly, Category = "WeaponFiringData")
    float BaseDamage = 20.0f;

    UPROPERTY(EditDefaultsOnly, Category = "WeaponFiringData", meta = (ClampMin=0.0f))
    float BulletSpread = 0.0f;

    /** How much we should orient the camera's pitch during an ApplyRecoil application */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponFiringData")
    float RecoilScalarPitch = 1.0f;

    /** How much we should orient the camera's yaw during an ApplyRecoil application */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponFiringData")
    float RecoilScalarYaw = 1.0f;

    /** How much pitch recoil is added after each consecutive shot */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponFiringData")
    float RecoilScalarPitchConsecutiveShotModifier = 0.0f;
    float CurrentRecoilPitchConsqecModifier = 0.0f;

    /** How much yaw recoil is added after each consecutive shot  */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponFiringData")
    float RecoilScalarYawConsecutiveShotModifier = 0.0f;
    float CurrentRecoilYawConsqecModifier = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "WeaponFiringData")
    float RecoilResetDelay = 0.0f;

    float RecoilResetTime = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponEffectData")
    UParticleSystem* DefaultImpactEffect = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponEffectData")
    UParticleSystem* FleshImpactEffect = nullptr;

    /** Is replicated whenever a shot is fired */
    UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
    FHitScanTrace HitScanTrace;

    /** The weapon's tracer effect, if any */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponEffectData")
    UParticleSystem* TracerEffect = nullptr;

    /** idk some weird tracer shit */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "WeaponEffectData")
    FName TracerTargetName = "Target";

    /** Normally used to display effects (muzzle, tracer, impact, etc) */
    UFUNCTION()
    void OnRep_HitScanTrace();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void ApplyRecoil();

    /** Fires off a hitscan shot, optionally doing damage and draining ammo*/
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void PerformHitScan(bool bDoDamage, bool bConsumeAmmo);

    /** Functionality performed after a hit is recorded */
    void OnTraceHit(FHitResult Hit, FVector ShotDirection, bool bDoDamage);

    /** Draws the tracer effects for this weapon, and also spawns the muzzle effect*/
    void DrawTracerEffect(const FVector &TraceEndPoint);

    /** Draws the impact effects for this weapon */
    void PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint);

};
