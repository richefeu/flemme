# TODO — Flemme

## Langage : ergonomie et fonctionnalités

### Fait
- [x] Opérateurs composés `+=` `-=` `*=` `/=`
- [x] Incrémentation / décrémentation `i++` `i--`
- [x] Comparaison de chaînes `==` `!=` `<` `>` `<=` `>=`
- [x] `push(arr, val)` et `pop(arr)`
- [x] `input(prompt)` — lecture stdin
- [x] `num(s)` / `str(x)` — conversions
- [x] Affichage des tableaux dans `print` : `[1, 2, 3]`
- [x] Commentaires uniquement avec `#`
- [x] Division entière `//`
- [x] Numéros de ligne dans les messages d'erreur
- [x] Fonctions sur les chaînes : `upper`, `lower`, `trim`, `split`, `find`, `replace`
- [x] `type(x)` → retourne le nom du type (`"nombre"`, `"chaine"`, `"tableau"`, …)
- [x] `print_raw(...)` — affichage sans saut de ligne
- [x] `assert(cond [, msg])` — assertion avec message
- [x] Tableaux 2D (tableaux de tableaux) + accès `mat[i][j]`
- [x] Interpolation d'expressions dans les chaînes : `"{sin(3)}"`, `"{a + b}"`, `"{f(x)}"` — le contenu de `{...}` est parsé et évalué comme une expression complète

### À faire (fonctionnalités avancées — non prioritaires)
- [ ] Dictionnaires / tables associatives `{"clé": valeur}`
- [ ] Portée des fonctions : closures complètes (capture de variables libres)
- [ ] Gestion des erreurs dans le script : `try` / `catch`

### À faire (petites améliorations)
- [ ] Opérateur ternaire `a ? b : c`
- [ ] Pré-incrémentation `++i` / `--i`
- [ ] `range(a, b)` — forme à deux arguments

---

## Outillage

### Fait
- [x] README.md et DOC-LANG.md (documentation française complète)
- [x] Coloration syntaxique TextMate 2 / Sublime Text (`.tmLanguage`)
- [x] Coloration syntaxique Vim / Neovim (plugin `vim/`)
- [x] SYNTAX-COLOR.md (guide d'installation des plugins)
- [x] Web IDE (`flemme-web`) — éditeur + sortie dans le navigateur, timeout 5 s
- [x] Web IDE : numéros de ligne (gouttière)
- [x] Web IDE : mise en évidence de la ligne en erreur
- [x] Web IDE : sauvegarde locale du code (localStorage)
- [x] Web IDE : exemples prédéfinis (menu déroulant)
- [x] Web IDE : bouton de chargement d'un fichier `.flm` local
- [x] Web IDE : bouton de téléchargement du fichier `.flm`
- [x] Tests automatisés (`test_all.sh`) — valide tous les `.flm` de test
- [x] Suite de tests remaniée : 17 fichiers thématiques avec `assert` (variables, conditions, boucles, fonctions, opérateurs, tableaux, chaînes, maths, conversions, interpolation, algorithmes, plugins)
- [x] BINDING.md — documentation du système de plugins C++

### À faire
- [ ] Web IDE : coloration syntaxique dans l'éditeur (CodeMirror 6)
- [ ] Web IDE : mini-tutoriel interactif sur la page d'accueil
- [ ] Web IDE : logo dans le `<header>`
- [ ] Extension VS Code (`.vsix`) : grammaire + `flemme-web` intégré comme tâche

---

## Système de plugins C++

### Fait
- [x] `NativeObject` : conteneur générique (getters, setters, méthodes via lambdas)
- [x] Architecture plugin : chaque plugin = `.hpp` + `.cpp` + `Makefile` + `*_plugin.hpp`
- [x] `plugins.hpp` — agrégateur unique, un `#ifdef FLEMME_PLUGIN_X` par plugin
- [x] Plugin `timer` (`FlemmeTimer`) avec script de test
- [x] Plugin `stats` (`Stats`) avec script de test
- [x] Plugin `mockrockable` (`MockRockable`) avec script de test
- [x] Désactivation à la compilation : `make DISABLE_X=1`
- [x] Désactivation à la compilation pour le web uniquement : `make WEBIDE_DISABLE_X=1`
- [x] Désactivation à l'exécution : `./flemme --no-plugin x script.flm`
- [x] `rockable_plugin.hpp` — squelette prêt pour le vrai Rockable (commenté dans `plugins.hpp`)

### À faire
- [ ] Plugin Rockable réel : tester avec la vraie lib DEM
- [ ] Permettre l'exposition de tableaux C++ comme `ArrayObject` depuis un plugin

---

## Pédagogie

### Fait
- [x] Série d'exercices graduels TP1 … TP5 (`tp/tp1.flm` … `tp/tp5.flm`)
- [x] Corrigés (`tp/solutions/tp1.flm` … `tp/solutions/tp5.flm`)

### À faire
- [ ] TP6 : exercices sur les chaînes (`upper`, `split`, `find`, `replace`, interpolation)
- [ ] Corrigés commentés (annotations pédagogiques ligne par ligne)
