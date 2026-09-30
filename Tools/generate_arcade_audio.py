"""Synthesize original rock collision and pickup sounds. No sampled recordings."""
from pathlib import Path
import wave
import numpy as np

out = Path(__file__).resolve().parents[1] / 'ArtSources' / 'Arcade'
out.mkdir(parents=True, exist_ok=True)
rate = 44100
rng = np.random.default_rng(20641)

def save(name, samples):
    samples = .82 * samples / max(np.max(np.abs(samples)), .001)
    with wave.open(str(out / (name + '.wav')), 'wb') as f:
        f.setnchannels(1); f.setsampwidth(2); f.setframerate(rate)
        f.writeframes((samples * 32767).astype('<i2').tobytes())

t = np.arange(int(rate * .95)) / rate
noise = rng.normal(0, 1, len(t))
rumble = np.convolve(noise, np.ones(65) / 65, mode='same') * 4
impact = np.sin(2*np.pi*(92*t-30*t*t)) * np.exp(-t*13)
rock = (rumble * .7 + noise * .16) * np.exp(-t*6) + impact * .55
for start in [.012, .052, .108, .19, .29]:
    u = np.maximum(0, t-start)
    rock += noise * .32 * np.exp(-u*65) * (t>=start)
rock *= np.minimum(1, t/.0015) * np.minimum(1, (.95-t)/.035)
save('S_RockCollision', rock)
t = np.arange(int(rate * .38)) / rate
tone = sum(np.sin(2*np.pi*f*t) * np.exp(-t*12) for f in [660, 990, 1320])
tone *= np.minimum(1,t/.005) * np.minimum(1,(.38-t)/.03)
save('S_Pickup', tone)
print('Original collision (0.95s) and pickup (0.38s) WAVs generated.')
