"""Encode the actual Unreal viewport and its master-submix audio on one timeline.

Run the Delivery.RecordDemo test with
  -ini:Engine:[Audio]:UnfocusedVolumeMultiplier=1.0
  -ini:EditorPerProjectUserSettings:[/Script/UnrealEd.LevelEditorMiscSettings]:bAllowBackgroundAudio=True
so offscreen capture is not muted. No desktop/microphone audio is captured.
Usage: python assemble_demo.py --ffmpeg PATH --output PATH
"""
import argparse
import csv
from pathlib import Path
import subprocess
import wave
import numpy as np

parser=argparse.ArgumentParser()
parser.add_argument('--ffmpeg',required=True)
parser.add_argument('--output',required=True)
args=parser.parse_args()
folder=Path(__file__).resolve().parents[1]/'Saved'/'ArcadeDemoCapture'
rows=list(csv.reader((folder/'frames.csv').open(encoding='utf-8-sig')))
first=float(rows[0][1]); last=float(rows[-1][1]); duration=last-first+.1
audio=folder/'DemoAudio.wav'
with wave.open(str(audio)) as wav:
    audio_duration=wav.getnframes()/wav.getframerate()
    samples=np.frombuffer(wav.readframes(wav.getnframes()),dtype='<i2').astype(float)/32768
assert abs(audio_duration-last)<.5, f'Audio clock mismatch: {audio_duration} vs {last}'
assert np.sqrt(np.mean(samples*samples))>.002, 'Recording is silent; do not publish it'
assert np.max(np.abs(samples))<.999, 'Recorded mix clips'
lines=['ffconcat version 1.0']
for i,row in enumerate(rows):
    lines.append("file '"+(folder/row[0]).as_posix()+"'")
    dt=float(rows[i+1][1])-float(row[1]) if i+1<len(rows) else .1
    lines.append(f'duration {dt:.6f}')
lines.append("file '"+(folder/rows[-1][0]).as_posix()+"'")
manifest=folder/'frames.ffconcat'
manifest.write_text('\n'.join(lines),encoding='utf-8')
subprocess.run([args.ffmpeg,'-y','-hide_banner','-loglevel','warning','-safe','0','-f','concat','-i',str(manifest),
                '-ss',f'{first:.6f}','-i',str(audio),'-t',f'{duration:.6f}',
                '-vf','scale=1280:720:force_original_aspect_ratio=decrease,pad=1280:720:(ow-iw)/2:(oh-ih)/2,fps=30',
                '-af',f'afade=t=out:st={duration-.35:.6f}:d=0.35',
                '-c:v','libx264','-crf','19','-preset','fast','-pix_fmt','yuv420p',
                '-c:a','aac','-b:a','192k','-movflags','+faststart',args.output],check=True)
print(f'Encoded {len(rows)} real frames, {duration:.2f}s with game audio; peak {np.max(np.abs(samples)):.4f}')
