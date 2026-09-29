using UnrealBuildTool;
public class SpaceShooter : ModuleRules
{
    public SpaceShooter(ReadOnlyTargetRules Target) : base(Target)
    {
        if (Target.bBuildEditor) PrivateDependencyModuleNames.Add("UnrealEd");
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
    }
}
