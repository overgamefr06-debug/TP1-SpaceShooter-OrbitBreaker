# Arcade — assets originaux

Créés le 30 septembre 2026 avec l’outil intégré imagegen. Les PNG RGBA sont copiés sans retouche, avec leur alpha, puis importés dans Unreal. Pas de pack externe ni d’image copiée d’un jeu de référence.

Préfixe commun aux deux requêtes : Production sprite asset for an original space shooter. Actual transparent background, single centered object occupying 75% of canvas. Avoid ornate chrome UI.

## T_RockA.png

One isolated irregular rounded asteroid of dark charcoal basalt with warm brown dust, large shallow craters, layered chips and pitted mineral surface. No crystals, no gems, no glowing cracks. Top down orthographic, softly lit from upper left. Broad chunky asymmetric silhouette, painterly 3D arcade game sprite, readable at 80px. Actual transparent background.

## T_RockB.png

One isolated lopsided rugged meteor of weathered warm grey ironstone, several dark impact craters, fractures and ochre dust. No crystals, no gems, no glowing cracks. Top down orthographic, softly lit from upper left. Asymmetric compact silhouette, painterly 3D arcade game sprite, readable at 80px. Actual transparent background.

## Sons et effets

S_RockCollision.wav : synthèse originale de bruit filtré, choc grave et craquements, 0,95 s. S_Pickup.wav : trois harmoniques avec enveloppe courte, 0,38 s. Générateur reproductible : Tools/generate_arcade_audio.py. Effet de collision : gerbe additive de poussière et d’étincelles, BP_RockCollision.

## Références de lisibilité

Interfaces observées : [Super Stardust HD, Housemarque](https://housemarque.com/games/sshd) et [Nova Drift, page officielle Steam](https://store.steampowered.com/app/858210/Nova_Drift/). Retenir la place donnée au jeu et la sobriété des informations ; aucun asset de ces jeux n’est réutilisé.


Les quatre anciennes tuiles de bonus ont été supprimées du projet actif. Les symboles actuels et leurs prompts sont dans ArtSources/Effects. Les anciennes versions restent dans les historiques.
