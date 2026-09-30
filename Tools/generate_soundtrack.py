"""Compose the second score: sampled orchestra and dry, non-musical UI clicks.
Original arrangement; VSCO 2 CE samples (CC0), pinned in OrchestraSamples.json.
External source samples stay in Saved/OrchestralSamples, never in the build.
Usage: python Tools/generate_soundtrack.py [--ffmpeg PATH]
"""
from pathlib import Path
import argparse, concurrent.futures, hashlib, json, re, subprocess, urllib.parse, urllib.request, wave
import numpy as np
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'ArtSources/Soundtrack'; CACHE=ROOT/'Saved/OrchestralSamples'
p=argparse.ArgumentParser();p.add_argument('--ffmpeg',default=str(ROOT.parent/'.tools/media/imageio_ffmpeg/binaries/ffmpeg-win-x86_64-v7.1.exe'));args=p.parse_args()
SR=48000; rng=np.random.default_rng(206412); stats={}
manifest=json.loads((OUT/'OrchestraSamples.json').read_text())
def fetch(s):
    dest=CACHE/s['path'];dest.parent.mkdir(parents=True,exist_ok=True)
    if not dest.exists():
        url='https://raw.githubusercontent.com/sgossner/VSCO-2-CE/'+manifest['revision']+'/'+urllib.parse.quote(s['path'])
        urllib.request.urlretrieve(url,dest)
    b=dest.read_bytes();digest=hashlib.sha1(b'blob '+str(len(b)).encode()+b'\0'+b).hexdigest()
    assert digest==s['git_blob_sha'], 'Sample checksum mismatch: '+s['path']
    return s,dest
with concurrent.futures.ThreadPoolExecutor(max_workers=6) as pool: files=list(pool.map(fetch,manifest['samples']))
bank={}
for s,path in files:
    raw=subprocess.check_output([args.ffmpeg,'-v','error','-i',str(path),'-f','f32le','-ar',str(SR),'-ac','1','-'])
    a=np.frombuffer(raw,dtype='<f4').astype(np.float64)
    # Remove recording lead-in; keep the natural bow/breath/drum attack.
    hot=np.flatnonzero(abs(a)>max(abs(a).max()*.035,.00001))
    if len(hot): a=a[max(0,int(hot[0])-int(.012*SR)):]
    a-=a.mean();a*=.7/max(abs(a).max(),1e-8)
    m=re.search(r'_([A-G])(#?)([0-9])_',path.name)
    note=24+int(m[3])*12+{'C':0,'D':2,'E':4,'F':5,'G':7,'A':9,'B':11}[m[1]]+bool(m[2]) if m else 0
    bank.setdefault(s['instrument'],[]).append((note,a))

def sample(inst,note,duration,expression=1.,variant=0):
    candidates=sorted(bank[inst],key=lambda x:abs(x[0]-note))
    nearest=[x for x in candidates if abs(x[0]-note)==abs(candidates[0][0]-note)]
    key,a=nearest[variant%len(nearest)]
    ratio=2**((note-key)/12) if key else 1.
    natural=inst in ['spic','cello_short','bassdrum','snare','timp','cymbal']
    length=min(len(a)/ratio/SR, duration+(.9 if natural else .35))
    pos=np.arange(round(length*SR))*ratio
    audio=np.interp(pos,np.arange(len(a)),a)
    t=np.arange(len(audio))/SR
    if not natural:
        attack=.28 if inst in ['strings','cello'] else .065
        env=np.minimum(t/attack,1)*np.clip((length-t)/.35,0,1)
        env*=.8+.2*np.sin(np.pi*np.minimum(t/max(duration,.1),1))
    else:
        env=np.ones(len(t)); fade=min(.15,length*.3);env*=np.clip((length-t)/fade,0,1)
        if inst in ['spic','cello_short']:env*=np.exp(-np.maximum(0,t-duration)*9)
    return audio*env*expression

def write(name,a,peak=.75):
    a-=a.mean(axis=0);a*=peak/max(np.max(abs(a)),1e-8)
    pcm=np.round(a*32767).astype('<i2')
    with wave.open(str(OUT/(name+'.wav')),'wb') as w:
        w.setparams((2,2,SR,len(pcm),'NONE','not compressed'));w.writeframes(pcm.tobytes())
    stats[name]={'seconds':round(len(a)/SR,3),'peak_dbfs':round(20*np.log10(peak),2),
                 'rms_dbfs':round(20*np.log10(np.sqrt(np.mean(a*a))),2),'seam_step':float(abs(a[-1]-a[0]).max())}

def compose(name,bpm,bars,battle):
    beat=60/bpm;n=round(bars*4*beat*SR);mix=np.zeros((n,2));room=np.zeros((n,2))
    def put(inst,note,at,dur,level,pan=0.,variant=0):
        a=sample(inst,note,dur,1.,variant)*level
        start=round((at+float(rng.uniform(-.008,.008)))*SR)%n
        stereo=a[:,None]*np.array([np.sqrt((1-pan)/2),np.sqrt((1+pan)/2)])
        end=start+len(a)
        if end<=n:mix[start:end]+=stereo
        else:mix[start:]+=stereo[:n-start];mix[:end-n]+=stereo[n-start:]
    # E minor / C major / G major / D suspended; eight-bar theme and response.
    roots=[40,36,43,38];chords=[[64,67,71],[60,64,67],[62,67,71],[62,66,69]]
    theme=[[(71,0,1.5),(67,1.75,.75),(69,3,1)],[(72,0,2.5),(71,3,1)],
           [(74,0,1.5),(71,2,1.5)],[(69,0,2),(66,2.5,1.5)]]
    for bar in range(bars):
        section=bar//8;ci=(bar//2)%4;root=roots[ci];chord=chords[ci];at=bar*4*beat
        strength=[.76,1.,.82,1.07][section%4] if battle else [.65,.88][section%2]
        if bar%2==0:
            put('cello',root,at,7.8*beat,.17*strength,.22)
            put('cello',root+12,at+.06,7.6*beat,.09*strength,.3)
            for j,note in enumerate(chord):put('strings',note,at+j*.022,7.8*beat,.13*strength,(j-1)*.34-.1)
        if battle:
            # Bowed ostinato, with alternate recorded strokes and breathing accents.
            pattern=[root+12,root+19,root+12,root+15,root+12,root+19,root+24,root+19]
            for k,note in enumerate(pattern):
                put('cello_short',note,at+k*.5*beat,.24*beat,.18*strength*(1 if k%2==0 else .7),.22,k+bar)
            if section!=2 or bar%2:
                for k in range(8):put('spic',chord[[0,2,1,2,0,1,2,1][k]],at+k*.5*beat,.25*beat,.11*strength*(1 if k%2==0 else .72),-.35,k+bar)
            for k in [0,1.5,2.75]:put('bassdrum',0,at+k*beat,.7,.5*strength,0,bar)
            for k in [1,3]:put('snare',0,at+k*beat,.4,.18*strength,-.1,bar)
            if bar%4==3:
                for k in [3.25,3.5,3.75]:put('snare',0,at+k*beat,.22,.08+.035*(k-3.25)*4,.05,bar+int(k*4))
            if bar%2==0:put('timp',0,at,1.4,.33*strength,.18,bar)
            if bar%8==0:put('cymbal',0,at,5.,.13, .25)
        else:
            if bar%4==0:put('bassdrum',0,at,2.5,.16,0,bar)
            if bar%4==3:put('timp',0,at+3*beat,2.,.12,.22,bar)
            if section and bar%4==0:put('cymbal',0,at,4.,.055,.2)
        # Broad brass melody, answered by strings; no bright arpeggiated beeps.
        if (bar%2==0 and (not battle or section!=2)):
            for note,offset,hold in theme[ci]:
                put('horn',note-12,at+offset*beat,hold*beat,.19*strength,.07)
                if battle and section in [1,3]:put('trumpet',note,at+offset*beat+.027,hold*beat,.10*strength,-.03)
        if bar%2 and (not battle or section==2):
            answer=[chord[2],chord[1],chord[0]]
            for j,note in enumerate(answer):put('strings',note+12,at+j*1.25*beat,1.4*beat,.12*strength,-.4)
    # Diffuse concert-hall tail. Circular convolution carries the tail into the loop.
    dry=mix.copy();irlen=int(SR*2.4)
    for ch in range(2):
        ir=np.zeros(n);ix=rng.integers(int(.05*SR),irlen,4500)
        values=rng.normal(0,1,len(ix))*np.exp(-ix/(SR*.6))
        np.add.at(ir,ix,values);ir/=max(np.sqrt(np.sum(ir*ir)),1e-8)
        room[:,ch]=np.fft.irfft(np.fft.rfft(dry[:,ch]) * np.fft.rfft(ir),n)
    mix=dry*.86+room*.22
    # Gentle peak rounding, leaving the percussion's dynamics intact.
    write(name,np.tanh(mix*.9),.75)

compose('M_QuietOrbit',76,16,False)
compose('M_BreakerRun',128,32,True)
# Non-musical interface Foley: filtered pressure click, low body and short air tail.
for name,duration,cut,body in [('UI_Hover',.04,1550,.07),('UI_Select',.065,2000,.14),
                               ('UI_Confirm',.11,1750,.22),('UI_Back',.09,1150,.13)]:
    n=round(duration*SR);t=np.arange(n)/SR
    noise=rng.normal(0,1,n);f=np.fft.rfftfreq(n,1/SR)
    filt=np.exp(-(f/cut)**4)*(1-np.exp(-(f/220)**2))
    click=np.fft.irfft(np.fft.rfft(noise)*filt,n)
    click*=np.exp(-t/(.009 if name=='UI_Hover' else .016))
    a=click*.65+body*np.sin(2*np.pi*(135*t+10*.015*(1-np.exp(-t/.015))))*np.exp(-t*65)
    a*=np.minimum(t/.0015,1)*np.clip((duration-t)/.015,0,1)
    write(name,np.column_stack([a,a]),.18 if name=='UI_Hover' else .3)
(OUT/'Analysis.json').write_text(json.dumps(stats,indent=2)+'\n',encoding='utf-8')
print(json.dumps(stats,indent=2))
