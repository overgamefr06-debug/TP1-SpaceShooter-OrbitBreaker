using UnrealBuildTool;
public class SpaceShooterTarget : TargetRules
{
    public SpaceShooterTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("SpaceShooter");
    }
}
