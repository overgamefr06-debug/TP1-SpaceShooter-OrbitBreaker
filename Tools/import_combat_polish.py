"""Run after import_effects_content.py and generate_laser_audio.py.
Check out BP_Asteroid, BP_Projectile, BP_RockCollision, BP_AsteroidBurst, BP_MuzzleFlash.
All combat VFX now use alpha/additive planes; no opaque primitive fragments.
"""
from pathlib import Path
import unreal
lib=unreal.EditorAssetLibrary
tools=unreal.AssetToolsHelpers.get_asset_tools()
edit=unreal.MaterialEditingLibrary
root='/Game/Art/CombatPolish'
lib.make_directory(root)

def make(name,blend):
    path=root+'/'+name
    if lib.does_asset_exist(path):
        m=lib.load_asset(path); edit.delete_all_material_expressions(m)
    else: m=tools.create_asset(name,root,unreal.Material,unreal.MaterialFactoryNew())
    m.set_editor_property('blend_mode',blend)
    m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    m.set_editor_property('two_sided',True)
    m.set_editor_property('disable_depth_test',True)
    return m

def save_mat(m):
    edit.recompile_material(m); assert lib.save_loaded_asset(m)
    return m

def energy(name,code):
    m=make(name,unreal.BlendMode.BLEND_ADDITIVE)
    node=edit.create_material_expression(m,unreal.MaterialExpressionCustom,0,0)
    node.set_editor_property('code',code)
    node.set_editor_property('output_type',unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    inputs=[]
    for key in ['UV','Strength']:
        v=unreal.CustomInput(); v.set_editor_property('input_name',key); inputs.append(v)
    node.set_editor_property('inputs',inputs)
    uv=edit.create_material_expression(m,unreal.MaterialExpressionTextureCoordinate,-300,0)
    strength=edit.create_material_expression(m,unreal.MaterialExpressionScalarParameter,-300,150)
    strength.set_editor_property('parameter_name','Strength'); strength.set_editor_property('default_value',1.)
    edit.connect_material_expressions(uv,'',node,'UV')
    edit.connect_material_expressions(strength,'',node,'Strength')
    edit.connect_material_property(node,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    opacity=edit.create_material_expression(m,unreal.MaterialExpressionConstant,150,150)
    opacity.set_editor_property('r',1.)
    edit.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
    return save_mat(m)

burst=energy('M_RockBurst','''
float2 p=(UV-.5)*2;
float r=length(p);
float a=atan2(p.y,p.x);
float dust=.13*exp(-abs(r-.58)*24);
float sparks=0;
for(int i=0;i<13;i++) {
 float angle=i*6.2831853/13.0;
 float2 dir=float2(cos(angle),sin(angle));
 float radius=.42+.30*frac(sin(i*127.1+13.0)*43758.5453);
 float along=dot(p,dir)-radius;
 float side=dot(p,float2(-dir.y,dir.x));
 sparks+=exp(-along*along*130-side*side*2200);
}
float core=.32*exp(-r*r*35);
return (float3(.65,.36,.12)*dust+float3(1.9,.82,.24)*sparks+float3(1.5,.64,.16)*core)*Strength;
''')
laser=energy('M_LaserPulse','''
float2 p=(UV-.5)*2;
float lengthMask=1-smoothstep(.62,.91,abs(p.y));
float core=exp(-p.x*p.x*160)*lengthMask;
float glow=.18*exp(-p.x*p.x*20)*lengthMask;
return (float3(.65,2.4,3.0)*core+float3(.04,.5,1.3)*glow)*Strength;
''')
mesh=lib.load_asset('/Game/Art/Fleet/SM_FleetSprite')

def bp(name):
    return lib.load_asset('/Game/Blueprints/'+name),unreal.get_default_object(lib.load_blueprint_class('/Game/Blueprints/'+name))
def save(b):
    unreal.BlueprintEditorLibrary.compile_blueprint(b); assert lib.save_loaded_asset(b)

rocks=[]
for name in ['RockA','RockB']:
    m=make('M_'+name,unreal.BlendMode.BLEND_TRANSLUCENT)
    t=edit.create_material_expression(m,unreal.MaterialExpressionTextureSample,0,0)
    t.set_editor_property('texture',lib.load_asset('/Game/Art/Arcade/T_'+name))
    edit.connect_material_property(t,'RGB',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    edit.connect_material_property(t,'A',unreal.MaterialProperty.MP_OPACITY)
    rocks.append(save_mat(m))
b,o=bp('BP_Asteroid')
o.set_editor_property('rock_materials',rocks); o.get_editor_property('mesh').set_material(0,rocks[0]); save(b)
b,o=bp('BP_Projectile')
v=o.get_editor_property('mesh'); v.set_static_mesh(mesh); v.set_material(0,laser)
v.set_relative_scale3d(unreal.Vector(.27,.10,1.))
v.set_editor_property('cast_shadow',False)
v.set_editor_property('translucency_sort_priority',2)
save(b)
for name in ['BP_RockCollision','BP_AsteroidBurst']:
    b,o=bp(name)
    o.set_editor_property('fragment_count',0)
    o.set_editor_property('shockwave_mesh',mesh)
    o.set_editor_property('shockwave_material',burst)
    o.set_editor_property('expansion_radius',95.)
    save(b)

source=Path(__file__).resolve().parents[1]/'ArtSources'/'Effects'
sounds=[]
for i in range(1,4):
    name=f'S_LaserPulse_{i:02}'
    path=root+'/'+name
    if not lib.does_asset_exist(path):
        task=unreal.AssetImportTask(); task.filename=str(source/(name+'.wav'))
        task.destination_path=root; task.destination_name=name; task.automated=True; task.save=True
        tools.import_asset_tasks([task])
    sounds.append(lib.load_asset(path))
b,o=bp('BP_MuzzleFlash')
o.set_editor_property('sound',None); o.set_editor_property('sound_variants',sounds)
save(b)
unreal.log('COMBAT_POLISH_IMPORTED_OK')
