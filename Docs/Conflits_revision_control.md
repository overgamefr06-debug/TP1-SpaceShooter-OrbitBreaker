# Conflits volontaires Git et Perforce

Exercice réalisé le 29 septembre 2026 sur `Docs/ExerciceConflit.md`. Le document ne pilote pas le jeu. Les deux outils ont réellement refusé la fusion automatique, car main et dev modifiaient la même ligne de leur ancêtre commun.

## Git

- Base commune : commit `a54b9b1`.
- Main : `08df4bf`, validation du menu et des commandes.
- Dev : `fc27d18`, validation des tirs et des collisions.
- La commande `git merge --no-ff main` a produit `CONFLICT (content)` et un état `UU`.
- Résolution : `88f1ef1`, commit de fusion à deux parents, qui conserve les deux critères dans une phrase cohérente.

Les commits précédents et leurs dates sont conservés. Avant toute publication, un jeton local généré par le serveur de fichiers Android inutilisé a été retiré des versions Git destinées à GitHub; les identifiants ci-dessus correspondent à cet historique nettoyé.

## Perforce

- 5901 : base commune sur dev.
- 5902 : copie de dev vers main, avec le jeu et son build Windows.
- 5903 : modification de la ligne sur main.
- 5904 : modification concurrente de la même ligne sur dev.
- `p4 merge -Af --from //20263_8PRO135_KEVIN_ORTEGA/main Docs/ExerciceConflit.md`, puis `p4 resolve -am`, ont signalé **1 conflicting** et **resolve skipped**.
- Le fichier a été ouvert pour modification (Check Out), puis la phrase a été réécrite pour conserver les deux critères.
- `p4 resolve -ay` a accepté ce fichier de travail déjà fusionné manuellement, puis le changement **5905** a soumis la résolution. Le lien d'intégration porte donc l'étiquette `ignored` dans Perforce : c'est la sémantique de l'acceptation du fichier local. Le contenu final contient bien les apports main ET dev; aucune version brute n'a simplement remplacé l'autre.

## Pourquoi il y avait un conflit

Ancêtre : `Critères de validation : à compléter.`

Main : `Critères de validation : valider le menu et les commandes sur main.`

Dev : `Critères de validation : valider les tirs et les collisions sur dev.`

Résultat : `Critères de validation : valider le menu et les commandes (main), ainsi que les tirs et les collisions (dev).`

## Traces brutes

Les sorties réelles des outils lors de la détection sont archivées dans `ConflitGit.txt` et `ConflitPerforce.txt`. Les captures d'historique demandées pour la remise se trouvent dans `Files` lorsqu'elles sont disponibles.
