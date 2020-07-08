// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SimpleCheatPlugin.h"

DEFINE_LOG_CATEGORY(LogSimpleCheatPlugin);

#define LOCTEXT_NAMESPACE "FSimpleCheatPluginModule"

void FSimpleCheatPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    UE_LOG(LogSimpleCheatPlugin, Log, TEXT("SimpleCheatPlugin Started."));
}

void FSimpleCheatPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
    UE_LOG(LogSimpleCheatPlugin, Log, TEXT("SimpleCheatPlugin Shutdown."));

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSimpleCheatPluginModule, SimpleCheatPlugin)