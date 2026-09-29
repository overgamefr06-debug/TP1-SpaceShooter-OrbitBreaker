# TP1 — Space Shooter

Premier jalon : projet C++ Unreal 5.8, vaisseau pilotable et réglages Blueprint.

Ouvrir `SpaceShooter.uproject`, puis la carte `Content/Maps/L_Arena`. Lancer Play et cliquer dans la vue. Déplacement : flèches, ZQSD ou WASD. Échap arrête le test dans l'éditeur.

## Répartition

- `AShipPawn` : composants, entrées Enhanced Input, déplacement dans le plan et limites de l'arène.
- `BP_Ship` : enfant du vaisseau C++, apparence et vitesse réglables.
- `ASpaceGameMode` : caméra et démarrage du premier prototype.
- `BP_SpaceGameMode` : choix du Blueprint du joueur.

Les formes simples sont temporaires. Le tir, les astéroïdes, les vies, le score, le menu et les effets viendront après validation de ce jalon.

## Gestion de versions

Les exclusions sont préparées. Le dépôt local contient un premier commit sur main et la branche dev, avec l'identité Git choisie par Over. Le dépôt GitHub public et les streams Perforce ne sont pas encore configurés. Pour Perforce, activer `.p4ignore` dans le contexte du projet; conserver `Build/Windows` dans la remise Perforce.

Ne pas ajouter les notes de cours, transcriptions ou outils d'analyse au projet remis.


## Validation du premier jalon — 29 septembre 2026

- Compilation C++ Editor et Shipping réussie avec Unreal 5.8.3.
- Test `SpaceShooter.Gameplay.ShipControls` réussi : dix touches, directions et limites de l’arène. Rapport local dans `Artifacts/Tests/index.json`.
- Carte, caméra et vaisseau contrôlés sur une capture réelle du jeu, conservée dans `Artifacts/Apercu_jalon1.png`.
- Build Windows complet : **363 785 234 octets**, soit **363,79 Mo**. Cette mesure concerne le premier prototype, pas la remise finale.
- L’exécutable Shipping autonome a été lancé pendant au moins 45 secondes, puis son processus de test a été arrêté. Le test automatisé des touches a été exécuté dans l’éditeur, pas dans le build Shipping.

Pour essayer sans l’éditeur, lancer `Build/Windows/SpaceShooter.exe`. Utiliser les flèches, ZQSD ou WASD; **Alt+F4** ferme cette première version. Il n’y a pas encore de menu.

Prochain jalon : tir, projectile et premier astéroïde destructible. Configurer GitHub et Perforce suffisamment tôt pour conserver un vrai historique de développement. Les conflits volontaires, captures et vidéo finale restent à réaliser.

Les versions d’Unreal et du compilateur ont été lues sur le PC; la configuration C++ a aussi été confrontée à la [documentation officielle Epic](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-your-development-environment-for-cplusplus-in-unreal-engine).
