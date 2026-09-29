"""Original mathematical nebula texture and synthesized SFX. Requires numpy and Pillow.
The generated sources are original to this project; no third-party media is used.
"""
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw, ImageFilter
import wave

out=Path(__file__).resolve().parents[1]/'ArtSources'
out.mkdir(exist_ok=True)
rng=np.random.default_rng(20641)
w,h=2048,1152
y,x=np.mgrid[0:h,0:w].astype(np.float32); x=x/w; y=y/h
noise=np.zeros((h,w),dtype=np.float32)
for size,weight in [(8,.5),(20,.24),(48,.14),(130,.08),(310,.04)]:
    small=Image.fromarray(np.uint8(rng.random((size,size))*255))
    noise+=np.asarray(small.resize((w,h),Image.Resampling.BICUBIC),dtype=np.float32)/255*weight
cloud=np.clip((noise-.28)*2.4,0,1)**2
band=np.exp(-((y-(.80-.48*x+.07*np.sin(x*12)))/.21)**2)
edge=np.clip((abs(x-.5)+abs(y-.5))*.95,.15,1)
rgb=np.zeros((h,w,3),dtype=np.float32)+np.array([2.5,6,16])
rgb+=cloud[:,:,None]*band[:,:,None]*edge[:,:,None]*np.array([31,29,68])
cyan=np.exp(-((x-.83)**2/.1+(y-.29)**2/.06))
rgb+=cloud[:,:,None]*cyan[:,:,None]*np.array([3,27,38])
img=Image.fromarray(np.uint8(np.clip(rgb,0,255)),'RGB')
d=ImageDraw.Draw(img)
for i in range(620):
    xx=int(rng.uniform(0,w)); yy=int(rng.uniform(0,h)); b=int(rng.uniform(45,145))
    r=.5 if i<560 else 1.2
    d.ellipse((xx-r,yy-r,xx+r,yy+r),fill=(b,int(b*.9),min(255,int(b*1.15))))
for i in range(9):
    xx=int(rng.uniform(80,w-80)); yy=int(rng.uniform(80,h-80))
    d.line((xx-4,yy,xx+4,yy),fill=(60,95,140)); d.line((xx,yy-4,xx,yy+4),fill=(60,95,140))
    d.ellipse((xx-1,yy-1,xx+1,yy+1),fill=(170,210,230))
img.save(out/'T_OrbitNebula.png')

def sound(name,duration,kind):
    rate=22050; t=np.arange(int(rate*duration))/rate
    if kind=='shot':
        phase=2*np.pi*(1150*t-3000*t*t)
        signal=(np.sin(phase)*.6+np.sin(phase*1.9)*.15)*np.exp(-t*30)
    else:
        n=rng.uniform(-1,1,len(t)); n=np.convolve(n,np.ones(9)/9,mode='same')
        signal=(n*.8+np.sin(2*np.pi*(100*t-65*t*t))*.25)*np.exp(-t*6)
    signal*=np.minimum(t/.006,1)
    with wave.open(str(out/name),'wb') as f:
        f.setnchannels(1); f.setsampwidth(2); f.setframerate(rate)
        f.writeframes((np.clip(signal,-1,1)*28000).astype('<i2').tobytes())
sound('S_Laser.wav',.16,'shot')
sound('S_Impact.wav',.65,'impact')
print('Original atmosphere and sounds created:',out)
