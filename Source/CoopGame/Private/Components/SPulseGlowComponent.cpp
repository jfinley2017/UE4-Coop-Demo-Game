#include "SPulseGlowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/Public/TimerManager.h"
#include "Materials/MaterialInstanceDynamic.h"

USPulseGlowComponent::USPulseGlowComponent()
{
}

void USPulseGlowComponent::BeginPulsing(float Intensity /*= 0.0f*/, FColor Color /*= FColor::White*/, float Frequency /*= 1.0f*/, float Duration /* = -1.0f */)
{
	AActor* MyOwner = GetOwner();

	if (!ensureAlways(MyOwner)) { return; }

	UMeshComponent* MeshComp = Cast<UMeshComponent>(MyOwner->GetComponentByClass(UMeshComponent::StaticClass()));

	if (!ensure(MeshComp)) { return; }

	MatInst = Cast<UMaterialInstanceDynamic>(MeshComp->GetMaterial(PulseMaterialIndex));

	if (!ensureMsgf(MatInst, TEXT("SPulseGlowComponent requires a mesh component with a material instance at GlowMaterialIndex")))
	{
		return;
	}

	SetParams(MatInst, Intensity, Color, Frequency);
	
	FTimerHandle Handle = FTimerHandle();

	if (Duration >= 0)
	{
		GetWorld()->GetTimerManager().SetTimer(Handle, this, &USPulseGlowComponent::StopPulsing, Duration, false);
	}

	FPulseData NewPulse = FPulseData(Handle, Intensity, Color, Frequency, Duration);

	ActivePulses.Add(NewPulse);
}

// @TODO: Rename GlowColor to PulseColor
void USPulseGlowComponent::SetParams(UMaterialInstanceDynamic* MatInstance, float Intensity /*= 0.0f*/, FColor Color /*= FColor::White*/, float Frequency /*= 1.0f*/)
{
	MatInstance->SetScalarParameterValue("PulseFreq", Frequency);
	MatInstance->SetScalarParameterValue("PulseIntensity", Intensity);
	MatInstance->SetVectorParameterValue("GlowColor", Color);
}

/** Pulses are added on in a stacked manner. Since they are not ordered by duration, RemoveAll is used to 
 * clear any effects which have ended. The top pulse in the stack should always be visible
 * A shorter pulse followed by a longer pulse will result in the shorter pulse being removed midway
 * without any noticeable effect. The longer pulse will then be "re-applied" without effect
 * A longer pulse followed by shorter will result in the long being re-applied after the short finishes
 **/
void USPulseGlowComponent::StopPulsing()
{
	ActivePulses.RemoveAll([&](FPulseData MyPulse) {
		if (!MyPulse.Timer.IsValid())
		{
			return false;
		}

		return GetWorld()->GetTimerManager().GetTimerRemaining(MyPulse.Timer) <= 0.0f;
	});

	if (ActivePulses.Num() == 0)
	{
		USPulseGlowComponent::SetParams(MatInst);
		return;
	}

	FPulseData PData = ActivePulses.Last();
	USPulseGlowComponent::SetParams(MatInst, PData.Intensity, PData.Color, PData.Frequency);
}

