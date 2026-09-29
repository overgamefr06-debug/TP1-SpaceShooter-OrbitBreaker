# Orbit Breaker — TP1 Space Shooter

Prototype jouable réalisé avec Unreal Engine 5.8.3. Logique de jeu en C++, paramètres et références des assets dans les Blueprints. Troisième jalon : boucle de partie et direction artistique.

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
- Build Windows autonome lancé et menu affiché. Le test automatisé des commandes a été effectué dans l'éditeur; ne pas le confondre avec un test manuel exhaustif du build.
- Taille du build complet : **364 531 345 octets**, soit **364,53 Mo**, sous la limite de 500 Mo. Cette version reste un jalon de développement, pas la remise finale.

## Gestion de versions et travail restant

Le dossier de travail `SpaceShooter` correspond au stream Perforce `//20263_8PRO135_KEVIN_ORTEGA/dev`, workspace `kortega_tp1_dev_DESKTOP_6NT64UN`. Le serveur est `ssl:p4prod.uqac.ca:1666`, utilisateur `kortega`. Avant de modifier un fichier suivi, faire Check Out; les assets utilisent le type exclusif `binary+l`. Les caches, rapports et paramètres de connexion restent exclus. Le dossier `Build/Windows` est versionné dans Perforce et ignoré dans Git.

Les branches Git locales `main` et `dev` existent. Git et Perforce ont des historiques distincts. Ne pas changer de branche Git au milieu de modifications Perforce. Le premier envoi Perforce est 5801 sur main, le peuplement de dev 5803, et le jalon tir/destruction 5847 sur dev.

Avant la remise : créer/publier le dépôt GitHub public, réaliser et documenter les conflits volontaires Git et Perforce, intégrer la version finale sur main, enregistrer `Files/GithubCommits.png`, `Files/PerforceCommits.png` et `Files/VideoDemoJeux.mp4`, puis refaire un contrôle final du build. Ces éléments ne sont pas encore réalisés. Les notes et transcriptions des cours restent hors du projet remis.
