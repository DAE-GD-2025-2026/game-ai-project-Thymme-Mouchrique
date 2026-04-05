using UnrealBuildTool;
using System.IO;

public class GameAIProg : ModuleRules
{
    public GameAIProg(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        bUseRTTI = true;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "AIModule",
            "NavigationSystem",
            "StateTreeModule",
            "GameplayStateTreeModule",
            "Niagara",
            "UMG",
            "Slate",
            "ImGui",
            "Navmesh",
        });

        PublicIncludePaths.AddRange(new string[]
        {
            ModuleDirectory,
            Path.Combine(ModuleDirectory, "Shared"),
            Path.Combine(ModuleDirectory, "GraphTheory"),
            Path.Combine(ModuleDirectory, "Movement"),
        });
    }
}