# Blueprints et réglages d’Orbit Breaker

Le projet utilise des **Blueprints enfants de classes C++**. La logique est implémentée en C++, conformément à l’approche du TP ; les Blueprints assemblent les composants, choisissent les assets et règlent le jeu. Les Event Graphs sont donc généralement vides : cela ne signifie pas que le jeu n’utilise pas les Blueprints. L’interface actuelle utilise Canvas en C++, pas un Widget Blueprint UMG.

Dans Unreal, ouvrir le Content Browser, puis **Content → Blueprints**. Double-cliquer sur un Blueprint, puis **Class Defaults**. Si Unreal affiche seulement une fiche de données, choisir **Open Full Blueprint Editor**. Faire **Check Out** avant de modifier, puis **Compile → Save**.

| Blueprint | Réglages à consulter |
|---|---|
| `BP_Ship` | Move Speed, Fire Interval, Projectile Class, Ship Materials, Shield Mesh et Shield Material. Composants Hull, Collision, Engine Glow, Shield Glow. Vaisseau réduit à 68 % ; collision adaptée. |
| `BP_Projectile` | Speed et Lifetime. Un projectile applique au plus un impact. |
| `BP_Asteroid` | Size Scales = 0,55 / 0,95 / 1,45 ; Hits by Size = 1 / 2 / 3 ; Points by Size = 100 / 200 / 400 ; Rock Materials ; Fragment Grace Seconds = 0,8. |
| `BP_SpaceGameMode` | Asteroid Class, Pickup Classes, Collision Effect Class ; délais et vitesse d’apparition ; Double Score Duration = 15 s, Shield Duration = 10 s, Triple Shot Duration = 15 s ; Repair Probability = 0,05 ; Unlock Scores = 0 / 5 000 / 15 000. |
| `BP_BonusDoubleScore` | Type DoubleScore, sprite doré, durée de présence au sol 12 s, son de collecte. |
| `BP_BonusShield` | Type Shield, sprite bleu, présence 12 s. |
| `BP_BonusRepair` | Type Repair, sprite vert, présence 12 s. Restaure une vie, maximum trois. |
| `BP_BonusTripleShot` | Type TripleShot, sprite orange, présence 12 s. |
| `BP_RockCollision` | Son original, gerbe de poussière et d’étincelles additive, durée 0,65 s, rayon d’expansion 95, Fragment Count = 0. Utilisé pour collision et destruction. |
| `BP_OrbitHUD` | Title Logo, Ship Portraits, Bonus Icons, auteur. Affiche les informations du GameMode et les HP actuels des astéroïdes. |
| `BP_CollectDoubleScore / Shield / Repair / TripleShot` | Onde lumineuse additive à la collecte, couleur propre à chaque bonus, durée 0,55 s. |
| `BP_MuzzleFlash` | Éclat additif transparent, durée 0,10 s ; Fragment Count = 0. Sound Variants contient trois nouveaux sons de laser à impulsion. Un seul son par salve, même en tir triple. |

## Suivre un mécanisme dans le code

- Tir : `ShipPawn::TryFire` crée un projectile, ou trois aux angles −12°, 0°, +12° lorsque TripleShot est actif. `ShotProjectile::OnOverlap` appelle `SpaceAsteroid::ReceiveShot`.
- Fragmentation : `SpaceAsteroid::OnRockOverlap` appelle `SpaceGameMode::SplitAsteroidPair`. Les deux parents sont consommés avant la création des trois enfants pour empêcher une double exécution des notifications de collision. Les petits et les tailles différentes ne se fragmentent pas. Les collisions ne donnent pas de points.
- Collecte : `SpacePickup::OnContact` reconnaît le vaisseau, désactive la collision, appelle `SpaceGameMode::ActivateBonus`, joue le son, puis détruit le bonus.
- Durées : les bonus utilisent une heure de fin indépendante. Reprendre le même bonus renouvelle sa durée ; plusieurs types peuvent fonctionner ensemble. Une nouvelle partie les efface.
- Progression : `AwardAsteroid` met à jour `BestScore = max(BestScore, Score)`. `IsShipUnlocked` utilise ce record, jamais un cumul de parties. Le record est enregistré lors du déblocage, de la fin de partie, du retour au menu et de la fermeture normale.

Les fonctions de jeu marquées `BlueprintCallable` et les valeurs `BlueprintReadOnly` permettent de construire ensuite des graphes ou des widgets sans réécrire les règles. Il n’y a pas actuellement de graphe de gameplay visuel à prétendre présenter au professeur.

## Sources et reconstruction

Art source : `ArtSources/Arcade` et `ArtSources/Effects`, sons originaux : `Tools/generate_arcade_audio.py` et `Tools/generate_laser_audio.py`. Exécuter les anciens imports de contenu puis de flotte, ensuite `Tools/import_arcade_content.py`, `Tools/import_effects_content.py` et `Tools/import_combat_polish.py` puis **en dernier `Tools/import_soundtrack.py`**. Faire Check Out sur les assets existants avant de relancer un import.

Chaque `BP_Bonus…` expose Aura Mesh, Aura Material et Collect Effect Class. Le matériau anime trois petites particules autour du symbole ; le composant Visual flotte doucement. Le bouclier de `BP_Ship` utilise `M_EnergyShield` : bord électrique, cellules hexagonales discrètes et centre transparent. `Strength` pilote son apparition et sa disparition ; `Impact` produit une impulsion lors du contact avec un astéroïde. Ces paramètres changent l’apparence, pas la durée de protection de dix secondes.

Les validations automatiques se lancent avec `-OrbitTestMode` pour protéger la vraie sauvegarde. Le test `SpaceShooter.Gameplay.ArcadeSystems` utilise un slot temporaire distinct pour vérifier l’enregistrement sur disque et le supprime ensuite.

Le test `SpaceShooter.Visual.Effects` compare les pixels de chaque vaisseau au repos et pendant une série de tirs. Il échoue si plus de 5 % de ses pixels lumineux deviennent noirs et conserve les captures dans `Saved/EffectsValidation`.

Le test `SpaceShooter.Visual.AsteroidEffects` détruit un astéroïde près d’un autre, puis vérifie aussi l’effet de collision avec le vaisseau. Il exige que 99 % des pixels lumineux du rocher survivant restent visibles. Les astéroïdes utilisent désormais l’alpha complet de leur texture ; projectiles, poussière et étincelles sont additifs, sans fragment opaque. Le logo flotte de ±4 pixels, s’incline de moins d’un demi-degré et varie de moins de 1 % en taille ; ces animations sont dans `SpaceHUD::DrawHUD`.

## Musiques et sons des boutons

Dans `BP_SpaceGameMode`, catégorie **Audio**, les références Menu Music / Game Music choisissent les deux musiques. Interface Sounds contient, dans cet ordre, survol, sélection, validation et retour. Menu Music Volume = 0,55 ; Game Music Volume = 0,38 ; Interface Volume = 0,50. Les sources WAV sont dans ArtSources/Soundtrack ; générateur original `Tools/generate_soundtrack.py`, import `Tools/import_soundtrack.py`.

`SpaceGameMode::UpdateMusic` assure les fondus, avec deux lecteurs réutilisés. Les événements de jeu et de clavier appellent les confirmations ; `SpaceHUD::NotifyHitBoxBeginCursorOver` joue un survol à l’entrée du curseur, pas à chaque dessin du HUD. Le bouton Quitter attend 180 ms pour laisser entendre son son. `SpaceShooter.Audio.Transitions` vérifie les pistes bouclées, les boutons non bouclés et les changements d’état rapides.
