"""Run once with UnrealEditor-Cmd -run=pythonscript -script=<this file>."""
import unreal
import random

assets = unreal.AssetToolsHelpers.get_asset_tools()
library = unreal.EditorAssetLibrary

def material(name, color):
    path = '/Game/Materials/' + name
    if library.does_asset_exist(path):
        return library.load_asset(path)
    result = assets.create_asset(name, '/Game/Materials', unreal.Material, unreal.MaterialFactoryNew())
    result.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    constant = unreal.MaterialEditingLibrary.create_material_expression(result, unreal.MaterialExpressionConstant3Vector, -250, 0)
    constant.set_editor_property('constant', unreal.LinearColor(*color, 1))
    unreal.MaterialEditingLibrary.connect_material_property(constant, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(result)
    library.save_loaded_asset(result)
    return result

def blueprint(name, parent):
    path = '/Game/Blueprints/' + name
    if library.does_asset_exist(path):
        return library.load_asset(path)
    factory = unreal.BlueprintFactory()
    factory.set_editor_property('parent_class', unreal.load_class(None, parent))
    return assets.create_asset(name, '/Game/Blueprints', unreal.Blueprint, factory)

ship_mat = material('M_Ship', (0.12, 0.85, 1.0))
wing_mat = material('M_Wings', (0.04, 0.32, 0.55))
floor_mat = material('M_Space', (0.003, 0.006, 0.016))
star_mat = material('M_Stars', (0.35, 0.45, 0.65))
border_mat = material('M_Border', (0.02, 0.13, 0.20))
ship = blueprint('BP_Ship', '/Script/SpaceShooter.ShipPawn')
ship_class = library.load_blueprint_class('/Game/Blueprints/BP_Ship')
ship_defaults = unreal.get_default_object(ship_class)
ship_defaults.set_editor_property('move_speed', 650.0)
ship_defaults.set_editor_property('arena_half_size', unreal.Vector2D(500, 900))
ship_defaults.get_editor_property('hull').set_material(0, ship_mat)
ship_defaults.get_editor_property('wings').set_material(0, wing_mat)
unreal.BlueprintEditorLibrary.compile_blueprint(ship)
library.save_loaded_asset(ship)

mode = blueprint('BP_SpaceGameMode', '/Script/SpaceShooter.SpaceGameMode')
mode_defaults = unreal.get_default_object(library.load_blueprint_class('/Game/Blueprints/BP_SpaceGameMode'))
mode_defaults.set_editor_property('default_pawn_class', ship_class)
unreal.BlueprintEditorLibrary.compile_blueprint(mode)
library.save_loaded_asset(mode)

map_path = '/Game/Maps/L_Arena'
if not library.does_asset_exist(map_path):
    level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level.new_level(map_path):
        raise RuntimeError('Cannot create arena')
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    cube = library.load_asset('/Engine/BasicShapes/Cube')
    def box(label, location, scale, mat):
        actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*location))
        actor.set_actor_label(label)
        actor.set_actor_scale3d(unreal.Vector(*scale))
        component = actor.static_mesh_component
        component.set_static_mesh(cube)
        component.set_material(0, mat)
        component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        return actor
    box('Space background', (0,0,-100), (14,23,.1), floor_mat)
    for x in (-550,550):
        box('Arena horizontal border', (x,0,-40), (.025,19,.02), border_mat)
    for y in (-950,950):
        box('Arena vertical border', (0,y,-40), (11,.025,.02), border_mat)
    rng = random.Random(20641)
    for i in range(75):
        size = rng.uniform(.013, .035)
        box(f'Star {i+1:02}', (rng.uniform(-545,545),rng.uniform(-940,940),-60), (size,size,.01), star_mat)
    actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0,0,0))
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    world.get_world_settings().set_editor_property('default_game_mode', library.load_blueprint_class('/Game/Blueprints/BP_SpaceGameMode'))
    level.save_current_level()

assert library.does_asset_exist(map_path)
assert mode_defaults.get_editor_property('default_pawn_class') == ship_class
unreal.log('TP1_STARTER_CONTENT_OK: map, materials and Blueprint classes saved.')
