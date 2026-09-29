using UnrealBuildTool;
public class SpaceShooter : ModuleRules
{
    public SpaceShooter(ReadOnlyTargetRules Target) : base(Target)
    {
        if (Target.bBuildEditor) PrivateDependencyModuleNames.AddRange(new[] { "UnrealEd", "AudioMixer" });
        PrivateDependencyModuleNames.Add("SlateCore");
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
    }
}
