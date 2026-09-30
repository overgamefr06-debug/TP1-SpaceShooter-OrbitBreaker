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
| `BP_RockCollision` | Son original, seize débris, anneau d’impact, durée 0,65 s et rayon d’expansion 125. Utilisé pour collision et destruction. |
| `BP_OrbitHUD` | Ship Portraits, Bonus Icons, auteur. Affiche les informations du GameMode et les HP actuels des astéroïdes. |

## Suivre un mécanisme dans le code

- Tir : `ShipPawn::TryFire` crée un projectile, ou trois aux angles −12°, 0°, +12° lorsque TripleShot est actif. `ShotProjectile::OnOverlap` appelle `SpaceAsteroid::ReceiveShot`.
- Fragmentation : `SpaceAsteroid::OnRockOverlap` appelle `SpaceGameMode::SplitAsteroidPair`. Les deux parents sont consommés avant la création des trois enfants pour empêcher une double exécution des notifications de collision. Les petits et les tailles différentes ne se fragmentent pas. Les collisions ne donnent pas de points.
- Collecte : `SpacePickup::OnContact` reconnaît le vaisseau, désactive la collision, appelle `SpaceGameMode::ActivateBonus`, joue le son, puis détruit le bonus.
- Durées : les bonus utilisent une heure de fin indépendante. Reprendre le même bonus renouvelle sa durée ; plusieurs types peuvent fonctionner ensemble. Une nouvelle partie les efface.
- Progression : `AwardAsteroid` met à jour `BestScore = max(BestScore, Score)`. `IsShipUnlocked` utilise ce record, jamais un cumul de parties. Le record est enregistré lors du déblocage, de la fin de partie, du retour au menu et de la fermeture normale.

Les fonctions de jeu marquées `BlueprintCallable` et les valeurs `BlueprintReadOnly` permettent de construire ensuite des graphes ou des widgets sans réécrire les règles. Il n’y a pas actuellement de graphe de gameplay visuel à prétendre présenter au professeur.

## Sources et reconstruction

Art source : `ArtSources/Arcade`, import Unreal : `Tools/import_arcade_content.py`, sons originaux : `Tools/generate_arcade_audio.py`. Le dernier import à exécuter est l’import **arcade**, après les anciens imports de contenu puis de flotte. Les scripts historiques reproduisent les étapes précédentes et ne doivent pas écraser les nouveaux réglages.

Les validations automatiques se lancent avec `-OrbitTestMode` pour protéger la vraie sauvegarde. Le test `SpaceShooter.Gameplay.ArcadeSystems` utilise un slot temporaire distinct pour vérifier l’enregistrement sur disque et le supprime ensuite.
