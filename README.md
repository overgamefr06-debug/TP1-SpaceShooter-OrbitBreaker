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

Les exclusions sont préparées. Le dépôt local contient un premier commit sur main et la branche dev, avec l'identité Git choisie par Over. Le dépôt GitHub public reste à créer. Perforce est maintenant configuré sur le serveur UQAC avec les streams main et dev. Pour Perforce, activer `.p4ignore` dans le contexte du projet; conserver `Build/Windows` dans la remise Perforce.

Ne pas ajouter les notes de cours, transcriptions ou outils d'analyse au projet remis.


## Validation du premier jalon — 29 septembre 2026

- Compilation C++ Editor et Shipping réussie avec Unreal 5.8.3.
- Test `SpaceShooter.Gameplay.ShipControls` réussi : dix touches, directions et limites de l’arène. Rapport local dans `Artifacts/Tests/index.json`.
- Carte, caméra et vaisseau contrôlés sur une capture réelle du jeu, conservée dans `Artifacts/Apercu_jalon1.png`.
- Build Windows complet : **363 785 234 octets**, soit **363,79 Mo**. Cette mesure concerne le premier prototype, pas la remise finale.
- L’exécutable Shipping autonome a été lancé pendant au moins 45 secondes, puis son processus de test a été arrêté. Le test automatisé des touches a été exécuté dans l’éditeur, pas dans le build Shipping.

Pour essayer sans l’éditeur, lancer `Build/Windows/SpaceShooter.exe`. Utiliser les flèches, ZQSD ou WASD; **Alt+F4** ferme cette première version. Il n’y a pas encore de menu.

Prochain jalon : tir, projectile et premier astéroïde destructible. Configurer GitHub pour publier les branches Git. Continuer les changelists Perforce au fil du développement. Les conflits volontaires, captures et vidéo finale restent à réaliser.

Les versions d’Unreal et du compilateur ont été lues sur le PC; la configuration C++ a aussi été confrontée à la [documentation officielle Epic](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-your-development-environment-for-cplusplus-in-unreal-engine).


## Connexion Perforce — 29 septembre 2026

- Serveur : `ssl:p4prod.uqac.ca:1666`; compte universitaire : `kortega`.
- Dépôt attribué : `20263_8PRO135_KEVIN_ORTEGA`.
- Stream principal : `//20263_8PRO135_KEVIN_ORTEGA/main`; premier envoi : changelist **5801**, 51 fichiers dont le build Windows complet.
- Stream de développement : `//20263_8PRO135_KEVIN_ORTEGA/dev`, créé depuis main dans la changelist **5803**.
- Workspace de travail : `kortega_tp1_dev_DESKTOP_6NT64UN`, racine `C:\Users\Over\Desktop\Tp1 Unreal\SpaceShooter`.
- Workspace principal : `kortega_tp1_main_DESKTOP_6NT64UN`, racine `C:\Users\Over\Desktop\Tp1 Unreal\Perforce\main`.

Continuer à ouvrir le projet dans **SpaceShooter**, qui correspond au stream **dev**. Les dossiers locaux des deux streams sont distincts. Les paramètres de connexion sont locaux, sans mot de passe enregistré dans le projet versionné.

Avant une modification, ouvrir les fichiers pour édition (Check Out). Dans P4V, contrôler les fichiers de la changelist avant Submit. Les assets `.uasset` et `.umap` sont de type `binary+l` pour leur ouverture exclusive. Les fichiers non ouverts sont en lecture seule; ne pas utiliser Make Writable pour remplacer le checkout.

Les exclusions des caches sont ancrées à la racine : les dossiers Binaries nécessaires **à l’intérieur du build Windows** restent versionnés. `.git`, `.p4config`, caches, rapports de tests et journaux intermédiaires de packaging sont exclus.

Les branches Git et les streams Perforce sont deux historiques distincts. Ne pas changer de branche Git au milieu de modifications Perforce sans contrôler les changements locaux. Le conflit demandé par le TP reste à réaliser séparément dans chaque outil.
