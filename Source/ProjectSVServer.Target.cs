// Shin YeongSeop 2026

using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectSVServerTarget : TargetRules
{
	public ProjectSVServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("ProjectSV");

		// Dedicated Server optimizations
		bUseLoggingInShipping = true;

		// Disable unnecessary features for dedicated server
		bCompileAgainstEditor = false;
		bBuildWithEditorOnlyData = false;

		// Enable dedicated server optimizations
		bWithServerCode = true;

		// Disable features not needed for dedicated server
		bCompileRecast = true; // Keep for AI navigation
		bCompileNavmeshSegmentLinks = true;
		bCompileNavmeshClusterLinks = true;

		// Network settings for dedicated server
		bUseChecksInShipping = false;

		// Global definitions for dedicated server
		GlobalDefinitions.Add("PROJECTSV_DEDICATED_SERVER=1");
	}
}