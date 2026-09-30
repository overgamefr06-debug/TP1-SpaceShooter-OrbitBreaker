# Laser à impulsion — 30 septembre 2026

Trois prises originales synthétisées par `Tools/generate_laser_audio.py`, sans échantillon externe :

- `S_LaserPulse_01.wav`
- `S_LaserPulse_02.wav`
- `S_LaserPulse_03.wav`

Chaque prise dure 205 ms (mono PCM 16 bits, 48 kHz, crête −3,1 dBFS). Attaque courte, corps grave, descente de fréquence amortie et traîne très discrète. Le son se termine avant le tir suivant à la cadence normale de 220 ms. Aucun écrêtage numérique ; les extrémités sont adoucies pour éviter les clics.

`BP_MuzzleFlash.SoundVariants` contient ces trois sons. Un seul est choisi aléatoirement par salve, y compris lors du tir triple ; une faible variation de hauteur de ±2 % évite une répétition mécanique. Les anciens sons restent dans les sources historiques, mais le laser actuel ne les utilise plus.
