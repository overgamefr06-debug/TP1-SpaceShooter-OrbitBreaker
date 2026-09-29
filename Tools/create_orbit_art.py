"""Author original faceted static meshes directly in Unreal; no external asset pack.
Run with the Geometry Scripting editor plugin. Existing meshes are never overwritten.
"""
import math, random, unreal
from pathlib import Path

lib=unreal.EditorAssetLibrary
tools=unreal.AssetToolsHelpers.get_asset_tools()
ROOT='/Game/Art'

def vertex_material():
    path=ROOT+'/M_Faceted'
    if lib.does_asset_exist(path):
        m=lib.load_asset(path)
        unreal.MaterialEditingLibrary.delete_all_material_expressions(m)
    else: m=tools.create_asset('M_Faceted',ROOT,unreal.Material,unreal.MaterialFactoryNew())
    m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    m.set_editor_property('two_sided',True)
    n=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionVertexColor,0,0)
    assert unreal.MaterialEditingLibrary.connect_material_property(n,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(m)
    lib.save_loaded_asset(m)
    return m

MAT=vertex_material()
def sub(a,b): return tuple(x-y for x,y in zip(a,b))
def normal(a,b,c):
    u,v=sub(b,a),sub(c,a)
    n=(u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0])
    d=math.sqrt(sum(x*x for x in n)) or 1
    return tuple(x/d for x in n)

class Mesh:
    def __init__(self): self.v=[]; self.n=[]; self.c=[]; self.t=[]
    def tri(self,a,b,c,color,shade=True):
        n=normal(a,b,c)
        light=.48+.52*max(0,n[0]*.35-n[1]*.45+n[2]*.82) if shade else 1
        i=len(self.v)
        self.v.extend([a,b,c]); self.n.extend([n]*3)
        self.c.extend([tuple(x*light for x in color)]*3); self.t.append((i,i+1,i+2))
    def polygon(self,points,color,shade=True):
        for i in range(1,len(points)-1): self.tri(points[0],points[i],points[i+1],color,shade)
    def prism(self,outline,z0,z1,color):
        top=[(x,y,z1) for x,y in outline]; bottom=[(x,y,z0) for x,y in outline]
        self.polygon(top,color)
        self.polygon(list(reversed(bottom)),color)
        for i in range(len(top)):
            j=(i+1)%len(top)
            self.polygon([bottom[i],bottom[j],top[j],top[i]],tuple(c*.65 for c in color))
    def save(self,name):
        path=ROOT+'/Meshes/'+name
        if lib.does_asset_exist(path): return lib.load_asset(path)
        buffers=unreal.GeometryScriptSimpleMeshBuffers()
        buffers.vertices=[unreal.Vector(*v) for v in self.v]
        buffers.normals=[unreal.Vector(*n) for n in self.n]
        buffers.vertex_colors=[unreal.LinearColor(*c,1) for c in self.c]
        buffers.triangles=[unreal.IntVector(*t) for t in self.t]
        buffers.uv0=[unreal.Vector2D(v[0]/100,v[1]/100) for v in self.v]
        mesh=unreal.DynamicMesh()
        unreal.GeometryScript_MeshEdits.append_buffers_to_mesh(mesh,buffers,0,False)
        options=unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
        options.enable_collision=False
        result=unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(mesh,path,options)
        asset=result[0] if isinstance(result,tuple) else result
        assert asset, path
        asset.set_material(0,MAT)
        lib.save_loaded_asset(asset)
        unreal.log(f'ORBIT_MESH {name}: {len(self.t)} triangles')
        return asset

ivory=(.78,.88,.96); graphite=(.10,.17,.24); blue=(.035,.32,.55); gold=(.95,.35,.075)
ship=Mesh()
# A raised, faceted central fuselage with swept wings and two engine nacelles.
for sign in [-1,1]:
    wing=[(20,sign*12),(-20,sign*55),(-45,sign*49),(-30,sign*15)]
    if sign<0: wing.reverse()
    ship.prism(wing,-5,1,ivory)
    plate=[(-17,sign*37),(-25,sign*50),(-35,sign*48),(-25,sign*34)]
    if sign<0: plate.reverse()
    ship.prism(plate,1,2.5,gold)
    nacelle=[(-13,sign*27),(-20,sign*34),(-44,sign*34),(-47,sign*25)]
    if sign<0: nacelle.reverse()
    ship.prism(nacelle,0,10,graphite)
    ship.prism([(-44,sign*30-4),(-38,sign*30-4),(-38,sign*30+4),(-44,sign*30+4)],10,11,(.10,.9,1.25))
    ship.tri((54,0,0),(-32,sign*17,0),(-14,0,16),ivory)
    ship.tri((-14,0,16),(-32,sign*17,0),(-43,0,3),graphite)
ship.prism([(21,0),(6,-8),(-12,-6),(-16,0),(-12,6),(6,8)],13,16,blue)
ship.tri((20,0,16),(6,-8,16),(-10,0,19),(.16,.75,1.2),False)
ship.tri((20,0,16),(-10,0,19),(6,8,16),(.07,.35,.65),False)
ship.prism([(37,-2),(51,0),(37,2),(24,0)],1,2,gold)
ship_mesh=ship.save('SM_K07_Interceptor')
flame=Mesh()
for y in [-29,29]:
    flame.tri((-43,y-5,5),(-43,y+5,5),(-80,y,5),(.04,.9,1.4),False)
    flame.tri((-43,y-2,6),(-43,y+2,6),(-67,y,6),(.7,1.5,1.8),False)
flame_mesh=flame.save('SM_K07_Exhaust')

def rock(name,seed):
    rng=random.Random(seed); m=Mesh(); seg=18; rings=11
    crater_centres=[(.6,-.45,.65),(-.3,.4,.85),(-.7,-.3,.55)]
    points=[]
    for j in range(rings+1):
        phi=math.pi*j/rings
        row=[]
        for i in range(seg):
            theta=2*math.pi*i/seg
            d=(math.sin(phi)*math.cos(theta),math.sin(phi)*math.sin(theta),math.cos(phi))
            radius=51*(1+rng.uniform(-.13,.13)+.10*math.sin(theta*3+seed)*math.sin(phi))
            for centre in crater_centres:
                angle=sum(a*b for a,b in zip(d,centre))
                if angle>.91: radius*=.76
                elif angle>.84: radius*=1.04
            row.append((d[0]*radius,d[1]*radius*.92,d[2]*radius*.85))
        points.append(row)
    for j in range(rings):
        for i in range(seg):
            k=(i+1)%seg
            for a,b,c in [(points[j][i],points[j+1][i],points[j+1][k]),(points[j][i],points[j+1][k],points[j][k])]:
                if j==0 and a==b or j==rings-1 and b==c: continue
                shade=rng.uniform(.65,1.15)
                color=(.40*shade,.31*shade,.25*shade)
                if rng.random()<.04: color=(.13,.4,.46)
                m.tri(a,b,c,color)
    return m.save(name)
rocks=[rock('SM_Asteroid_A',17),rock('SM_Asteroid_B',51),rock('SM_Asteroid_C',93)]

def bp(name,parent):
    path='/Game/Blueprints/'+name
    if lib.does_asset_exist(path): asset=lib.load_asset(path)
    else:
        f=unreal.BlueprintFactory(); f.set_editor_property('parent_class',unreal.load_class(None,parent))
        asset=tools.create_asset(name,'/Game/Blueprints',unreal.Blueprint,f)
    cls=lib.load_blueprint_class(path)
    return asset,cls,unreal.get_default_object(cls)
def save(b):
    unreal.BlueprintEditorLibrary.compile_blueprint(b)
    assert lib.save_loaded_asset(b)

b,c,o=bp('BP_Ship','/Script/SpaceShooter.ShipPawn')
h=o.get_editor_property('hull'); h.set_static_mesh(ship_mesh); h.set_material(0,MAT)
h.set_relative_rotation(unreal.Rotator(0,0,0),False,False)
h.set_relative_scale3d(unreal.Vector(1,1,1))
o.get_editor_property('wings').set_static_mesh(None)
o.get_editor_property('engine_glow').set_static_mesh(flame_mesh)
o.get_editor_property('engine_glow').set_material(0,MAT)
o.set_editor_property('damage_effect_class',lib.load_blueprint_class('/Game/Blueprints/BP_AsteroidBurst'))
save(b)
b,c,o=bp('BP_Asteroid','/Script/SpaceShooter.SpaceAsteroid')
o.set_editor_property('mesh_variants',rocks)
o.get_editor_property('mesh').set_static_mesh(rocks[0]); o.get_editor_property('mesh').set_material(0,MAT)
o.get_editor_property('mesh').set_relative_scale3d(unreal.Vector(1,1,1))
save(b)
b,hud_cls,o=bp('BP_OrbitHUD','/Script/SpaceShooter.SpaceHUD')
save(b)
b,c,o=bp('BP_SpaceGameMode','/Script/SpaceShooter.SpaceGameMode')
o.set_editor_property('asteroid_class',lib.load_blueprint_class('/Game/Blueprints/BP_Asteroid'))
o.set_editor_property('hud_class',hud_cls)
save(b)

level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level.load_level('/Game/Maps/L_Arena')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for a in actors.get_all_level_actors():
    if a.get_actor_label().startswith('TestAsteroid_'): actors.destroy_actor(a)

source=Path(__file__).resolve().parents[1]/'ArtSources'
def import_asset(filename,name):
    path=ROOT+'/'+name
    if lib.does_asset_exist(path): return lib.load_asset(path)
    task=unreal.AssetImportTask()
    task.filename=str(source/filename)
    task.destination_path=ROOT
    task.destination_name=name
    task.automated=True; task.save=True
    tools.import_asset_tasks([task])
    return lib.load_asset(path)
tex=import_asset('T_OrbitNebula.png','T_OrbitNebula')
assert tex
path=ROOT+'/M_OrbitNebula'
if lib.does_asset_exist(path): backdrop=lib.load_asset(path)
else:
    backdrop=tools.create_asset('M_OrbitNebula',ROOT,unreal.Material,unreal.MaterialFactoryNew())
    backdrop.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    sampler=unreal.MaterialEditingLibrary.create_material_expression(backdrop,unreal.MaterialExpressionTextureSample,0,0)
    sampler.set_editor_property('texture',tex)
    unreal.MaterialEditingLibrary.connect_material_property(sampler,'RGB',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(backdrop)
    lib.save_loaded_asset(backdrop)
for a in actors.get_all_level_actors():
    if a.get_actor_label()=='Space background':
        a.static_mesh_component.set_static_mesh(lib.load_asset('/Engine/BasicShapes/Plane'))
        a.static_mesh_component.set_material(0,backdrop)
        a.set_actor_scale3d(unreal.Vector(12,21.34,1))
    if a.get_actor_label().startswith('Star '): actors.destroy_actor(a)
laser=import_asset('S_Laser.wav','S_Laser')
impact=import_asset('S_Impact.wav','S_Impact')
for name,sound in [('BP_MuzzleFlash',laser),('BP_AsteroidBurst',impact)]:
    b,c,o=bp(name,'/Script/SpaceShooter.CombatBurst')
    o.set_editor_property('sound',sound)
    save(b)
level.save_current_level()
unreal.log('ORBIT_ART_OK')
