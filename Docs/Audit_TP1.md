# Vérification du TP1 — 30 septembre 2026

Référence : « TP1 - Space Shooter - Revison Control.pdf », trois pages. Cette vérification confronte le barème aux sources, aux assets, aux tests du jeu et aux dépôts. Elle ne constitue ni une note attribuée par le professeur ni une confirmation administrative de remise.

## Barème, point par point

| Critère du PDF | Vérification / preuve | État |
|---|---|---|
| Structure du projet, 0,5 | Projet C++ ouvrable, carte L_Arena, assets dans Content, build et Files au niveau du stream main. | Réalisé |
| Norme de remise, 0,5 | Build/Windows/SpaceShooter.exe, sous-dossiers Engine et SpaceShooter ; trois pièces nommées exactement comme dans le sujet. Les autres règles éventuelles du cours ne figurent pas dans ce PDF. | Structure vérifiée ; modalité administrative à confirmer |
| Fonctionnement C++ et paramétrage Blueprint, 3 | ShipPawn, ShotProjectile, SpaceAsteroid, SpaceGameMode, SpaceHUD et CombatBurst ; leurs Blueprint enfants règlent classes, vitesses, résistances, visuels et sons. Voir Blueprints_et_reglages.md. | Réalisé |
| Polish, 3 | Flotte, astéroïdes détourés, logo animé, décor, particules, bouclier, HP, bonus, effets sonores, musiques et transitions. L'appréciation esthétique reste subjective. | Réalisé, à apprécier en jouant |
| Menu jouer/quitter/auteur, 1 | Menu principal, bouton Jouer, bouton Quitter ; Kevin Ortega affiché. Bilan avec Rejouer / Retour au menu. | Réalisé |
| Vaisseau visible, 1 | Trois sprites, Aegis disponible dès une sauvegarde neuve. Spectre / Helios déblocables. | Réalisé |
| Quatre directions, 2 | Flèches, ZQSD et WASD ; tests de déplacement et limites. | Réalisé |
| Tir sur une touche, 2 | Espace et clic gauche, cadence, projectiles, impact unique ; tests Combat. | Réalisé |
| Astéroïdes : bords, temps et position aléatoires, 2 | SpawnAsteroid choisit un bord et une position ; délai variable ; tests RunLoop. | Réalisé |
| Force initiale, 3 | Chaos : SphereComponent simule la physique, Launch utilise AddImpulse avec variation de direction et vitesse. | Réalisé |
| Nombre aléatoire de tirs pour détruire, 2 | Taille tirée au hasard à la création : petit 1, moyen 2, grand 3. Résistance aléatoire entre astéroïdes, déterministe pour une catégorie, conformément au choix de jeu demandé. Tests Combat. | Réalisé selon cette interprétation du sujet |
| Contact astéroïde/joueur enlève une vie, 1 | Retire une vie, puis 1,5 s d'invulnérabilité ; exception volontaire du bonus bouclier. Tests RunLoop et ArcadeSystems. | Réalisé |
| Interface score/vies, 1 | Score en haut à gauche et silhouettes de vies à droite ; actualisation après tir et collision. | Réalisé |
| Effets visuels de tir et destruction, 1 | Éclat, laser, poussière et étincelles transparents. Deux tests visuels protègent contre les rectangles noirs signalés. | Réalisé |
| GitHub public, 0,5 | overgamefr06-debug/TP1-SpaceShooter-OrbitBreaker ; visibilité PUBLIC vérifiée avec l'API GitHub. | Réalisé |
| Gitignore, 0,5 | Saved, Intermediate, Binaries, DerivedDataCache, paramètres locaux et Build/Windows exclus ; aucun de ces fichiers temporaires dans les fichiers Git suivis. | Réalisé |
| Git main et premier push, 0,5 | main publié, historique remontant à l'initialisation 99e7369 ; capture GithubCommits.png. | Réalisé |
| Git dev, 0,5 | dev publié avec le projet. | Réalisé |
| Conflit Git volontaire résolu, 1 | Fusion 88f1ef1 à deux parents ; modifications concurrentes main/dev et résolution conservant les deux critères. Traces brutes et capture. | Réalisé |
| Perforce main et initialisation, 0,5 | Stream mainline, changement initial 5801. | Réalisé |
| Perforce dev, 0,5 | Stream development enfant de main, peuplé en 5803. | Réalisé |
| P4ignore, 0,5 | Fichier .p4ignore avec exclusions Unreal et configurations de connexion. Build/Windows reste suivi. | Réalisé |
| Conflit Perforce volontaire résolu, 1,5 | Changements 5901–5905 ; trace « 1 conflicting », résolution manuelle et intégration, capture P4V. Voir Conflits_revision_control.md pour le sens du lien « ignored ». | Réalisé |
| Commits Perforce pertinents, 1 | Jalons distincts : initialisation, tirs, jeu, conflits, flotte, bonus, effets, son et build. | Réalisé |

## Fichiers de remise et validation finale

Le build autonome complet doit rester sous 500 000 000 octets et se trouver dans **main Perforce**, pas uniquement sur le poste local. Le build n'est pas demandé dans GitHub et y reste ignoré. Les captures d'historique montrent les exercices de conflit, pas seulement une liste de changements récents.

Les trois pièces obligatoires sont `Files/GithubCommits.png`, `Files/PerforceCommits.png` et `Files/VideoDemoJeux.mp4`. La vidéo actuelle est l’enregistrement OBS du 1er octobre 2026, copié sans réencodage sous le nom exigé. Elle remplace la démonstration automatisée précédente.

Validation finale : **8 tests réussis, 0 échec, 0 avertissement de test**, compilation et packaging Shipping réussis. Build : **371 653 399 octets (371,65 Mo)**, 26 fichiers comparés par empreinte avec la sortie du packaging. La synchronisation des branches et streams est vérifiée au moment de publier ce jalon. Les rapports détaillés restent en local dans Artifacts, conformément aux exclusions.

## Ce qu'il reste à faire pour remettre

1. **Confirmer la date applicable.** Le PDF annonce « 23 septembre, 23 h 59 », sans année. L'audit est daté du 30 septembre 2026 ; aucune prolongation ou autre date n'est présumée.
2. **Suivre la procédure administrative du cours**, si une validation Moodle, un lien ou une déclaration est demandé en plus de la présence du build sur Perforce. Aucun envoi final sur Moodle n'est attesté ici. Le PDF fourni décrit surtout l'arborescence Perforce.
3. **Faire une dernière partie soi-même et savoir présenter le code.** Le travail est individuel : comprendre les classes C++, montrer les valeurs modifiables dans les Blueprints, expliquer AddImpulse, la résistance, le score et les deux conflits. Le guide Blueprints_et_reglages.md et les traces de conflit servent de support.

La résistance 1/2/3 est liée à la taille aléatoire, et non à un second tirage indépendant pour chaque astéroïde de même taille. C'est le comportement explicitement demandé pour ce jeu ; cette précision permet de le présenter honnêtement au professeur.

Le document indique « Valeur : 25 pt » en première page mais « TOTAL /30 » au barème. L'audit ne tente pas de convertir ni de garantir une note.

## Contrôles du nettoyage final

- Compilation Editor et packaging Shipping refaits après les suppressions ; build autonome lancé, menu Jouer/Quitter, partie, tir, redémarrage, retour au menu et fermeture vérifiés.
- 29 assets orphelins supprimés ; 65 assets actifs, aucune dépendance /Game manquante. Les références C++/Config ont aussi été recherchées pour ne pas se limiter aux références de l’Asset Registry. Aucun chargement dynamique d’un asset supprimé n’a été trouvé.
- 16 composants de fragments invisibles par effet, composant Wings vide, anciens réglages et fonction HUD inutilisés retirés. Les Blueprints recompilés restent les points de paramétrage du jeu.
- Quatre acteurs de bordure cachés retirés de L_Arena ; aucune modification des règles de jeu demandées.
- Trois scripts de prototype remplacés par les seuls imports actifs ; réimportation complète exécutée avec succès. Tous les scripts Python conservés passent l’analyse syntaxique.
- Exclusions complétées pour .slnx, .vsconfig et caches Python. Aucun cache, fichier de connexion ou jeton détecté dans les fichiers texte suivis lors du contrôle final.
- Rapports locaux : Artifacts/CleanupValidation/index.json (8 succès), Artifacts/AssetValidation.json et Artifacts/DeliveryValidation.json (26 empreintes du build).
- Démonstration automatisée validée le 30 septembre : 835 images du viewport, environ 59 secondes, mix audio Unreal non silencieux et non écrêté. Cette ancienne démonstration a été remplacée le 1er octobre par la nouvelle vidéo (82,70 s, 720p/30, audio AAC, 63 728 625 octets), dont le décodage intégral et l’identité avec le fichier source ont été vérifiés. Les résultats de tests ci-dessus concernent toujours le build inchangé.
- Avertissements d’outillage : Unreal signale que la version installée du compilateur Visual Studio n’est pas sa version préférée, et des dépréciations dans les en-têtes du moteur. Ils n’empêchent ni la compilation ni le packaging ; les tests du projet n’ont aucun avertissement.

Les copies locales issues de l’ouverture avec une association de moteur incorrecte ont été comparées : aucune modification de gameplay propre à ces copies. Leurs sauvegardes et réglages ont été préservés. La progression de la copie la plus récente (16 000 points) a été reprise dans le projet éditeur principal ; la sauvegarde du jeu autonome est restée distincte et intacte.

Le contrôle automatique de suppression récursive a refusé l’effacement des copies et des caches sans fournir de motif détaillé. Ils ont donc été déplacés hors du projet dans `C:/Users/Over/Documents/OrbitBreaker_Backup_2026-09-30`. Cette archive privée n’est ni versionnée ni destinée à la remise ; elle occupe toujours de l’espace disque. Le nettoyage des assets suivis, lui, est effectif et conservé dans les historiques.
