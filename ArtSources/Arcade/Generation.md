# Arcade — assets originaux

Créés le 30 septembre 2026 avec l’outil intégré imagegen. Les PNG RGBA sont copiés sans retouche, avec leur alpha, puis importés dans Unreal. Pas de pack externe ni d’image copiée d’un jeu de référence.

Préfixe commun aux six requêtes : Production sprite asset for an original space shooter. Actual transparent background, single centered object occupying 75% of canvas. Avoid ornate chrome UI.

## T_RockA.png

One isolated irregular rounded asteroid of dark charcoal basalt with warm brown dust, large shallow craters, layered chips and pitted mineral surface. No crystals, no gems, no glowing cracks. Top down orthographic, softly lit from upper left. Broad chunky asymmetric silhouette, painterly 3D arcade game sprite, readable at 80px. Actual transparent background.

## T_RockB.png

One isolated lopsided rugged meteor of weathered warm grey ironstone, several dark impact craters, fractures and ochre dust. No crystals, no gems, no glowing cracks. Top down orthographic, softly lit from upper left. Asymmetric compact silhouette, painterly 3D arcade game sprite, readable at 80px. Actual transparent background.

## T_DoubleScore.png

One isolated arcade power-up token. Bold warm gold coinlike rounded square with a simple large engraved white 'x2' symbol. Hand-painted 2D game sprite, restrained bevel, thick dark outline, subtle warm glint, readable at 40px. Front view. Actual transparent background.

## T_Shield.png

One isolated arcade power-up token. Bold ice blue rounded square token with a simple white shield emblem. Hand-painted 2D game sprite, restrained bevel, thick dark outline, subtle cool glint, readable at 40px. Front view. Actual transparent background.

## T_Repair.png

One isolated arcade power-up token. Bold mint green rounded square capsule token with a large simple white plus symbol. Hand-painted 2D game sprite, restrained bevel, thick dark outline, subtle glint, readable at 40px. Front view. Actual transparent background.

## T_TripleShot.png

One isolated arcade power-up token. Bold burnt orange rounded square token with three simple white parallel upward arrows, left arrow angled slightly left and right slightly right. Hand-painted 2D game sprite, restrained bevel, thick dark outline, subtle glint, readable at 40px. Front view. Actual transparent background.

## Sons et effets

S_RockCollision.wav : synthèse originale de bruit filtré, choc grave et craquements, 0,95 s. S_Pickup.wav : trois harmoniques avec enveloppe courte, 0,38 s. Générateur reproductible : Tools/generate_arcade_audio.py. Effet de collision : anneau maillé et seize fragments animés, BP_RockCollision.

## Références de lisibilité

Interfaces observées : [Super Stardust HD, Housemarque](https://housemarque.com/games/sshd) et [Nova Drift, page officielle Steam](https://store.steampowered.com/app/858210/Nova_Drift/). Retenir la place donnée au jeu et la sobriété des informations ; aucun asset de ces jeux n’est réutilisé.

