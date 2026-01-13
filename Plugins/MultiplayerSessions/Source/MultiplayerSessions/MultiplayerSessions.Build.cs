// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MultiplayerSessions : ModuleRules
{
	public MultiplayerSessions(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);

		// Core dependencies for all targets
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"OnlineSubsystem",
				"OnlineSubsystemSteam",
				"OnlineSubsystemUtils"
			}
			);

		// UI dependencies only for non-server builds
		if (Target.Type != TargetType.Server)
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"UMG",
					"Slate",
					"SlateCore"
				}
				);
		}


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Sockets",
				"Networking"
			}
			);

		// Slate dependencies only for non-server builds
		if (Target.Type != TargetType.Server)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Slate",
					"SlateCore"
				}
				);
		}

		// Define macro for UI code availability
		if (Target.Type == TargetType.Server)
		{
			PublicDefinitions.Add("MULTIPLAYER_WITH_UI=0");
		}
		else
		{
			PublicDefinitions.Add("MULTIPLAYER_WITH_UI=1");
		}

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}