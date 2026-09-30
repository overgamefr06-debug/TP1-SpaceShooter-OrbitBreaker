# Orbit Breaker — TP1 Space Shooter

Prototype jouable réalisé avec Unreal Engine 5.8.3. Logique de jeu en C++, paramètres et références des assets dans les Blueprints. Version jouable avec exercices de gestion de versions et vidéo de démonstration.

## Ouvrir et jouer

Ouvrir `SpaceShooter.uproject`, charger `Content/Maps/L_Arena` et lancer Play. Dans le menu, cliquer **Lancer la mission** ou appuyer sur **Entrée**.

- Menu : cliquer sur Aegis, Spectre ou Helios, ou utiliser 1 / 2 / 3 (pavé numérique également). Le choix est conservé lors des nouvelles parties. Les trois vaisseaux ont les mêmes performances.
- Flèches, ZQSD ou WASD : déplacement dans les quatre directions.
- Espace ou clic gauche, maintenu : tir avec cadence limitée.
- R : recommencer une partie en cours ou terminée.
- Échap : retour au menu dans le build; dans Play in Editor, le raccourci d'Unreal arrête généralement le test.
- Quitter : bouton du menu principal. Depuis la fin de partie, Retour au menu permet de le retrouver.

Exécutable autonome : `Build/Windows/SpaceShooter.exe`. Le dossier Windows complet est nécessaire; ne pas déplacer uniquement le petit exécutable de lancement.

## Comportement

Les astéroïdes apparaissent sur l'un des quatre bords, à une position et après un délai aléatoires. Une impulsion physique initiale les dirige vers la position du joueur avec une dispersion; ils ne poursuivent pas ensuite le joueur. La catégorie de chaque astéroïde est choisie au hasard une seule fois à sa création. Elle lie la taille, la résistance et les points :

| Catégorie | Échelle | Tirs nécessaires | Points |
|---|---:|---:|---:|
| Petit | 0,55 | 1 | 100 |
| Moyen | 0,95 | 2 | 200 |
| Grand | 1,45 | 3 | 400 |

Les astéroïdes tournent ; leur catégorie ne change pas lorsqu’ils sont touchés.

Seul le tir qui détruit l’astéroïde rapporte les points de sa catégorie, une seule fois. Un contact retire une vie et consomme l'astéroïde. Une protection de 1,5 seconde évite de perdre plusieurs vies immédiatement; le vaisseau clignote pendant cette récupération. Une collision durant cette protection consomme aussi l'astéroïde, sans score. À zéro vie, les apparitions et le score s'arrêtent et le bilan propose de rejouer ou de revenir au menu.

La difficulté augmente progressivement avec la vitesse des astéroïdes (plafond +50 % après 90 secondes). Leur nombre est limité et ceux sortis de la zone sont supprimés. Le redémarrage nettoie astéroïdes, projectiles et effets puis réinitialise score, vies et durée.

## Architecture et réglages

| C++ | Blueprint | Responsabilité et valeurs utiles |
|---|---|---|
| ShipPawn | BP_Ship | Entrées, mouvement, limites, cadence, projectile, effets et trois matériaux de vaisseau |
| ShotProjectile | BP_Projectile | Mouvement balayé, impact unique, vitesse et durée de vie |
| SpaceAsteroid | BP_Asteroid | Trois catégories : taille, résistance et points liés; impulsion physique, rotation et variantes de mesh |
| SpaceGameMode | BP_SpaceGameMode | Menu/partie/fin, score et vies; délais, vitesse, bord et plafond des apparitions |
| CombatBurst | BP_MuzzleFlash / BP_AsteroidBurst | Fragments animés, durée, couleur et son |
| SpaceHUD | BP_OrbitHUD | Interface Canvas, sélection de flotte, portraits, emblème du score, icônes de vies et écrans de fin |

L’interface est dessinée en C++ avec mise à l’échelle : menu de flotte à trois fiches, aperçu animé, panneaux biseautés, emblème du score et icônes du vaisseau choisi pour les vies. Son Blueprint expose les textures et la direction artistique. Elle utilise Canvas, sans Designer UMG.

![Menu de sélection](Files/MenuFlotte.png)
![Interface en partie](Files/JeuFlotte.png)

## Assets originaux

Les trois nouveaux vaisseaux **Aegis**, **Spectre** et **Helios**, ainsi que l’emblème de score, sont des PNG transparents détaillés créés pour le projet avec l’outil intégré imagegen. Les sources et les prompts complets sont dans `ArtSources/Fleet/Generation.md`. Ce sont des sprites prérendus appliqués sur un plan en jeu, et non des modèles 3D volumétriques. Les matériaux, textures et le plan sont dans `Content/Art/Fleet` ; `Tools/import_fleet_art.py` réalise leur import et configure les Blueprints. Les mêmes images servent aux portraits du menu et aux vies.

Les trois meshes d’astéroïdes, la nébuleuse et les sons proviennent des outils procéduraux du projet (`Tools/create_orbit_art.py`, `Tools/generate_atmosphere.py`). L’ancien vaisseau K-07 reste dans les sources du premier jalon mais a été remplacé en jeu. Les polices et primitives de base proviennent d’Unreal. Aucun pack externe n’est requis.

Avant de relancer un outil de génération, faire Check Out sur ses assets existants. Pour reconstruire les assets depuis leurs sources, l’import de flotte doit venir **après** l’ancien script `create_orbit_art.py`, car ce dernier configure l’apparence du premier jalon.

Les plugins Geometry Scripting, EditorToolset et MCP servent uniquement à l'éditeur et sont exclus de la cible du jeu. MCP peut être démarré avec `ModelContextProtocol.StartServer 8000`; adresse locale `http://127.0.0.1:8000/mcp`. Il ne démarre pas automatiquement dans le build Windows.

## Validation — 30 septembre 2026

- Compilation Editor et packaging Shipping réussis.
- Trois tests de gameplay réussis, sans avertissement : `Combat`, `ShipControls`, `RunLoop`. Rapport local : `Artifacts/FleetValidation/index.json`. Le quatrième test, `RecordDemo`, a également réussi et produit la capture de démonstration.
- Combat : touches de tir, cadence, impact unique, tir à bout portant, projectile rapide balayé et expiration. Chaque catégorie est vérifiée : échelle, 1/2/3 impacts, aucun point avant destruction et gain unique de 100/200/400 points.
- Mouvement : dix touches, directions, contrainte du plan et limites.
- Flotte : les trois choix appliquent leur matériau, un index invalide est rejeté, la sélection est bloquée en combat et conservée après redémarrage.
- Partie : écran initial, apparitions temporisées, positions sur les bords, déplacement après impulsion, score, collisions physiques, protection temporaire, dernière vie, arrêt du score, redémarrage et retour au menu.
- Menu et HUD inspectés sur les captures réelles : trois portraits, aperçu, emblème de score, icônes de vies, silhouettes en jeu et trois tailles d’astéroïdes.
- Build Windows Shipping de la refonte : menu, sélection du Spectre à la souris, sélection d’Helios au pavé numérique et lancement par le bouton vérifiés. Les tests complets de mouvement, de résistance et de collisions sont exécutés dans l’éditeur.
- Taille du build complet : **365 154 162 octets**, soit **365,15 Mo** (hors journaux temporaires exclus du dépôt), sous la limite de 500 Mo. Le build jouable est également intégré sur le stream Perforce main. La structure de remise contient le build Windows, la vidéo et les captures des deux historiques.

## Gestion de versions et remise

Le dossier de travail `SpaceShooter` correspond au stream Perforce `//20263_8PRO135_KEVIN_ORTEGA/dev`, workspace `kortega_tp1_dev_DESKTOP_6NT64UN`. Le serveur est `ssl:p4prod.uqac.ca:1666`, utilisateur `kortega`. Avant de modifier un fichier suivi, faire Check Out; les assets utilisent le type exclusif `binary+l`. Les caches, rapports et paramètres de connexion restent exclus. Le dossier `Build/Windows` est versionné dans Perforce et ignoré dans Git.

Les branches Git locales `main` et `dev` existent. Git et Perforce ont des historiques distincts. Ne pas changer de branche Git au milieu de modifications Perforce. Le premier envoi Perforce est 5801 sur main, le peuplement de dev 5803, et le jalon tir/destruction 5847 sur dev.

Les conflits volontaires Git et Perforce sont réalisés et expliqués dans `Docs/Conflits_revision_control.md`, avec les sorties brutes des deux outils. Le commit de fusion Git conserve ses deux parents; Perforce contient les changements 5901 à 5905. Le jeu et le build ont été intégrés sur main dans Perforce (5902), puis le build reconstruit et les preuves de remise dans 5913. Les branches Git locales main et dev contiennent aussi la version jouable et les preuves.

`Files/PerforceCommits.png` est une capture réelle de P4V montrant les changements main/dev et la résolution.

`Files/VideoDemoJeux.mp4` contient environ 59 secondes de capture réelle dans Unreal : aperçu des trois vaisseaux, lancement avec Helios, déplacements, tirs, score, collisions, fin de partie et nouvelle partie. Les commandes sont simulées par l'outil d'enregistrement Editor `SpaceShooter.Delivery.RecordDemo`, exclu du build du jeu. Les images du viewport sont assemblées en respectant leurs horodatages; la vidéo est muette, la capture audio hors écran n'étant pas correctement synchronisée. Le jeu lui-même dispose de sons. Les sources de capture temporaires restent dans Saved, ignoré par Git et Perforce.

Dépôt GitHub public : https://github.com/overgamefr06-debug/TP1-SpaceShooter-OrbitBreaker

Les branches `main` et `dev` sont publiées avec leur historique complet, dont le commit de fusion `88f1ef1` qui résout le conflit volontaire. `Files/GithubCommits.png` est une capture réelle de l'historique GitHub. Les trois fichiers demandés (`GithubCommits.png`, `PerforceCommits.png`, `VideoDemoJeux.mp4`) sont réunis dans `Files` sur le stream main de Perforce, à côté de `Build/Windows`.

Les fichiers générés, les paramètres de connexion locaux, les notes et les transcriptions des cours restent hors du dépôt public. Le build est conservé dans Perforce et ignoré par Git. Les traces de tests locales restent dans `Artifacts`.

Avant de remettre ou présenter le travail, rejouer une partie et relire `Docs/Conflits_revision_control.md` ainsi que les classes C++ pour pouvoir expliquer les choix techniques.
