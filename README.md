<p align="center">
  <img src="logos/zzZ-UGA.png" alt="Flemme" width="50%"/>
</p>

<h1 align="center">flemme</h1>

<p align="center">
  <em>Un langage de script interprété, simple à lire, simple à écrire, simple à enseigner.</em>
</p>

---

## Qu'est-ce que c'est ?

**Flemme** est un langage de script à typage dynamique, inspiré de C et de Python, conçu pour deux usages complémentaires :

- **Enseignement de l'algorithmique** en licence et BTS — syntaxe proche du pseudo-code académique, exécutable sans installation complexe.
- **Pilotage de simulations C++** — un système de plugins permet d'exposer des objets C++ au script sans recompiler l'interpréteur.

```flemme
fn factorielle(n) {
  if (n <= 1) { return 1; }
  return n * factorielle(n - 1);
}

for (let i in range(8)) {
  print "{i}! = {factorielle(i)}";
}
```

---

## Philosophie et contexte

### Un projet assumé minimaliste

Flemme est développé avec un objectif délibéré : **rester simple, lisible et sans dépendances superflues**. L'interpréteur tient en quelques fichiers d'en-tête C++ (`lexer.hpp`, `parser.hpp`, `interpreter.hpp`) et un unique `main.cpp`. Le Web IDE n'utilise qu'une seule bibliothèque externe, [`cpp-httplib`](https://github.com/yhirose/cpp-httplib), téléchargée automatiquement à la compilation. C'est à peu près tout.

Cette contrainte est volontaire : la base de code doit rester lisible et modifiable par quelqu'un qui apprend le C++, pas seulement par son auteur.

### Un langage lent — et c'est très bien ainsi

Flemme est un **interpréteur tree-walking** : l'AST est parcouru directement à l'exécution, sans compilation en bytecode ni optimisation. Il est donc intrinsèquement **plus lent** que Python, Lua ou tout langage compilé. Ce n'est pas un objectif de le rendre rapide.

Les cas d'usage visés — algorithmes pédagogiques sur des données modestes, scripts de pilotage d'une simulation C++ qui fait elle-même le travail lourd — n'ont pas besoin de performance côté script. Si un script flemme est lent, c'est probablement un signe qu'il faudrait déplacer ce traitement dans un plugin C++.

### Développé avec Claude

Ce projet est développé en collaboration étroite avec [Claude](https://claude.ai) (Anthropic). La quasi-totalité du code, de la documentation et de l'outillage a été écrite ou co-écrite lors de sessions de travail interactives. Claude est utilisé non pas pour générer du code en une seule passe, mais comme **partenaire de conception** : discussion des choix d'architecture, revue du code, rédaction de la documentation, ajout de fonctionnalités.

C'est aussi, en soi, une expérience sur ce que permet ce mode de développement assisté pour un projet à ambition modeste et à périmètre bien défini.

---

## Ce que le langage sait faire

| Fonctionnalité | Exemple |
|---|---|
| Types dynamiques | `let x = 42;` · `let s = "bonjour";` · `let t = [1, 2, 3];` |
| Contrôle de flux | `if` / `else if` / `else` · `while` · `for` · `for...in` · `break` · `continue` |
| Fonctions | `fn max(a, b) { ... }` · récursivité · closures |
| Arithmétique complète | `+` `-` `*` `/` `//` `%` `^` |
| Tableaux | indexation · `push` / `pop` · `len` · `range` · tableaux 2D |
| Chaînes | `upper` `lower` `trim` `split` `find` `replace` · concaténation |
| Interpolation | `print "hypotenuse = {sqrt(a*a + b*b):.4f}";` |
| Bibliothèque standard | maths · conversions · `assert` · `type` · `input` |
| Plugins C++ | `let s = Stats(); s.add(3.14); print "{s.mean():.4f}";` |

---

## Démarrage rapide

### Prérequis

- Un compilateur C++17 (`g++` ou `clang++`)
- `make`
- Aucune autre dépendance (cpp-httplib est téléchargée automatiquement pour le Web IDE)

### Compilation

```bash
make           # compile les plugins puis flemme
make plugins   # compile uniquement les bibliothèques de plugins (.a)
```

Pour désactiver un plugin à la compilation :

```bash
make DISABLE_TIMER=1
make DISABLE_STATS=1
make DISABLE_MOCKROCKABLE=1
```

### Exécution

```bash
./flemme mon_script.flm
./flemme -v                               # version
./flemme --help                           # aide
./flemme --no-plugin timer script.flm    # désactive un plugin à l'exécution
```

Les fichiers source portent l'extension `.flm`.

### Web IDE

```bash
make web    # compile et lance l'IDE dans le navigateur (port 8765)
```

L'IDE intègre numéros de ligne, mise en évidence des erreurs, sauvegarde locale, chargement et téléchargement de fichiers `.flm`, ainsi qu'une galerie d'exemples.

---

## Système de plugins

Les plugins exposent des objets C++ au langage via `NativeObject`. Chaque plugin est autonome — il a son propre `Makefile`, ses propres sources et un fichier `*_plugin.hpp` qui définit le binding :

```
plugins/
├── timer/         FlemmeTimer.hpp  FlemmeTimer.cpp  Makefile  timer_plugin.hpp
├── stats/         Stats.hpp  Stats.cpp  Makefile  stats_plugin.hpp
└── mockrockable/  MockRockable.hpp  MockRockable.cpp  Makefile  mockrockable_plugin.hpp
```

```flemme
let t = Timer();

let s = Stats();
for (let x in donnees) { s.add(x); }
print "Temps    : {t.elapsed():.3f} s";
print "Moyenne  : {s.mean():.4f}";
print "Ecart-type : {s.stddev():.4f}";
```

Voir [`BINDING.md`](BINDING.md) pour créer son propre plugin.

---

## Documentation

| Fichier | Contenu |
|---|---|
| [`DOC-LANG.md`](DOC-LANG.md) | Manuel complet du langage (syntaxe, types, opérateurs, bibliothèque standard, exemples) |
| [`BINDING.md`](BINDING.md) | Guide du système de plugins C++ |
| [`SYNTAX-COLOR.md`](SYNTAX-COLOR.md) | Installation de la coloration syntaxique (TextMate, Vim/Neovim) |
| [`tp/`](tp/) | Série d'exercices graduels TP1–TP5 avec corrigés |

---

## Structure du projet

```
flemme/
├── lexer.hpp         # analyseur lexical
├── parser.hpp        # parseur → AST
├── interpreter.hpp   # interpréteur (tree-walking)
├── plugins.hpp       # agrégateur de plugins (à éditer pour en activer un)
├── main.cpp          # point d'entrée CLI
├── webide.cpp        # serveur Web IDE (cpp-httplib)
├── plugins/          # plugins C++ intégrés
│   ├── timer/        # chronomètre (libflemme-timer.a)
│   ├── stats/        # statistiques descriptives (libflemme-stats.a)
│   └── mockrockable/ # simulation DEM factice (libflemme-mockrockable.a)
├── tests/            # suite de tests automatisés
├── tp/               # exercices pédagogiques
└── vim/              # plugin de coloration Vim/Neovim
```

---

## Tests

```bash
cd tests && bash test_all.sh       # tous les tests
cd tests && bash test_all.sh -v    # avec sortie détaillée
```
