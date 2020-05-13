#include "SFunctionLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Public/Config.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"

bool USFunctionLibrary::FindFloor(UWorld* World, FVector FindFrom, FVector& OutFoundLocation)
{
	FHitResult HitResult;
	FVector EndTrace = FindFrom - FVector(0.0f, 0.0f, 9999.0f);
	bool bFoundGround = World->LineTraceSingleByChannel(HitResult, FindFrom, EndTrace, ECC_WorldStatic);

	OutFoundLocation = HitResult.ImpactPoint; 

	return bFoundGround;
}

UConfig* USFunctionLibrary::GetConfigSingleton()
{
	return UConfig::GetConfigSingleton();
}
