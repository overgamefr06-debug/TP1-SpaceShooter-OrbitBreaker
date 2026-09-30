"""Import original soundtrack/UI cues and configure the actual GameMode Blueprint.
Check Out BP_SpaceGameMode and existing Soundtrack assets before rerunning.
"""
from pathlib import Path
import unreal

source=Path(__file__).resolve().parents[1]/'ArtSources'/'Soundtrack'
root='/Game/Audio/Soundtrack'
lib=unreal.EditorAssetLibrary
lib.make_directory(root)
names=['M_QuietOrbit','M_BreakerRun','UI_Hover','UI_Select','UI_Confirm','UI_Back']
sounds=[]
for name in names:
    task=unreal.AssetImportTask()
    task.filename=str(source/(name+'.wav'))
    task.destination_path=root
    task.destination_name=name
    task.automated=True
    task.replace_existing=True
    task.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    sound=lib.load_asset(root+'/'+name)
    assert sound, name
    sound.set_editor_property('looping',name.startswith('M_'))
    sound.set_editor_property('volume',1.)
    sound.set_editor_property('compression_quality',70 if name.startswith('M_') else 90)
    sound.set_editor_property('virtualization_mode',unreal.VirtualizationMode.PLAY_WHEN_SILENT)
    assert lib.save_loaded_asset(sound)
    sounds.append(sound)
b=lib.load_asset('/Game/Blueprints/BP_SpaceGameMode')
o=unreal.get_default_object(lib.load_blueprint_class('/Game/Blueprints/BP_SpaceGameMode'))
o.set_editor_property('menu_music',sounds[0])
o.set_editor_property('game_music',sounds[1])
o.set_editor_property('interface_sounds',sounds[2:])
o.set_editor_property('menu_music_volume',.55)
o.set_editor_property('game_music_volume',.38)
o.set_editor_property('interface_volume',.5)
unreal.BlueprintEditorLibrary.compile_blueprint(b)
assert lib.save_loaded_asset(b)
unreal.log('SOUNDTRACK_IMPORTED_OK: two looping tracks, four UI cues, BP configured')
