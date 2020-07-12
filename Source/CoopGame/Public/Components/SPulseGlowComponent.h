#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SPulseGlowComponent.generated.h"

class UMaterialInstanceDynamic;

USTRUCT()
struct FPulseData {
	GENERATED_BODY()
public:
	FPulseData()
	: Timer(FTimerHandle()), Intensity(0.0f), Color(FColor::White), Frequency(0), Duration(0)
	{}

	FPulseData(FTimerHandle InTimer, float InIntensity /* lol */, FColor InColor, float InFrequency /* lol 2 */, float InDuration)
	: Timer(InTimer), Intensity(InIntensity), Color(InColor), Frequency(InFrequency), Duration(InDuration)
	{}

	FTimerHandle Timer;
	float Intensity;
	FColor Color;
	float Frequency;
	float Duration;
};

UCLASS(Blueprintable, ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USPulseGlowComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USPulseGlowComponent();
	// Negative duration indicates that pulse goes on until stopped
	UFUNCTION(BlueprintCallable)
	void BeginPulsing(float Intensity = 1.0f, FColor Color = FColor::White, float Frequency = 1.0f, float Duration  = -1.0f);

	UFUNCTION(BlueprintCallable)
	void StopPulsing();

protected:
	// Pulses will override each other, but at least a quick pulse will revert back to the longer pulse after
	TArray<FPulseData> ActivePulses;

	UPROPERTY(EditDefaultsOnly, Category = "PulseComp")
	int PulseMaterialIndex = 0;

	UMaterialInstanceDynamic* MatInst = nullptr;

	static void SetParams(UMaterialInstanceDynamic* MatInstance, float Intensity = 0.0f, FColor Color = FColor::White, float Frequency = 1.0f);
};
