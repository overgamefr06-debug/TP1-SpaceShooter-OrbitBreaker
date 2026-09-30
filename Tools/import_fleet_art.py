"""Import original generated fleet sprites, bind Blueprint settings, preserve alpha.
Check out BP_Ship, BP_OrbitHUD and BP_Asteroid before running in Unreal Python.
ArtSources/Fleet/Generation.md records the source prompts. No external asset pack.
"""
from pathlib import Path
import unreal

lib=unreal.EditorAssetLibrary
tools=unreal.AssetToolsHelpers.get_asset_tools()
root='/Game/Art/Fleet'
source=Path(__file__).resolve().parents[1]/'ArtSources'/'Fleet'
textures=[]
for name in ['Aegis','Spectre','Helios']:
    path=root+'/T_'+name
    if not lib.does_asset_exist(path):
        task=unreal.AssetImportTask()
        task.filename=str(source/('T_'+name+'.png'))
        task.destination_path=root; task.destination_name='T_'+name
        task.automated=True; task.save=True
        tools.import_asset_tasks([task])
    tex=lib.load_asset(path)
    assert tex,path
    tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_DEFAULT)
    tex.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    tex.set_editor_property('srgb',True)
    tex.set_editor_property('max_texture_size',1024)
    lib.save_loaded_asset(tex)
    textures.append(tex)

materials=[]
for name,tex in zip(['Aegis','Spectre','Helios'],textures):
    path=root+'/M_'+name
    if lib.does_asset_exist(path): m=lib.load_asset(path)
    else:
        m=tools.create_asset('M_'+name,root,unreal.Material,unreal.MaterialFactoryNew())
        m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
        m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_MASKED)
        m.set_editor_property('two_sided',True)
        m.set_editor_property('opacity_mask_clip_value',.15)
        node=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionTextureSample,0,0)
        node.set_editor_property('texture',tex)
        unreal.MaterialEditingLibrary.connect_material_property(node,'RGB',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        unreal.MaterialEditingLibrary.connect_material_property(node,'A',unreal.MaterialProperty.MP_OPACITY_MASK)
        unreal.MaterialEditingLibrary.recompile_material(m)
        lib.save_loaded_asset(m)
    materials.append(m)

path=root+'/SM_FleetSprite'
if lib.does_asset_exist(path): mesh=lib.load_asset(path)
else:
    # Explicit UVs: texture up maps to world +X, right to world +Y.
    b=unreal.GeometryScriptSimpleMeshBuffers()
    b.vertices=[unreal.Vector(80,-80,0),unreal.Vector(80,80,0),unreal.Vector(-80,80,0),unreal.Vector(-80,-80,0)]
    b.normals=[unreal.Vector(0,0,1)]*4
    b.uv0=[unreal.Vector2D(0,0),unreal.Vector2D(1,0),unreal.Vector2D(1,1),unreal.Vector2D(0,1)]
    b.triangles=[unreal.IntVector(0,1,2),unreal.IntVector(0,2,3)]
    b.vertex_colors=[unreal.LinearColor(1,1,1,1)]*4
    dyn=unreal.DynamicMesh()
    unreal.GeometryScript_MeshEdits.append_buffers_to_mesh(dyn,b,0,False)
    options=unreal.GeometryScriptCreateNewStaticMeshAssetOptions(); options.enable_collision=False
    result=unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(dyn,path,options)
    mesh=result[0] if isinstance(result,tuple) else result
    mesh.set_material(0,materials[0]); lib.save_loaded_asset(mesh)

def defaults(name):
    bp=lib.load_asset('/Game/Blueprints/'+name)
    return bp,unreal.get_default_object(lib.load_blueprint_class('/Game/Blueprints/'+name))
def save(bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert lib.save_loaded_asset(bp)

bp,o=defaults('BP_Ship')
h=o.get_editor_property('hull'); h.set_static_mesh(mesh); h.set_material(0,materials[0])
h.set_relative_rotation(unreal.Rotator(0,0,0),False,False)
h.set_relative_scale3d(unreal.Vector(.68,.68,.68))
o.get_editor_property('engine_glow').set_relative_location(unreal.Vector(-23,0,-2),False,False)
o.set_editor_property('ship_materials',materials)
o.set_editor_property('muzzle_offset',56.)
save(bp)
bp,o=defaults('BP_OrbitHUD')
o.set_editor_property('ship_portraits',textures[:3]); save(bp)
bp,o=defaults('BP_Asteroid')
o.set_editor_property('random_size',True)
o.set_editor_property('size_scales',unreal.Vector(.55,.95,1.45))
o.set_editor_property('hits_by_size',unreal.IntVector(1,2,3))
o.set_editor_property('points_by_size',unreal.IntVector(100,200,400)); save(bp)
unreal.log('FLEET_ART_IMPORTED_OK')
