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

Les exclusions sont préparées. Le dépôt GitHub public et les streams Perforce ne sont pas encore configurés. Renseigner l'identité Git de l'étudiant avant le premier commit. Pour Perforce, activer `.p4ignore` dans le contexte du projet; conserver `Build/Windows` dans la remise Perforce.

Ne pas ajouter les notes de cours, transcriptions ou outils d'analyse au projet remis.
