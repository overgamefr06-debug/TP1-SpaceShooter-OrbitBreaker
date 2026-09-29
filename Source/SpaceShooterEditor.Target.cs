using UnrealBuildTool;
public class SpaceShooterEditorTarget : TargetRules
{
    public SpaceShooterEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("SpaceShooter");
    }
}
