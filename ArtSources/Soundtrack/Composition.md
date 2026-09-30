# Bande-son orchestrale — deuxième version

Cette version remplace entièrement les deux anciennes compositions électroniques et leurs confirmations mélodiques. Direction demandée : **cinématique, orchestral et percussions épiques**.

| Fichier | Usage | Composition |
|---|---|---|
| M_QuietOrbit.wav | Menu et bilan | 76 BPM, 16 mesures, 50,526 s. Cordes tenues, violoncelles graves, thème de cors, réponses de violons et percussions espacées. |
| M_BreakerRun.wav | Partie | 128 BPM, 32 mesures, 60 s. Ostinatos de cordes, cors et trompettes, grosse caisse orchestrale, caisse claire, timbales et cymbales ; quatre sections avec variations et passage plus retenu. |
| UI_Hover.wav | Survol | Clic mat et discret, 40 ms. |
| UI_Select.wav | Choix du vaisseau | Clic tactile court, 65 ms. |
| UI_Confirm.wav | Jouer / rejouer | Pression plus grave, 110 ms. |
| UI_Back.wav | Retour / Quitter | Déclic assourdi, 90 ms. |

Les sons d’interface sont synthétisés à partir de bruit filtré et d’une courte résonance grave ; ils ne jouent plus de petites mélodies. Les deux arrangements musicaux sont originaux, en mi mineur, avec réverbération de salle et variations d’articulation. Ils utilisent de vrais échantillons instrumentaux **VSCO 2 Community Edition**, de Versilian Studios / Sam Gossner, sous **CC0-1.0** :

- Projet officiel : https://versilian-studios.com/vsco-community/
- Sources : https://github.com/sgossner/VSCO-2-CE
- Licence complète : VSCO-CC0-LICENSE.txt
- Les 33 échantillons employés, leur révision et leur empreinte Git sont listés dans OrchestraSamples.json.

`Tools/generate_soundtrack.py` télécharge uniquement ces échantillons dans Saved/OrchestralSamples, vérifie leurs empreintes, puis les joue selon la partition programmée. Le cache de samples n’est pas publié et ne fait pas partie du build. Seuls les morceaux rendus sont importés dans Unreal. Pré-requis pour reproduire le rendu : Python avec numpy, FFmpeg (`--ffmpeg CHEMIN`) et réseau lors du premier lancement. Aucun logiciel d’instrument supplémentaire n’est nécessaire pour jouer au jeu.

Rendu : stéréo PCM 16 bits, 48 kHz. Crête musicale −2,50 dBFS ; aucun écrêtage. Les queues de salle et notes sont reportées au début de la boucle pour conserver la continuité. Mesures : Analysis.json. Les noms techniques des SoundWave sont conservés pour maintenir les références Blueprint, mais leur contenu audio est entièrement remplacé.

Import : Tools/import_soundtrack.py, après Check Out des fichiers existants. Les deux musiques bouclent ; les quatre sons de boutons ne bouclent pas. Volumes de BP_SpaceGameMode : menu 0,55 ; partie 0,38 ; interface 0,50, survol atténué de moitié. Les fondus existants et les commandes souris/clavier sont conservés.
