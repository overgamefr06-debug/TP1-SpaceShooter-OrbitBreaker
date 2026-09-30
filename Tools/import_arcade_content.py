"""Import arcade art and configure real Blueprint subclasses (UE 5.8 Python).
Check out existing BP_Ship, BP_Asteroid, BP_OrbitHUD, BP_SpaceGameMode and L_Arena.
Run generate_arcade_audio.py first. Generated PNG provenance: ArtSources/Arcade.
"""
from pathlib import Path
import unreal

lib = unreal.EditorAssetLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()
edit = unreal.MaterialEditingLibrary
root = '/Game/Art/Arcade'
source = Path(__file__).resolve().parents[1] / 'ArtSources' / 'Arcade'

def imported(name, ext):
    path = root + '/' + name
    if not lib.does_asset_exist(path):
        task = unreal.AssetImportTask()
        task.filename = str(source / (name + ext))
        task.destination_path = root; task.destination_name = name
        task.automated = True; task.save = True
        tools.import_asset_tasks([task])
    obj = lib.load_asset(path)
    assert obj, path
    return obj

def bp(name, parent):
    path = '/Game/Blueprints/' + name
    if lib.does_asset_exist(path): asset = lib.load_asset(path)
    else:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property('parent_class', unreal.load_class(None, '/Script/SpaceShooter.' + parent))
        asset = tools.create_asset(name, '/Game/Blueprints', unreal.Blueprint, factory)
    cls = lib.load_blueprint_class(path)
    return asset, cls, unreal.get_default_object(cls)

def save(asset):
    unreal.BlueprintEditorLibrary.compile_blueprint(asset)
    assert lib.save_loaded_asset(asset)

mesh = lib.load_asset('/Game/Art/Fleet/SM_FleetSprite')
sound = imported('S_RockCollision', '.wav')
chime = imported('S_Pickup', '.wav')

for name in ['RockA','RockB']: imported('T_'+name,'.png')
b,c,o = bp('BP_RockCollision','CombatBurst'); o.set_editor_property('sound',sound); save(b)
for name in ['DoubleScore','Shield','Repair','TripleShot']:
    b,c,o = bp('BP_Bonus'+name,'SpacePickup')
    o.set_editor_property('collect_sound',chime); save(b)
# Preserve the nebula artwork but lower its brightness through a material.
path = root + '/M_QuietBackdrop'
if lib.does_asset_exist(path): backdrop = lib.load_asset(path)
else:
    backdrop = tools.create_asset('M_QuietBackdrop',root,unreal.Material,unreal.MaterialFactoryNew())
    backdrop.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    n = edit.create_material_expression(backdrop,unreal.MaterialExpressionTextureSample,0,0)
    n.set_editor_property('texture',lib.load_asset('/Game/Art/T_OrbitNebula'))
    mul = edit.create_material_expression(backdrop,unreal.MaterialExpressionMultiply,200,0)
    mul.set_editor_property('const_b',.25)
    edit.connect_material_expressions(n,'RGB',mul,'A')
    edit.connect_material_property(mul,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    edit.recompile_material(backdrop); lib.save_loaded_asset(backdrop)
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert level.load_level('/Game/Maps/L_Arena')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for a in actors.get_all_level_actors():
    label = a.get_actor_label()
    if label=='Space background': a.static_mesh_component.set_material(0,backdrop)
assert level.save_current_level()
unreal.log('ARCADE_CONTENT_IMPORTED_OK')
