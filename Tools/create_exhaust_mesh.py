"""Author original faceted static meshes directly in Unreal; no external asset pack.
Run with the Geometry Scripting editor plugin. Existing meshes are never overwritten.
"""
import math, unreal
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

flame=Mesh()
for y in [-29,29]:
    flame.tri((-43,y-5,5),(-43,y+5,5),(-80,y,5),(.04,.9,1.4),False)
    flame.tri((-43,y-2,6),(-43,y+2,6),(-67,y,6),(.7,1.5,1.8),False)
flame_mesh=flame.save('SM_K07_Exhaust')


bp=lib.load_asset("/Game/Blueprints/BP_Ship")
o=unreal.get_default_object(lib.load_blueprint_class("/Game/Blueprints/BP_Ship"))
o.get_editor_property("engine_glow").set_static_mesh(flame_mesh)
o.get_editor_property("engine_glow").set_material(0,MAT)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert lib.save_loaded_asset(bp)
