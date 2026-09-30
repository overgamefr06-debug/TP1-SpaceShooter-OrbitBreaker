# Orbit Breaker — TP1 Space Shooter

Prototype jouable réalisé avec Unreal Engine 5.8.3. Logique de jeu en C++, paramètres et références des assets dans les Blueprints. Version jouable avec exercices de gestion de versions et vidéo de démonstration.

## Ouvrir et jouer

Ouvrir `SpaceShooter.uproject`, charger `Content/Maps/L_Arena` et lancer Play. Dans le menu, cliquer **Lancer la mission** ou appuyer sur **Entrée**.

- Flèches, ZQSD ou WASD : déplacement dans les quatre directions.
- Espace ou clic gauche, maintenu : tir avec cadence limitée.
- R : recommencer une partie en cours ou terminée.
- Échap : retour au menu dans le build; dans Play in Editor, le raccourci d'Unreal arrête généralement le test.
- Quitter : bouton du menu principal. Depuis la fin de partie, Retour au menu permet de le retrouver.

Exécutable autonome : `Build/Windows/SpaceShooter.exe`. Le dossier Windows complet est nécessaire; ne pas déplacer uniquement le petit exécutable de lancement.

## Comportement

Les astéroïdes apparaissent sur l'un des quatre bords, à une position et après un délai aléatoires. Une impulsion physique initiale les dirige vers la position du joueur avec une dispersion; ils ne poursuivent pas ensuite le joueur. Les astéroïdes tournent, ont une taille variable et nécessitent de un à trois tirs, tirés au hasard une seule fois à leur création.

La destruction par un projectile rapporte 100 points. Un contact retire une vie et consomme l'astéroïde. Une protection de 1,5 seconde évite de perdre plusieurs vies immédiatement; le vaisseau clignote pendant cette récupération. Une collision durant cette protection consomme aussi l'astéroïde, sans score. À zéro vie, les apparitions et le score s'arrêtent et le bilan propose de rejouer ou de revenir au menu.

La difficulté augmente progressivement avec la vitesse des astéroïdes (plafond +50 % après trois minutes). Leur nombre est limité et ceux sortis de la zone sont supprimés. Le redémarrage nettoie astéroïdes, projectiles et effets puis réinitialise score, vies et durée.

## Architecture et réglages

| C++ | Blueprint | Responsabilité et valeurs utiles |
|---|---|---|
| ShipPawn | BP_Ship | Entrées, mouvement, limites, cadence, projectile, effets, meshes |
| ShotProjectile | BP_Projectile | Mouvement balayé, impact unique, vitesse et durée de vie |
| SpaceAsteroid | BP_Asteroid | Résistance, impulsion physique, rotation, variantes de mesh et tailles |
| SpaceGameMode | BP_SpaceGameMode | Menu/partie/fin, score et vies; délais, vitesse, bord et plafond des apparitions |
| CombatBurst | BP_MuzzleFlash / BP_AsteroidBurst | Fragments animés, durée, couleur et son |
| SpaceHUD | BP_OrbitHUD | Interface Canvas, menu cliquable, textes, auteur, police et couleurs |

L'interface est dessinée en C++ avec une police vectorielle et une mise à l'échelle; son Blueprint expose les textes et la direction artistique. Ce n'est pas encore un écran composé dans le Designer UMG.

## Assets originaux

`Content/Art/Meshes` contient le vaisseau K-07, ses deux jets et trois variantes d'astéroïdes rocheux à facettes. Les couleurs par sommet donnent une surface lisible sans éclairage coûteux. `Content/Art` contient aussi la nébuleuse, ses matériaux et les sons de laser/impact.

Les meshes, la texture et les sons ont été créés pour ce projet, sans pack externe. Les sources PNG/WAV sont dans `ArtSources`; la génération est documentée dans `Tools/generate_atmosphere.py` et `Tools/create_orbit_art.py`. Les polices et primitives restantes proviennent d'Unreal. Les scripts de génération ne doivent être relancés qu'après checkout des assets concernés; ils appliquent les réglages de ce jalon.

Les plugins Geometry Scripting, EditorToolset et MCP servent uniquement à l'éditeur et sont exclus de la cible du jeu. MCP peut être démarré avec `ModelContextProtocol.StartServer 8000`; adresse locale `http://127.0.0.1:8000/mcp`. Il ne démarre pas automatiquement dans le build Windows.

## Validation — 29 septembre 2026

- Compilation Editor et packaging Shipping réussis.
- Trois tests de gameplay réussis, sans avertissement : `Combat`, `ShipControls`, `RunLoop`. Rapport local : `Artifacts/TestsJalon3Final/index.json`.
- Combat : touches de tir, cadence, impact unique, tir à bout portant, projectile rapide balayé, résistance et expiration.
- Mouvement : dix touches, directions, contrainte du plan et limites.
- Partie : écran initial, apparitions temporisées, positions sur les bords, déplacement après impulsion, score, collisions physiques, protection temporaire, dernière vie, arrêt du score, redémarrage et retour au menu.
- Menu et assets inspectés visuellement dans le jeu Development; lancement depuis le bouton et fin de partie observés. La révision de police a ensuite été inspectée visuellement.
- Build Windows Shipping final lancé depuis le workspace main : menu, démarrage par Entrée, apparitions et fin de partie, redémarrage par R, projectile à l'écran, retour au menu par Échap et bouton Quitter vérifiés. Les tests complets de mouvement et de collisions restent ceux exécutés dans l'éditeur.
- Taille du build complet : **363 202 611 octets**, soit **363,20 Mo** (hors journaux temporaires exclus du dépôt), sous la limite de 500 Mo. Le build jouable est également intégré sur le stream Perforce main. La structure de remise contient le build Windows, la vidéo et les captures des deux historiques.

## Gestion de versions et remise

Le dossier de travail `SpaceShooter` correspond au stream Perforce `//20263_8PRO135_KEVIN_ORTEGA/dev`, workspace `kortega_tp1_dev_DESKTOP_6NT64UN`. Le serveur est `ssl:p4prod.uqac.ca:1666`, utilisateur `kortega`. Avant de modifier un fichier suivi, faire Check Out; les assets utilisent le type exclusif `binary+l`. Les caches, rapports et paramètres de connexion restent exclus. Le dossier `Build/Windows` est versionné dans Perforce et ignoré dans Git.

Les branches Git locales `main` et `dev` existent. Git et Perforce ont des historiques distincts. Ne pas changer de branche Git au milieu de modifications Perforce. Le premier envoi Perforce est 5801 sur main, le peuplement de dev 5803, et le jalon tir/destruction 5847 sur dev.

Les conflits volontaires Git et Perforce sont réalisés et expliqués dans `Docs/Conflits_revision_control.md`, avec les sorties brutes des deux outils. Le commit de fusion Git conserve ses deux parents; Perforce contient les changements 5901 à 5905. Le jeu et le build ont été intégrés sur main dans Perforce (5902), puis le build reconstruit et les preuves de remise dans 5913. Les branches Git locales main et dev contiennent aussi la version jouable et les preuves.

`Files/PerforceCommits.png` est une capture réelle de P4V montrant les changements main/dev et la résolution.

`Files/VideoDemoJeux.mp4` contient 34 secondes de gameplay réel capturé dans Unreal : menu, quatre directions, tirs, score, collisions, perte des trois vies, fin de partie et nouvelle partie. Les commandes sont simulées par l'outil d'enregistrement Editor `SpaceShooter.Delivery.RecordDemo`, exclu du build du jeu. Les images du viewport sont assemblées en respectant leurs horodatages; la vidéo est muette, la capture audio hors écran n'étant pas correctement synchronisée. Le jeu lui-même dispose de sons. Les sources de capture temporaires restent dans Saved, ignoré par Git et Perforce.

Dépôt GitHub public : https://github.com/overgamefr06-debug/TP1-SpaceShooter-OrbitBreaker

Les branches `main` et `dev` sont publiées avec leur historique complet, dont le commit de fusion `88f1ef1` qui résout le conflit volontaire. `Files/GithubCommits.png` est une capture réelle de l'historique GitHub. Les trois fichiers demandés (`GithubCommits.png`, `PerforceCommits.png`, `VideoDemoJeux.mp4`) sont réunis dans `Files` sur le stream main de Perforce, à côté de `Build/Windows`.

Les fichiers générés, les paramètres de connexion locaux, les notes et les transcriptions des cours restent hors du dépôt public. Le build est conservé dans Perforce et ignoré par Git. Les traces de tests locales restent dans `Artifacts`.

Avant de remettre ou présenter le travail, rejouer une partie et relire `Docs/Conflits_revision_control.md` ainsi que les classes C++ pour pouvoir expliquer les choix techniques.
