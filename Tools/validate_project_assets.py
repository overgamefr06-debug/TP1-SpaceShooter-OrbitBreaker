"""Read-only dependency/default audit. Run with UnrealEditor-Cmd -run=pythonscript.
Native dynamic asset loads must be added to roots when introduced.
"""
import unreal,json
from pathlib import Path
r=unreal.AssetRegistryHelpers.get_asset_registry();r.search_all_assets(True)
opts=unreal.AssetRegistryDependencyOptions(include_soft_package_references=True,include_hard_package_references=True,include_searchable_names=False,include_soft_management_references=True,include_hard_management_references=True)
assets={str(a.package_name):a for a in r.get_assets_by_path('/Game',True)}
rows={p:{'class':str(a.asset_class_path.asset_name),'dependencies':sorted(str(x) for x in r.get_dependencies(p,opts) if str(x).startswith('/Game/')),'referencers':sorted(str(x) for x in r.get_referencers(p,opts) if str(x).startswith('/Game/'))} for p,a in assets.items()}
roots=['/Game/Maps/L_Arena','/Game/Blueprints/BP_SpaceGameMode'];seen=set();todo=roots[:]
while todo:
 p=todo.pop()
 if p in seen:continue
 seen.add(p);todo+=rows.get(p,{}).get('dependencies',[])
for p,v in rows.items():v['reachable']=p in seen
bprops={}
for p,a in assets.items():
 if str(a.asset_class_path.asset_name)!='Blueprint':continue
 cls=unreal.EditorAssetLibrary.load_blueprint_class(p);o=unreal.get_default_object(cls);props={}
 for key in ['shockwave_mesh','shockwave_material','sound','shield_mesh','shield_material','rock_materials','starting_lives','minimum_spawn_delay','maximum_spawn_delay','random_size','hits_by_size','points_by_size','move_speed','fire_interval','repair_probability','double_score_duration','shield_duration','triple_shot_duration']:
  try:props[key]=str(o.get_editor_property(key))
  except Exception:pass
 props['components']=[]
 if hasattr(o,'get_components_by_class'):
  for c in o.get_components_by_class(unreal.StaticMeshComponent):
   props['components'].append({'name':c.get_name(),'mesh':str(c.get_editor_property('static_mesh')),'materials':[str(x) for x in c.get_materials()]})
 bprops[p]=props
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);level.load_level('/Game/Maps/L_Arena')
actors=[]
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
 actors.append({'label':a.get_actor_label(),'class':a.get_class().get_name()})
report={'assets':rows,'blueprints':bprops,'actors':actors,'unreachable':sorted(set(rows)-seen)}
output=Path(unreal.Paths.project_dir())/'Artifacts'/'AssetValidation.json'
output.parent.mkdir(parents=True,exist_ok=True)
report['missing_dependencies']=sorted({d for row in rows.values() for d in row['dependencies'] if d not in rows})
output.write_text(json.dumps(report,indent=2),encoding='utf-8')
assert not report['missing_dependencies'],report['missing_dependencies']
assert not report['unreachable'],report['unreachable']
unreal.log('ASSET_AUDIT_OK '+str(len(rows))+' assets; unused: '+str(len(report['unreachable'])))

