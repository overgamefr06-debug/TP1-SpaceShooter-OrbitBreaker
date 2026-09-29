"""Create the combat Blueprints and add a small, explicitly temporary shooting range."""
import unreal

assets = unreal.AssetToolsHelpers.get_asset_tools()
library = unreal.EditorAssetLibrary

def make_material(name, color, shaded=False):
    path='/Game/Materials/'+name
    if library.does_asset_exist(path):
        return library.load_asset(path)
    mat=assets.create_asset(name,'/Game/Materials',unreal.Material,unreal.MaterialFactoryNew())
    mat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    edit=unreal.MaterialEditingLibrary
    color_node=edit.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector,-500,0)
    color_node.set_editor_property('constant',unreal.LinearColor(*color,1))
    result=color_node
    if shaded:
        normal=edit.create_material_expression(mat,unreal.MaterialExpressionVertexNormalWS,-800,200)
        light=edit.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector,-800,350)
        light.set_editor_property('constant',unreal.LinearColor(.5,-.35,.8,1))
        dot=edit.create_material_expression(mat,unreal.MaterialExpressionDotProduct,-600,200)
        edit.connect_material_expressions(normal,'',dot,'A')
        edit.connect_material_expressions(light,'',dot,'B')
        floor=edit.create_material_expression(mat,unreal.MaterialExpressionMax,-420,200)
        floor.set_editor_property('const_b',.22)
        edit.connect_material_expressions(dot,'',floor,'A')
        product=edit.create_material_expression(mat,unreal.MaterialExpressionMultiply,-220,0)
        edit.connect_material_expressions(color_node,'',product,'A')
        edit.connect_material_expressions(floor,'',product,'B')
        result=product
    edit.connect_material_property(result,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    edit.recompile_material(mat)
    library.save_loaded_asset(mat)
    return mat

def make_blueprint(name,parent):
    path='/Game/Blueprints/'+name
    if library.does_asset_exist(path):
        bp=library.load_asset(path)
    else:
        factory=unreal.BlueprintFactory()
        factory.set_editor_property('parent_class',unreal.load_class(None,parent))
        bp=assets.create_asset(name,'/Game/Blueprints',unreal.Blueprint,factory)
    cls=library.load_blueprint_class(path)
    return bp,cls,unreal.get_default_object(cls)

def save(bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert library.save_loaded_asset(bp), bp.get_name()

laser=make_material('M_Laser',(0.12,2.5,3.5))
rock=make_material('M_Asteroid',(.55,.27,.10),True)
burst=make_material('M_Destruction',(3.5,.9,.12))

shot_bp,shot_cls,shot=make_blueprint('BP_Projectile','/Script/SpaceShooter.ShotProjectile')
shot.set_editor_property('speed',1400)
shot.set_editor_property('lifetime',2)
shot.get_editor_property('mesh').set_material(0,laser)
save(shot_bp)

flash_bp,flash_cls,flash=make_blueprint('BP_MuzzleFlash','/Script/SpaceShooter.CombatBurst')
flash.set_editor_property('duration',.12)
flash.set_editor_property('expansion_radius',23)
flash.set_editor_property('fragment_size',4)
flash.set_editor_property('material',laser)
save(flash_bp)

burst_bp,burst_cls,burst_obj=make_blueprint('BP_AsteroidBurst','/Script/SpaceShooter.CombatBurst')
burst_obj.set_editor_property('duration',.45)
burst_obj.set_editor_property('expansion_radius',115)
burst_obj.set_editor_property('fragment_size',10)
burst_obj.set_editor_property('material',burst)
save(burst_bp)

asteroid_bp,asteroid_cls,asteroid=make_blueprint('BP_Asteroid','/Script/SpaceShooter.SpaceAsteroid')
asteroid.set_editor_property('minimum_hits',1)
asteroid.set_editor_property('maximum_hits',3)
asteroid.set_editor_property('destruction_effect_class',burst_cls)
asteroid.get_editor_property('mesh').set_material(0,rock)
save(asteroid_bp)

ship_bp=library.load_asset('/Game/Blueprints/BP_Ship')
ship=unreal.get_default_object(library.load_blueprint_class('/Game/Blueprints/BP_Ship'))
ship.set_editor_property('projectile_class',shot_cls)
ship.set_editor_property('muzzle_effect_class',flash_cls)
ship.set_editor_property('fire_interval',.22)
save(ship_bp)

level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert level.load_level('/Game/Maps/L_Arena')
actor_tools=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
labels={a.get_actor_label() for a in actor_tools.get_all_level_actors()}
for i,y in enumerate([-350,0,350]):
    label=f'TestAsteroid_{i+1}'
    if label not in labels:
        actor=actor_tools.spawn_actor_from_class(asteroid_cls,unreal.Vector(320,y,0))
        actor.set_actor_label(label)
assert level.save_current_level()
unreal.log('TP1_COMBAT_CONTENT_OK')
