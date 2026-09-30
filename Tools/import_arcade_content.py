"""Import arcade art and configure real Blueprint subclasses (UE 5.8 Python).
Check out existing BP_Ship, BP_Asteroid, BP_OrbitHUD, BP_SpaceGameMode and L_Arena.
Run generate_arcade_audio.py first. Generated PNG provenance: ArtSources/Arcade.
"""
from pathlib import Path
import math
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

names = ['RockA','RockB','DoubleScore','Shield','Repair','TripleShot']
textures = []
materials = []
for name in names:
    tex = imported('T_' + name, '.png')
    tex.set_editor_property('compression_settings', unreal.TextureCompressionSettings.TC_DEFAULT)
    tex.set_editor_property('lod_group', unreal.TextureGroup.TEXTUREGROUP_UI)
    tex.set_editor_property('srgb', True)
    tex.set_editor_property('max_texture_size', 1024)
    lib.save_loaded_asset(tex)
    textures.append(tex)
    path = root + '/M_' + name
    if lib.does_asset_exist(path): mat = lib.load_asset(path)
    else:
        mat = tools.create_asset('M_' + name, root, unreal.Material, unreal.MaterialFactoryNew())
        mat.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
        mat.set_editor_property('blend_mode', unreal.BlendMode.BLEND_MASKED)
        mat.set_editor_property('two_sided', True)
        mat.set_editor_property('opacity_mask_clip_value', .15)
        node = edit.create_material_expression(mat, unreal.MaterialExpressionTextureSample, 0, 0)
        node.set_editor_property('texture', tex)
        edit.connect_material_property(node, 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        edit.connect_material_property(node, 'A', unreal.MaterialProperty.MP_OPACITY_MASK)
        edit.recompile_material(mat); lib.save_loaded_asset(mat)
    materials.append(mat)

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

def solid(name, color):
    path = root + '/' + name
    if lib.does_asset_exist(path): return lib.load_asset(path)
    m = tools.create_asset(name, root, unreal.Material, unreal.MaterialFactoryNew())
    m.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    m.set_editor_property('two_sided', True)
    n = edit.create_material_expression(m, unreal.MaterialExpressionConstant3Vector, 0, 0)
    n.set_editor_property('constant', unreal.LinearColor(*color, 1))
    edit.connect_material_property(n, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    edit.recompile_material(m); lib.save_loaded_asset(m)
    return m

ring_mat = solid('M_DustRing', (.5,.29,.12))
debris_mat = solid('M_Debris', (.23,.17,.11))
shield_mat = solid('M_ShieldRing',(.08,.6,1.4))
path = root + '/SM_Shockwave'
if lib.does_asset_exist(path): ring = lib.load_asset(path)
else:
    b = unreal.GeometryScriptSimpleMeshBuffers()
    vertices = []; tris = []
    for i in range(65):
        a = i*math.tau/64
        for r in [48,50]: vertices.append(unreal.Vector(math.cos(a)*r, math.sin(a)*r, 4))
        if i<64: tris.extend([unreal.IntVector(i*2,i*2+1,i*2+3), unreal.IntVector(i*2,i*2+3,i*2+2)])
    b.vertices = vertices; b.normals = [unreal.Vector(0,0,1)]*len(vertices)
    b.uv0 = [unreal.Vector2D(0,0)]*len(vertices)
    b.vertex_colors = [unreal.LinearColor(1,1,1,1)]*len(vertices); b.triangles = tris
    dyn = unreal.DynamicMesh()
    unreal.GeometryScript_MeshEdits.append_buffers_to_mesh(dyn,b,0,False)
    opts = unreal.GeometryScriptCreateNewStaticMeshAssetOptions(); opts.enable_collision = False
    result = unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(dyn,path,opts)
    ring = result[0] if isinstance(result,tuple) else result
    ring.set_material(0,ring_mat); lib.save_loaded_asset(ring)

b,collision_cls,o = bp('BP_RockCollision','CombatBurst')
o.set_editor_property('sound',sound)
o.set_editor_property('duration',.65)
o.set_editor_property('expansion_radius',125.)
o.set_editor_property('fragment_size',14.)
o.set_editor_property('fragment_mesh',lib.load_asset('/Engine/BasicShapes/Sphere'))
o.set_editor_property('material',debris_mat)
o.set_editor_property('shockwave_mesh',ring); o.set_editor_property('shockwave_material',ring_mat)
save(b)

b,c,o = bp('BP_Asteroid','SpaceAsteroid')
o.set_editor_property('mesh_variants',[])
o.set_editor_property('rock_materials',materials[:2])
o.set_editor_property('fragment_grace_seconds',.8)
o.set_editor_property('destruction_effect_class',collision_cls)
v = o.get_editor_property('mesh'); v.set_static_mesh(mesh); v.set_material(0,materials[0])
v.set_relative_scale3d(unreal.Vector(.84,.84,1))
v.set_relative_rotation(unreal.Rotator(0,0,0),False,False)
save(b)
b,c,o = bp('BP_Ship','ShipPawn')
o.get_editor_property('hull').set_relative_scale3d(unreal.Vector(.68,.68,.68))
o.get_editor_property('engine_glow').set_relative_location(unreal.Vector(-23,0,-2),False,False)
o.get_editor_property('collision').set_box_extent(unreal.Vector(27,31,18),False)
o.set_editor_property('shield_mesh',ring)
o.set_editor_property('shield_material',shield_mat)
o.set_editor_property('muzzle_offset',56.)
save(b)

pickup_classes = []
types = [unreal.SpaceBonus.DOUBLE_SCORE,unreal.SpaceBonus.SHIELD,unreal.SpaceBonus.REPAIR,unreal.SpaceBonus.TRIPLE_SHOT]
for i,name in enumerate(names[2:]):
    b,c,o = bp('BP_Bonus'+name,'SpacePickup')
    o.set_editor_property('bonus_type',types[i]); o.set_editor_property('lifetime',12.)
    o.set_editor_property('collect_sound',chime)
    v = o.get_editor_property('visual'); v.set_static_mesh(mesh); v.set_material(0,materials[i+2])
    v.set_relative_scale3d(unreal.Vector(.43,.43,.43))
    save(b); pickup_classes.append(c)
b,c,o = bp('BP_SpaceGameMode','SpaceGameMode')
o.set_editor_property('pickup_classes',pickup_classes)
o.set_editor_property('collision_effect_class',collision_cls)
o.set_editor_property('double_score_duration',15.)
o.set_editor_property('shield_duration',10.)
o.set_editor_property('triple_shot_duration',15.)
o.set_editor_property('repair_probability',.05)
o.set_editor_property('unlock_scores',unreal.IntVector(0,5000,15000))
save(b)
b,c,o = bp('BP_OrbitHUD','SpaceHUD')
o.set_editor_property('bonus_icons',textures[2:]); save(b)

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
    if label.startswith('Arena ') and 'border' in label: a.set_actor_hidden_in_game(True)
    if label=='Space background': a.static_mesh_component.set_material(0,backdrop)
assert level.save_current_level()
unreal.log('ARCADE_CONTENT_IMPORTED_OK')
