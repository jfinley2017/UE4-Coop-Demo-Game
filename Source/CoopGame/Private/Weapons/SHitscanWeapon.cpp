#include "SHitscanWeapon.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystem.h"
#include "CoopGame.h"
#include "SPlayerController.h"
#include "Engine/Public/TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "DamageDealer.h"
#include "Library/SDamageLibrary.h"


ASHitscanWeapon::ASHitscanWeapon()
    : ASWeapon()
{
    SetActorTickEnabled(true);
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.bCanEverTick = true;
}


void ASHitscanWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    APawn* OwningPawn = Cast<APawn>(GetOwner());
    if (OwningPawn)
    {
        APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());

        if (PC)
        {
            // Get the amount that the mouse has moved this frame, scale it by the input scale
            float MouseInputX;
            float MouseInputY;
            PC->GetInputMouseDelta(MouseInputX, MouseInputY);
            MouseInputX *= PC->InputYawScale;
            MouseInputY *= PC->InputPitchScale;

            if (GetWorldTimerManager().IsTimerActive(TH_RecoilReset))
            {
                // Interp to the recoil position
                float PitchOffsetThisFrame = FMath::FInterpTo(0, DesiredPitchOffset, DeltaTime, 20.0);
                float YawOffsetThisFrame = FMath::FInterpTo(0, DesiredYawOffset, DeltaTime, 20.0);

                // Move the camera
                PC->AddPitchInput(PitchOffsetThisFrame);
                PC->AddYawInput(YawOffsetThisFrame);

                // Since the interp does not happen instantly, there may be cases where we never actually reach the full desired offset
                // thus we must keep track of how far we did get
                CurrentPitchOffset += PitchOffsetThisFrame;
                CurrentYawOffset += YawOffsetThisFrame;

                // Subtract the amount which the mouse moved this frame from the offset, but do not keep track of movement over or under the offset value
                // (ex. If the recoil is tracking top right, and the player moved his mouse top right, do not track that as our current offset, rather keep the offset as it was)
                // This prevents large camera swings after firing is over
                CurrentPitchOffset = CurrentPitchOffset < 0.0f ? FMath::Clamp(CurrentPitchOffset + MouseInputY, CurrentPitchOffset, 0.0f) : FMath::Clamp(CurrentPitchOffset +  MouseInputY, 0.0f, CurrentPitchOffset);
                CurrentYawOffset = CurrentYawOffset < 0.0f ? FMath::Clamp(CurrentYawOffset + MouseInputX, CurrentYawOffset, 0.0f) : FMath::Clamp(CurrentYawOffset + MouseInputX, 0.0f, CurrentYawOffset);
            }
            else if (!FMath::IsNearlyZero(CurrentYawOffset, 0.01f) || !FMath::IsNearlyZero(CurrentPitchOffset, 0.01f))
            {
                // If the player moves his mouse, abort resetting the recoil
                if (MouseInputX != 0.0f || MouseInputY != 0.0f)
                {
                    CurrentPitchOffset = 0.0f;
                    CurrentYawOffset = 0.0f;
                    return;
                }

                // Same as above interps, but in reverse in order to bring the camera back to the shot origin
                float PitchOffsetThisFrame = FMath::FInterpTo(0, -CurrentPitchOffset, DeltaTime, 25.0);
                float YawOffsetThisFrame = FMath::FInterpTo(0, -CurrentYawOffset, DeltaTime, 25.0);

                // Move the camera
                PC->AddPitchInput(PitchOffsetThisFrame);
                PC->AddYawInput(YawOffsetThisFrame);

                // Add (since we negated the CurrentPitch/YawOffsets in our interps) the amount we interped to the CurrentPitch/Yaw values
                CurrentPitchOffset += PitchOffsetThisFrame;
                CurrentYawOffset += YawOffsetThisFrame;
            }
        }
    }
}


void ASHitscanWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASHitscanWeapon, AmmoInClip);
    DOREPLIFETIME_CONDITION(ASHitscanWeapon, HitScanTrace, COND_SkipOwner);
}


void ASHitscanWeapon::PerformHitScan(bool bDoDamage, FHitResult &OutHitResult)
{
    AActor* Owner = GetOwner();
    if (Owner)
    {
        FVector EyeLocation;
        FRotator EyeRotation;
        Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Owner);
        QueryParams.AddIgnoredActor(this);
        QueryParams.bTraceComplex = true;
        QueryParams.bReturnPhysicalMaterial = true;

        // Bullet Spread
        float HalfRad = FMath::DegreesToRadians(BulletSpread);
        FVector ShotDirection = FMath::VRandCone(EyeRotation.Vector(), HalfRad, HalfRad);
        FVector TraceEnd = EyeLocation + (ShotDirection * 1000000);

        FHitResult OutHit;
        if (GetWorld()->LineTraceSingleByChannel(OutHit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
        {
            OnTraceHit(OutHit, ShotDirection, bDoDamage);
            HitScanTrace.TraceTo = OutHit.ImpactPoint;
            HitScanTrace.SurfaceType = UPhysicalMaterial::DetermineSurfaceType(OutHit.PhysMaterial.Get());
        }
        else
        {
            HitScanTrace.TraceTo = TraceEnd;
            HitScanTrace.SurfaceType = SurfaceType_Default;
        }

        // Force the struct to replicate
        HitScanTrace.HitIndex = (HitScanTrace.HitIndex == 255) ? 0 : HitScanTrace.HitIndex + 1;

        // Need to call this in order to play the effects ourselves
        OnRep_HitScanTrace();

        OutHitResult = OutHit;
    }

}

void ASHitscanWeapon::OnTraceHit(FHitResult Hit, FVector ShotDirection, bool bDoDamage)
{
    AActor* HitActor = Hit.GetActor();
    AActor* Owner = GetOwner();
    if (!HitActor || !Owner) { return; }

    // Get surface type to use to calculate damage multipler/impact effects
    EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
    OnHit(HitActor);

    // Calculate and apply damage for this shot
    if (bDoDamage)
    {
        FSDamageInstance NewDamageInstance;
        NewDamageInstance.Instigator = GetOwner();
        NewDamageInstance.Receiver = HitActor;
        NewDamageInstance.Timestamp = GetWorld()->GetTimeSeconds();
        NewDamageInstance.DamageDealer = this;

        // Better way of doing this in the future, maybe ensure that owners are damage dealers
        IDamageDealer* DamageDealer = Cast<IDamageDealer>(GetOwner());
        // Calculate damage multiplier
        float ActualDamage = BaseDamage;

        if (DamageDealer)
        {
            ActualDamage += (DamageDealer->GetDamageModifier() / 100) * ActualDamage;
        }

        if (SurfaceType == SURFACE_FLESHVULNERABLE)
        {
            ActualDamage *= HeadshotBonus;
            NewDamageInstance.ContextTags.Add("CriticalHit");
            NewDamageInstance.ContextTags.Add("Headshot");
        }

        NewDamageInstance.Damage = ActualDamage;
        USDamageLibrary::DealDamage(NewDamageInstance);
    }
}

// To display effects on clients
void ASHitscanWeapon::OnRep_HitScanTrace()
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation());
    DrawTracerEffect(HitScanTrace.TraceTo);
	PlayImpactEffect(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASHitscanWeapon::DrawTracerEffect(const FVector &TraceEndPoint)
{
    FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
    if (MuzzleEffect)
    {
		FVector ShotDirection = TraceEndPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleEffect, MuzzleLocation, ShotDirection.Rotation());
    }

    if (TracerEffect)
    {
        UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
        if (TracerComp)
        {
            TracerComp->SetVectorParameter(TracerTargetName, TraceEndPoint);
        }
    }
}

void ASHitscanWeapon::PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
    UParticleSystem* SelectedEffect = nullptr;
    switch (SurfaceType)
    {
        case SURFACE_FLESHDEFAULT:
        case SURFACE_FLESHVULNERABLE:
            SelectedEffect = FleshImpactEffect;
            break;
        default:
            SelectedEffect = DefaultImpactEffect;
            break;
    }

    if (SelectedEffect)
    {
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
        FVector ShotDirection = ImpactPoint - MuzzleLocation;
        ShotDirection.Normalize();
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
    }
}

void ASHitscanWeapon::ApplyRecoil()
{
    APawn* OwningPawn = Cast<APawn>(GetOwner());
    if (OwningPawn)
    {
        // Need to cast to our playercontroller to add some BS
        ASPlayerController* PC = Cast<ASPlayerController>(OwningPawn->GetController());
        if (PC)
        {
            if (GetWorldTimerManager().IsTimerActive(TH_RecoilReset))
            {
                CurrentRecoilPitchConsqecModifier += RecoilScalarPitchConsecutiveShotModifier;
                CurrentRecoilYawConsqecModifier += RecoilScalarYawConsecutiveShotModifier;
            }
            else
            {
                CurrentRecoilPitchConsqecModifier = RecoilScalarPitch;
                CurrentRecoilYawConsqecModifier = RecoilScalarYaw;
            }


            DesiredYawOffset = CurrentRecoilYawConsqecModifier;
            DesiredPitchOffset = CurrentRecoilPitchConsqecModifier;

          
            GetWorldTimerManager().SetTimer(TH_RecoilReset, this, &ASHitscanWeapon::RecoilExpired, RecoilResetDelay, false);
        }
    }
}

void ASHitscanWeapon::RecoilExpired()
{
    CurrentRecoilPitchConsqecModifier = 0.0f;
    CurrentRecoilYawConsqecModifier = 0.0f;
    DesiredPitchOffset = 0.0f;
    DesiredYawOffset = 0.0f;
}

