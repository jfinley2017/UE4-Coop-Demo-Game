// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class CoopGame : ModuleRules
{
	public CoopGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "UMG",
            "AIModule",
            "NavigationSystem",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "GameplayTags",
            "SimpleLoggingToolsPlugin",
            "SimpleCheatPlugin"
        });

        PublicIncludePathModuleNames.AddRange(new string[] { "SimpleLoggingToolsPlugin" });

        PrivateDependencyModuleNames.AddRange(new string[] {  });
        PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });


    }
}
