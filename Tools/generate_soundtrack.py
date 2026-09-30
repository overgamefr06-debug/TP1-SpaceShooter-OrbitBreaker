"""Original, deterministic stereo synth score. No samples or external recordings.

All notes, echoes and room tails wrap around the bar grid, so Unreal can loop
the full WAV without cutting a release. Run from any directory; numpy required.
"""
from pathlib import Path
import json
import wave
import numpy as np

RATE = 48000
OUT = Path(__file__).resolve().parents[1] / 'ArtSources' / 'Soundtrack'
OUT.mkdir(parents=True, exist_ok=True)
rng = np.random.default_rng(20641)
stats = {}

def hz(note):
    return 440 * 2 ** ((note - 69) / 12)

def write(name, audio, target=.72):
    peak = float(np.max(np.abs(audio)))
    audio = audio * (target / max(peak, 1e-8))
    pcm = np.round(audio * 32767).astype('<i2')
    with wave.open(str(OUT / (name + '.wav')), 'wb') as wav:
        wav.setparams((2, 2, RATE, len(pcm), 'NONE', 'not compressed'))
        wav.writeframes(pcm.tobytes())
    stats[name] = {'seconds': round(len(pcm)/RATE, 3),
                   'peak_dbfs': round(20*np.log10(target), 2),
                   'rms_dbfs': round(20*np.log10(np.sqrt(np.mean(audio**2))), 2),
                   'seam_step': float(np.max(np.abs(audio[-1]-audio[0])))}

def tone(note, duration, kind='pluck'):
    t = np.arange(round(duration*RATE)) / RATE
    f = hz(note)
    if kind == 'pad':
        signal = sum(np.sin(2*np.pi*f*(1+d)*t + d*19) for d in [-.002,0,.002])/3
        signal += .16*np.sin(2*np.pi*f*2*t)
        env = np.minimum(t/.65, 1)*np.minimum((duration-t)/1.2, 1)
    elif kind == 'bass':
        signal = np.sin(2*np.pi*f*t)+.24*np.sin(2*np.pi*2*f*t)+.08*np.sin(2*np.pi*3*f*t)
        env = (1-np.exp(-t*180))*np.exp(-t*4)*np.minimum((duration-t)/.045, 1)
    else:
        signal = np.sin(2*np.pi*f*t + .45*np.sin(2*np.pi*2*f*t)*np.exp(-t*9))
        signal += .12*np.sin(2*np.pi*f*3*t)*np.exp(-t*14)
        env = (1-np.exp(-t*350))*np.exp(-t*(3.4 if kind=='bell' else 7))*np.minimum((duration-t)/.04, 1)
    return signal*env

def drum(kind):
    duration={'kick':.4,'snare':.24,'hat':.065}[kind]
    t=np.arange(round(duration*RATE))/RATE
    noise=rng.normal(0,1,len(t))
    if kind=='kick':
        phase=2*np.pi*(46*t+60*.026*(1-np.exp(-t/.026)))
        return .85*np.sin(phase)*(1-np.exp(-t*1000))*np.exp(-t*13)
    high=noise-np.convolve(noise,np.ones(13)/13,mode='same')
    if kind=='snare':
        high=np.convolve(high,np.ones(4)/4,mode='same')
        return (.45*high+.2*np.sin(2*np.pi*175*t)*np.exp(-t*20))*(1-np.exp(-t*1400))*np.exp(-t*23)
    return high*.12*(1-np.exp(-t*1800))*np.exp(-t*75)

def compose(name, bpm, bars, action):
    beat=60/bpm
    n=round(bars*4*beat*RATE)
    audio=np.zeros((n,2),dtype=np.float64)
    def put(signal, seconds, level=1., pan=0.):
        start=round(seconds*RATE)%n
        stereo=signal[:,None]*level*np.array([np.sqrt((1-pan)/2),np.sqrt((1+pan)/2)])
        end=start+len(stereo)
        if end<=n: audio[start:end]+=stereo
        else:
            audio[start:]+=stereo[:n-start]
            audio[:end-n]+=stereo[n-start:]
    # D minor / Bb major / F major / C suspended: one original eight-bar phrase.
    chords=[[50,57,60,64,69],[46,53,57,60,65],[41,53,57,60,67],[48,55,60,62,67]]
    melody=[[74,77,76,69],[72,77,74,69],[69,72,76,79],[74,72,67,69]]
    for bar in range(bars):
        chord=chords[(bar//2)%4]
        section=bar//8
        for j,note in enumerate(chord):
            if bar%2==0:
                put(tone(note+12,beat*9.5,'pad'),bar*4*beat,.055 if action else .13,(j-2)*.32)
        if action:
            for b in [0,1,2,3]: put(drum('kick'),(bar*4+b)*beat,.55)
            for b in [1,3]: put(drum('snare'),(bar*4+b)*beat,.65)
            for h in range(8): put(drum('hat'),(bar*4+h*.5)*beat,.45 if h%2==0 else .75,(-1 if h%2 else 1)*.35)
            for b in [0,.75,1.5,2,2.75,3.5]:
                put(tone(chord[0]-12+(12 if b==2.75 else 0),.42,'bass'),(bar*4+b)*beat,.31)
        else:
            put(tone(chord[0]-12,beat*2.8,'bass'),bar*4*beat,.22)
            if section%2:
                for h in [1.5,3.5]: put(drum('hat'),(bar*4+h)*beat,.17,.3)
        # Arpeggio with breathing room, mellow FM plucks and stereo beat echoes.
        steps=range(8) if action and section!=2 else [0,3,5]
        for s in steps:
            note=chord[[1,2,3,4,2,3,1,3][s]]+12
            signal=tone(note,.72)
            at=(bar*4+s*.5)*beat
            level=.075 if action else .09
            put(signal,at,level,(-1 if s%2 else 1)*.35)
            put(signal,at+beat*.75,level*.27,.6)
            put(signal,at+beat*1.5,level*.13,-.6)
        if bar%2==1 and (not action or section in [1,3]):
            for k,note in enumerate(melody[(bar//2)%4]):
                signal=tone(note,1.6,'bell'); at=(bar*4+k*.75)*beat
                put(signal,at,.105 if action else .12,0)
                put(signal,at+beat*.75,.035,-.45)
                put(signal,at+beat*1.5,.017,.45)
    # Short diffuse room, folded through the loop rather than cut off at EOF.
    dry=audio.copy()
    for delay,level in [(.071,.065),(.113,.045),(.173,.028),(.239,.018)]:
        audio+=np.roll(dry[:,::-1],round(delay*RATE),axis=0)*level
    audio-=audio.mean(axis=0)
    write(name,np.tanh(audio*1.2),.72)

compose('M_QuietOrbit',84,16,False)
compose('M_BreakerRun',120,32,True)
for name,notes,duration in [('UI_Hover',[81],.065),('UI_Select',[74,81],.13),
                             ('UI_Confirm',[69,76,81],.24),('UI_Back',[76,69],.16)]:
    n=round(duration*RATE); mono=np.zeros(n)
    for i,note in enumerate(notes):
        offset=round(i*.032*RATE)
        signal=tone(note,(n-offset)/RATE)
        mono[offset:]+=signal*.3
    # A hard guarantee of silent endpoints on non-looping interface cues.
    edge=min(240,n//4); mono[:edge]*=np.linspace(0,1,edge); mono[-edge:]*=np.linspace(1,0,edge)
    write(name,np.column_stack([mono,mono]),.45 if name=='UI_Hover' else .6)
(OUT/'Analysis.json').write_text(json.dumps(stats,indent=2)+'\n',encoding='utf-8')
print(json.dumps(stats,indent=2))
