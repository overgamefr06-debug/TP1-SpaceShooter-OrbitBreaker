# Musiques et interface — créations originales

Deux compositions instrumentales synthétisées pour Orbit Breaker. Aucun enregistrement, sample, morceau tiers ou voix n'est utilisé. La génération est déterministe et reproductible avec `Tools/generate_soundtrack.py` (Python et numpy).

| Fichier | Usage | Composition |
|---|---|---|
| M_QuietOrbit.wav | Menu et bilan | 84 BPM, 16 mesures, 45,714 s : nappes douces, motif cristallin espacé, basse ronde et échos stéréo. |
| M_BreakerRun.wav | Partie | 120 BPM, 32 mesures, 64 s : batterie électronique, basse syncopée, arpège et variations sur quatre sections. |
| UI_Hover.wav | Entrée du curseur sur un bouton ou vaisseau disponible | Impulsion douce, 65 ms. |
| UI_Select.wav | Choix de vaisseau, souris ou clavier | Deux notes, 130 ms. |
| UI_Confirm.wav | Jouer / rejouer, souris ou clavier | Trois notes ascendantes, 240 ms. |
| UI_Back.wav | Retour au menu et Quitter | Deux notes descendantes, 160 ms. |

La progression harmonique originale utilise ré mineur, si bémol majeur, fa majeur et do suspendu. Tous les instruments sont construits à partir d'oscillateurs et de bruit filtré ; les notes, queues de réverbération et échos sont reportés au début de la boucle lorsqu'ils dépassent sa fin. Le rendu source est stéréo, PCM 16 bits, 48 kHz. Crête des musiques : −2,85 dBFS, sans écrêtage. Les mesures sont dans `Analysis.json`.

Les deux SoundWave de musique bouclent dans Unreal. Les quatre sons d'interface ne bouclent pas. `BP_SpaceGameMode` expose les références et les volumes (menu 0,55 ; partie 0,38 ; interface 0,50, survol atténué de moitié). Les transitions durent environ une seconde. Rejouer pendant une partie ne redémarre pas la musique ; la fin de partie retrouve l'ambiance du menu. Le survol est déclenché une fois à l'entrée, avec une limite de 80 ms entre événements, pas à chaque image.

Importer après les autres scripts avec `Tools/import_soundtrack.py`, en faisant Check Out sur les assets déjà suivis. La musique reste sous les effets de jeu. Le bouton Quitter laisse 180 ms au son de retour avant de fermer l'application.
