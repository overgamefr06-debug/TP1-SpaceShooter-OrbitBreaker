"""Run last, after import_arcade_content.py. Check out existing BPs first.
Original transparent PNGs: ArtSources/Effects. Shader effects are native UE assets.
"""
from pathlib import Path
import unreal

lib=unreal.EditorAssetLibrary
tools=unreal.AssetToolsHelpers.get_asset_tools()
edit=unreal.MaterialEditingLibrary
root='/Game/Art/Effects'
source=Path(__file__).resolve().parents[1]/'ArtSources'/'Effects'
lib.make_directory(root)

def texture(name):
    path=root+'/T_'+name
    if not lib.does_asset_exist(path):
        t=unreal.AssetImportTask()
        t.filename=str(source/('T_'+name+'.png')); t.destination_path=root
        t.destination_name='T_'+name; t.automated=True; t.save=True
        tools.import_asset_tasks([t])
    obj=lib.load_asset(path)
    obj.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    obj.set_editor_property('srgb',True)
    obj.set_editor_property('max_texture_size',2048 if name=='Logo' else 1024)
    lib.save_loaded_asset(obj)
    return obj

def material(name,blend):
    path=root+'/'+name
    if lib.does_asset_exist(path):
        m=lib.load_asset(path); edit.delete_all_material_expressions(m)
    else: m=tools.create_asset(name,root,unreal.Material,unreal.MaterialFactoryNew())
    m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    m.set_editor_property('blend_mode',blend)
    m.set_editor_property('two_sided',True)
    # Flat arcade overlays must not be clipped by the orthographic depth prepass.
    m.set_editor_property('disable_depth_test',True)
    return m

def finish(m):
    edit.recompile_material(m); assert lib.save_loaded_asset(m)
    return m

def energy(name,code,color):
    m=material(name,unreal.BlendMode.BLEND_ADDITIVE)
    custom=edit.create_material_expression(m,unreal.MaterialExpressionCustom,0,0)
    custom.set_editor_property('code',code)
    custom.set_editor_property('output_type',unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    inputs=[]
    for key in ['UV','Clock','Strength','Impact','Tint']:
        entry=unreal.CustomInput(); entry.set_editor_property('input_name',key); inputs.append(entry)
    custom.set_editor_property('inputs',inputs)
    uv=edit.create_material_expression(m,unreal.MaterialExpressionTextureCoordinate,-500,0)
    time=edit.create_material_expression(m,unreal.MaterialExpressionTime,-500,130)
    strength=edit.create_material_expression(m,unreal.MaterialExpressionScalarParameter,-500,260)
    strength.set_editor_property('parameter_name','Strength'); strength.set_editor_property('default_value',1.)
    impact=edit.create_material_expression(m,unreal.MaterialExpressionScalarParameter,-500,390)
    impact.set_editor_property('parameter_name','Impact'); impact.set_editor_property('default_value',0.)
    tint=edit.create_material_expression(m,unreal.MaterialExpressionVectorParameter,-500,520)
    tint.set_editor_property('parameter_name','Tint'); tint.set_editor_property('default_value',unreal.LinearColor(*color,1))
    for node,key in [(uv,'UV'),(time,'Clock'),(strength,'Strength'),(impact,'Impact'),(tint,'Tint')]:
        edit.connect_material_expressions(node,'',custom,key)
    edit.connect_material_property(custom,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    opacity=edit.create_material_expression(m,unreal.MaterialExpressionConstant,200,200)
    opacity.set_editor_property('r',1.)
    edit.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
    return finish(m)

shield_code='''
float2 p=(UV-.5)*2;
float r=length(p);
float angle=atan2(p.y,p.x);
float wobble=.007*sin(angle*13+Clock*3)+.004*sin(angle*21-Clock*5);
float edge=exp(-abs(r-.80-wobble)*115);
float halo=.20*exp(-abs(r-.80)*24);
float sweep=pow(saturate(.5+.5*sin(angle*3-Clock*2.4)),10);
float arcs=exp(-abs(r-.84)*180)*sweep;
// Tiled hexagonal cells, faint near the perimeter and absent over the cockpit.
float2 q=p*9;
float2 h=float2(1.73205,1);
float2 a=frac(q/h)-.5;
float2 b=frac((q-.5*h)/h)-.5;
a*=h; b*=h;
float2 cell=dot(a,a)<dot(b,b)?a:b;
cell=abs(cell);
float d=max(dot(cell,float2(.866025,.5)),cell.y);
float grid=(1-smoothstep(.015,.055,abs(d-.47)))*smoothstep(.24,.73,r)*(1-smoothstep(.76,.81,r));
float flash=Impact*exp(-abs(r-(.32+.48*(1-Impact)))*35);
float v=edge*(.34+.35*sweep)+halo+arcs*.55+grid*.10+flash*.55;
return Tint.rgb*v*Strength*(1-smoothstep(.90,.98,r));
'''
aura_code='''
float2 p=(UV-.5)*2;
float r=length(p);
float v=.04*exp(-r*r*9);
for(int i=0;i<3;i++) {
 float a=Clock*1.5+i*2.0944;
 float2 pos=float2(cos(a),sin(a))*.76;
 float d=length(p-pos);
 v+=exp(-d*d*2200)*1.8+.14*exp(-d*d*100);
 for(int j=1;j<6;j++) {
  float t=a-j*.055;
  float2 tail=float2(cos(t),sin(t))*.76;
  v+=exp(-dot(p-tail,p-tail)*2600)*.18*(1-j/6.0);
 }
}
return Tint.rgb*v*Strength;
'''
wave_code='''
float2 p=(UV-.5)*2;
float r=length(p);
float a=atan2(p.y,p.x);
float v=exp(-abs(r-.68)*60)*(.55+.25*sin(a*7+Clock*6));
v+=.12*exp(-abs(r-.68)*16);
return Tint.rgb*v*Strength;
'''
flash_code='''
float2 p=(UV-.5)*2;
float v=exp(-dot(p,p)*35);
v+=.3*exp(-abs(p.x)*85-abs(p.y)*5);
v+=.25*exp(-abs(p.y)*85-abs(p.x)*5);
return Tint.rgb*v*Strength;
'''

mesh=lib.load_asset('/Game/Art/Fleet/SM_FleetSprite')
names=['DoubleScore','Shield','Repair','TripleShot']
colors=[(1.8,.85,.10),(.08,.9,1.8),(.1,1.5,.35),(2.,.4,.06)]
icons=[]

def bp(name):
    b=lib.load_asset('/Game/Blueprints/'+name)
    return b,unreal.get_default_object(lib.load_blueprint_class('/Game/Blueprints/'+name))
def save(b):
    unreal.BlueprintEditorLibrary.compile_blueprint(b); assert lib.save_loaded_asset(b)

for name,color in zip(names,colors):
    tex=texture(name); icons.append(tex)
    m=material('M_'+name,unreal.BlendMode.BLEND_TRANSLUCENT)
    n=edit.create_material_expression(m,unreal.MaterialExpressionTextureSample,0,0)
    n.set_editor_property('texture',tex)
    edit.connect_material_property(n,'RGB',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    edit.connect_material_property(n,'A',unreal.MaterialProperty.MP_OPACITY)
    finish(m)
    aura=energy('M_Aura'+name,aura_code,color)
    wave=energy('M_Collect'+name,wave_code,color)
    path='/Game/Blueprints/BP_Collect'+name
    if not lib.does_asset_exist(path):
        f=unreal.BlueprintFactory(); f.set_editor_property('parent_class',unreal.load_class(None,'/Script/SpaceShooter.CombatBurst'))
        tools.create_asset('BP_Collect'+name,'/Game/Blueprints',unreal.Blueprint,f)
    b,o=bp('BP_Collect'+name)
    o.set_editor_property('duration',.55); o.set_editor_property('expansion_radius',58.)
    o.set_editor_property('shockwave_mesh',mesh); o.set_editor_property('shockwave_material',wave)
    save(b)
    collect=lib.load_blueprint_class(path)
    b,o=bp('BP_Bonus'+name)
    o.set_editor_property('aura_mesh',mesh); o.set_editor_property('aura_material',aura)
    o.set_editor_property('collect_effect_class',collect)
    o.get_editor_property('visual').set_material(0,m)
    save(b)

shield=energy('M_EnergyShield',shield_code,(.08,.52,1.3))
b,o=bp('BP_Ship')
o.set_editor_property('shield_mesh',mesh); o.set_editor_property('shield_material',shield)
save(b)
flash=energy('M_MuzzleEnergy',flash_code,(.15,2.2,3.))
b,o=bp('BP_MuzzleFlash')
o.set_editor_property('shockwave_mesh',mesh); o.set_editor_property('shockwave_material',flash)
o.set_editor_property('duration',.10); o.set_editor_property('expansion_radius',17.)
save(b)
b,o=bp('BP_OrbitHUD')
o.set_editor_property('title_logo',texture('Logo')); o.set_editor_property('bonus_icons',icons)
save(b)
unreal.log('EFFECTS_CONTENT_IMPORTED_OK')
