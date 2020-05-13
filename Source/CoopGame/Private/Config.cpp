#include "Public/Config.h"
#include "Engine/Engine.h"

void UConfig::SetMouseSensitivity(float Sens)
{
	MouseSensitivity = Sens;
	SaveConfig();
}

float UConfig::GetMouseSensitivity()
{
	return MouseSensitivity;
}

UConfig* UConfig::GetConfigSingleton()
{

	return GEngine ? Cast<UConfig>(GEngine->GameSingleton) : nullptr;
}
