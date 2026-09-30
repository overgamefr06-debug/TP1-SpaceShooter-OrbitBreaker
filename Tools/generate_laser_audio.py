"""Original dry pulse-laser SFX, 48 kHz PCM. No external samples.
Three related takes avoid identical rapid-fire repetition; no long pitched whistle.
"""
from pathlib import Path
import wave
import numpy as np

out=Path(__file__).resolve().parents[1]/'ArtSources'/'Effects'
out.mkdir(parents=True,exist_ok=True)
rate=48000
duration=.205
t=np.arange(round(rate*duration))/rate
for i in range(3):
    rng=np.random.default_rng(20641+i)
    tuning=[1.,.96,1.045][i]
    # Short metallic energy snap, falling smoothly into a low, soft body.
    frequency=(310+1550*np.exp(-t/0.019))*tuning
    phase=2*np.pi*np.cumsum(frequency)/rate
    carrier=np.sin(phase+.42*np.sin(phase*1.503))*np.exp(-t/0.035)
    body=np.sin(2*np.pi*(105*t+1.4*(1-np.exp(-t/.018))))*np.exp(-t/.031)
    noise=rng.standard_normal(t.size)
    low=np.convolve(noise,np.ones(19)/19,mode='same')
    high=np.convolve(noise,np.ones(5)/5,mode='same')-low
    air=high*np.exp(-t/.010)
    pulse=.48*carrier+.33*body+.18*air
    # A very quiet diffuse tail, kept shorter than the 220 ms firing interval.
    delay=round(.031*rate)
    pulse[delay:]+=.095*carrier[:-delay]*np.exp(-t[delay:]/.07)
    pulse*=np.minimum(t/.0018,1)*np.minimum((duration-t)/.024,1)
    pulse=np.tanh(pulse*1.2)
    pulse*=.70/max(float(np.max(np.abs(pulse))),.001)
    assert np.isfinite(pulse).all() and np.max(np.abs(pulse))<1
    name=out/f'S_LaserPulse_{i+1:02}.wav'
    with wave.open(str(name),'wb') as f:
        f.setnchannels(1); f.setsampwidth(2); f.setframerate(rate)
        f.writeframes((pulse*32767).astype('<i2').tobytes())
    print(name.name, 'duration', duration, 'peak dBFS', round(20*np.log10(np.max(np.abs(pulse))),2))
