#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SGlowOnHitComponent.generated.h"

class USHealthComponent;
class UDamageType;
class UMaterialInstanceDynamic;
/**
 * This component can be added to any actor which has a UMeshComponent, USHealthComponent, and
 * which has a material with the HitGlowColor, HitEmmissiveIntensity, and LastTimeDamageTaken params
 *
 * Throws error messages if components are missing, but does *not* check if the material params exist
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USGlowOnHitComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USGlowOnHitComponent();

protected:
	virtual void BeginPlay() override;

	// The index at which a material with the HitGlowColor, HitEmmissiveIntensity, and LastTimeDamageTaken params exist
	UPROPERTY(EditDefaultsOnly, Category = "GlowComp")
	int GlowMaterialIndex = 0;

	// Set to 0 for no glow
	UPROPERTY(EditDefaultsOnly, Category = "GlowComp")
	float GlowIntensity = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "GlowComp")
	FColor HitGlowColor;

	UPROPERTY(EditDefaultsOnly, Category = "GlowComp")
	FColor HealGlowColor;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent * ChangedHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "GlowComp")
	void FlashGlow(float TimeSeconds = 0.1f, FColor GlowColor = FColor::White, float Intensity = 0.0f);
};
