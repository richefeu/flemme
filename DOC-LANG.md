# Flemme — Manuel du langage

**Flemme** est un langage de script interprété, à typage dynamique, inspiré de C et de Python. Il est conçu pour l'enseignement de l'algorithmique en licence et BTS, et pour servir de langage de pilotage (scripting) pour des codes de simulation C++. Sa syntaxe est volontairement proche du pseudo-code académique tout en restant exécutable.

---

## Table des matières

1. [Mise en route](#1-mise-en-route)
2. [Syntaxe fondamentale](#2-syntaxe-fondamentale)
3. [Types de données](#3-types-de-données)
4. [Opérateurs](#4-opérateurs)
5. [Structures de contrôle](#5-structures-de-contrôle)
6. [Fonctions](#6-fonctions)
7. [Tableaux](#7-tableaux)
8. [Chaînes de caractères](#8-chaînes-de-caractères)
9. [Bibliothèque standard](#9-bibliothèque-standard)
10. [Plugins C++](#10-plugins-c)
11. [Exemples complets](#11-exemples-complets)
12. [Résumé de la grammaire](#12-résumé-de-la-grammaire)

---

## 1. Mise en route

### Compilation de l'interpréteur

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

### Exécution d'un script

```bash
./flemme mon_script.flm
./flemme -v                             # affiche la version
./flemme --help                         # aide
./flemme --no-plugin timer script.flm  # désactive un plugin à l'exécution
```

Les fichiers source portent l'extension `.flm` par convention.

### Web IDE

```bash
make web    # compile et lance l'IDE dans le navigateur (port 8765)
```

---

## 2. Syntaxe fondamentale

### 2.1 Commentaires

Un commentaire commence par `#` et s'étend jusqu'à la fin de la ligne. C'est le **seul** style de commentaire — le symbole `//` est réservé à la division entière.

```flemme
# Ceci est un commentaire
let x = 10;   # commentaire en fin de ligne
```

### 2.2 Instructions et points-virgules

Chaque instruction se termine par un point-virgule `;`. Le langage ignore les espaces et les sauts de ligne entre les tokens, ce qui permet d'écrire plusieurs instructions par ligne ou d'étaler une instruction sur plusieurs lignes.

```flemme
let a = 1; let b = 2; let c = a + b;
```

### 2.3 Blocs

Un bloc regroupe plusieurs instructions entre accolades `{ }`. Les blocs définissent la portée des variables.

```flemme
{
  let local = 42;   # visible uniquement dans ce bloc
}
```

### 2.4 Affichage : `print` et `print_raw`

`print` affiche une chaîne avec interpolation puis un retour à la ligne. `print_raw` fait la même chose **sans** retour à la ligne.

> **Important** : l'interpolation `{...}` est une fonctionnalité de l'instruction `print` (et `print_raw`), pas des chaînes en général. Une affectation comme `let s = "{x}";` produit la chaîne littérale `"{x}"`, sans substitution.

```flemme
let n = 7;
print "n vaut {n}";          # affiche : n vaut 7
print_raw("suite... ");      # affiche sans aller à la ligne
print_raw("fin");            # continue sur la même ligne
print "";                    # retour à la ligne explicite
```

#### Interpolation d'expressions

Les placeholders `{...}` acceptent n'importe quelle expression flemme : variables, opérations arithmétiques, appels de fonctions, accès membres.

```flemme
let a = 3;
let b = 4;
print "hypotenuse = {sqrt(a*a + b*b):.4f}";   # hypotenuse = 5.0000
print "sin(pi/6)  = {sin(3.14159 / 6):.4f}";  # sin(pi/6)  = 0.5000
print "len(\"abc\") = {len(\"abc\")}";         # len("abc") = 3
```

#### Format numérique

`{expr:.Nf}` formate un nombre avec `N` décimales.

```flemme
let pi = 3.14159265358979;
print "pi ≈ {pi:.2f}";          # pi ≈ 3.14
print "pi ≈ {pi:.6f}";          # pi ≈ 3.141593
```

---

## 3. Types de données

Flemme est **dynamiquement typé** : une variable peut changer de type au cours de l'exécution. Il existe quatre types de valeurs :

| Type | Exemples | Description |
|---|---|---|
| Nombre | `42`, `3.14`, `-7`, `0.001` | Toujours stocké en virgule flottante double précision |
| Chaîne | `"bonjour"`, `"résultat : 0"` | Séquence de caractères entre guillemets doubles |
| Tableau | `[1, 2, 3]`, `[]` | Liste ordonnée d'éléments (de types quelconques) |
| Objet natif | `Timer()`, `Stats()` | Instance d'une classe C++ exposée via un plugin |

Il n'existe pas de type booléen dédié : `0` est faux, toute autre valeur numérique est vraie. Les constantes `true` et `false` sont des alias de `1` et `0`. Par conséquent, `type(true)` retourne `"nombre"`.

La fonction `type(x)` retourne le nom du type sous forme de chaîne :

```flemme
print "{type(42)}";      # nombre
print "{type(true)}";    # nombre  (true == 1)
let s = "abc";
print "{type(s)}";       # chaine
let t = [1, 2];
print "{type(t)}";       # tableau
let obj = Stats();
print "{type(obj)}";     # objet
```

### Déclaration de variable : `let`

Toute variable doit être déclarée avec `let` avant utilisation. La portée est celle du bloc englobant (lexicale).

```flemme
let x = 10;
let message = "bonjour";
let valeurs = [1, 2, 3];
let actif = true;      # équivalent à let actif = 1
```

---

## 4. Opérateurs

### 4.1 Arithmétiques

| Opérateur | Rôle | Exemple |
|---|---|---|
| `+` | Addition (nombres) ou concaténation (chaînes) | `3 + 4` → `7` |
| `-` | Soustraction | `10 - 3` → `7` |
| `*` | Multiplication | `2 * 5` → `10` |
| `/` | Division réelle | `7 / 2` → `3.5` |
| `//` | Division entière (plancher) | `7 // 2` → `3` |
| `%` | Modulo | `10 % 3` → `1` |
| `^` | Puissance (associatif à droite) | `2 ^ 10` → `1024` |

La division entière `//` suit la convention du plancher : `-7 // 2` donne `-4` (et non `-3`). L'opérateur puissance est associatif à droite, donc `2 ^ 3 ^ 2` est évalué comme `2 ^ (3 ^ 2)` = `512`.

### 4.2 Moins unaire

```flemme
let y = -x;      # opposé de x
```

### 4.3 Comparaison

Les opérateurs de comparaison fonctionnent sur les nombres **et** sur les chaînes (ordre lexicographique). Ils retournent `1` (vrai) ou `0` (faux).

| Opérateur | Signification |
|---|---|
| `==` | Égal |
| `!=` | Différent |
| `<` | Strictement inférieur |
| `>` | Strictement supérieur |
| `<=` | Inférieur ou égal |
| `>=` | Supérieur ou égal |

```flemme
let ok    = 5 > 3;            # ok = 1
let meme  = "abc" == "abc";   # meme = 1
let ordre = "bac" < "bca";    # ordre = 1 (comparaison lexicographique)
```

### 4.4 Logiques

| Opérateur | Signification | Évaluation |
|---|---|---|
| `&&` | ET logique | court-circuit : évalue le second opérande seulement si le premier est vrai |
| `\|\|` | OU logique | court-circuit : évalue le second opérande seulement si le premier est faux |
| `!` | NON logique (unaire) | |

```flemme
if (x > 0 && x < 100) { ... }
if (erreur || timeout)  { ... }
let non_vide = !vide;
```

### 4.5 Affectation simple et composée

```flemme
x = x + 1;    # affectation simple
x += 1;       # addition
x -= 5;       # soustraction
x *= 2;       # multiplication
x /= 4;       # division
```

### 4.6 Incrémentation / décrémentation postfixe

```flemme
i++;    # équivalent à i = i + 1
i--;    # équivalent à i = i - 1
```

### 4.7 Priorité des opérateurs

De la plus basse à la plus haute :

```
||  →  &&  →  == != < > <= >=  →  + -  →  * / % //  →  ^  →  ! - (unaires)
```

---

## 5. Structures de contrôle

### 5.1 Condition : `if` / `else if` / `else`

```flemme
if (condition) {
  # branche vraie
} else if (autre_condition) {
  # branche intermédiaire
} else {
  # branche par défaut
}
```

Les accolades sont **obligatoires**, même pour un bloc d'une seule instruction.

```flemme
let note = 14;
if (note >= 16) {
  print "Tres bien";
} else if (note >= 12) {
  print "Bien";
} else if (note >= 10) {
  print "Passable";
} else {
  print "Insuffisant";
}
```

### 5.2 Boucle `while`

```flemme
while (condition) {
  # corps
}
```

```flemme
let n = 10;
let somme = 0;
while (n > 0) {
  somme += n;
  n--;
}
print "Somme = {somme}";   # 55
```

### 5.3 Boucle `for` (style C)

La boucle `for` reprend la syntaxe du C avec les trois clauses : initialisation, condition, incrément.

```flemme
for (let i = 0; i < n; i++) {
  # corps
}
```

L'initialisation peut déclarer une variable locale à la boucle avec `let`, ou réaffecter une variable existante. L'incrément est une expression quelconque.

```flemme
# Parcours croissant
for (let k = 1; k <= 10; k++) {
  print "{k}^2 = {k * k}";
}

# Parcours décroissant
for (let i = 10; i > 0; i--) {
  print_raw(i);
  print_raw(" ");
}
print "";

# Pas quelconque
for (let i = 0; i <= 20; i += 5) {
  print_raw(i);
  print_raw(" ");   # 0 5 10 15 20
}
print "";
```

### 5.4 Boucle `for...in` (itération sur tableau ou `range`)

```flemme
for (let element in tableau) {
  # corps — element prend successivement chaque valeur du tableau
}
```

```flemme
let notes = [12, 15, 8, 17, 10];
let total = 0;
for (let n in notes) {
  total += n;
}
print "Moyenne : {total / len(notes):.1f}";
```

Utilisée avec `range(n)`, la boucle itère sur `[0, 1, ..., n-1]` :

```flemme
for (let i in range(5)) {
  print_raw(i);
  print_raw(" ");   # 0 1 2 3 4
}
print "";
```

### 5.5 `break` et `continue`

- `break;` interrompt immédiatement la boucle englobante (`while`, `for`, `for...in`).
- `continue;` passe directement à l'itération suivante.

```flemme
for (let i = 0; i < 20; i++) {
  if (i % 2 == 0) { continue; }   # saute les pairs
  if (i > 9)      { break; }      # arrête au-delà de 9
  print_raw(i);
  print_raw(" ");                  # affiche : 1 3 5 7 9
}
print "";
```

---

## 6. Fonctions

### 6.1 Déclaration : `fn`

```flemme
fn nom(param1, param2) {
  # corps
  return valeur;
}
```

Une fonction sans `return` explicite retourne une valeur vide (`void`). Les paramètres sont passés **par valeur** pour les nombres et les chaînes. Les tableaux partagent leur contenu (passage par référence interne) — une modification dans la fonction est visible à l'extérieur.

```flemme
fn max(a, b) {
  if (a >= b) { return a; }
  return b;
}

print "max(3, 7) = {max(3, 7)}";   # 7
```

### 6.2 Récursivité

```flemme
fn factorielle(n) {
  if (n <= 1) { return 1; }
  return n * factorielle(n - 1);
}

print "10! = {factorielle(10)}";   # 3628800
```

### 6.3 Portée et closures

Les fonctions capturent l'environnement dans lequel elles sont définies (fermeture lexicale). Une fonction peut lire et modifier les variables du bloc englobant au moment de sa définition.

```flemme
let compteur = 0;

fn incrementer() {
  compteur += 1;
}

incrementer();
incrementer();
print "compteur = {compteur}";   # 2
```

### 6.4 Ordre de déclaration

Les fonctions utilisateur sont enregistrées lors de l'exécution de leur déclaration `fn`. Elles doivent donc être déclarées avant d'être appelées (sauf si la déclaration précède l'appel dans le flux d'exécution).

---

## 7. Tableaux

### 7.1 Création

```flemme
let t = [10, 20, 30, 40];    # tableau de 4 éléments
let vide = [];                # tableau vide
let mixte = [1, "deux", 3];  # types hétérogènes autorisés
```

### 7.2 Accès et modification par index

L'indexation commence à `0`. Un accès hors limites lève une erreur d'exécution.

```flemme
let t = [10, 20, 30];
let premier = t[0];   # 10
t[1] = 99;
print "t = {t}";      # [10, 99, 30]
```

### 7.3 Taille

```flemme
let n = len(t);      # fonction
let n2 = t.length;   # propriété équivalente
```

### 7.4 Ajout et retrait : `push` / `pop`

```flemme
push(t, 50);         # ajoute 50 en fin de t (modification en place)
let v = pop(t);      # retire et retourne le dernier élément
```

```flemme
let pile = [];
push(pile, "a");
push(pile, "b");
push(pile, "c");
let x = pop(pile);   # x = "c", pile = ["a", "b"]
```

### 7.5 Génération : `range`

`range(n)` retourne le tableau `[0, 1, 2, ..., n-1]`.

```flemme
let t = range(5);    # [0, 1, 2, 3, 4]

for (let i in range(5)) {
  let v = i * i;
  print_raw(v);
  print_raw(" ");   # 0 1 4 9 16
}
print "";
```

### 7.6 Tableaux 2D

Un tableau 2D est un tableau de tableaux. L'accès se fait avec une double indexation.

```flemme
let mat = [[1, 2, 3],
           [4, 5, 6],
           [7, 8, 9]];

print "mat[1][2] = {mat[1][2]}";   # 6

# Parcours
for (let i in range(3)) {
  for (let j in range(3)) {
    print_raw(mat[i][j]);
    print_raw(" ");
  }
  print "";
}
```

### 7.7 Affichage

Un tableau affiché donne sa représentation entre crochets avec des virgules.

```flemme
let t = [1, 4, 9, 16];
print "carres : {t}";   # carres : [1, 4, 9, 16]
```

---

## 8. Chaînes de caractères

### 8.1 Littéraux

Les chaînes sont délimitées par des guillemets doubles `"`. Le guillemet double interne est échappé avec `\"`.

```flemme
let s = "bonjour le monde";
let cite = "il a dit \"bonjour\"";
```

### 8.2 Concaténation

L'opérateur `+` concatène deux chaînes.

```flemme
let nom = "Alice";
let salut = "Bonjour, " + nom + " !";
```

### 8.3 Longueur

```flemme
let n = len("algorithme");   # 10
```

### 8.4 Comparaison

Les opérateurs `==`, `!=`, `<`, `>`, `<=`, `>=` comparent les chaînes selon l'ordre lexicographique.

```flemme
if ("abc" < "abd") { print "abc est avant abd"; }
```

### 8.5 Conversion

```flemme
let s = str(42);       # "42"
let s2 = str(3.14);    # "3.14"
let n = num("123");    # 123
let n2 = num("4.5");   # 4.5
```

`num` lève une erreur si la chaîne n'est pas un nombre valide.

### 8.6 Fonctions de manipulation

| Fonction | Description | Exemple |
|---|---|---|
| `upper(s)` | Met en majuscules | `upper("abc")` → `"ABC"` |
| `lower(s)` | Met en minuscules | `lower("ABC")` → `"abc"` |
| `trim(s)` | Supprime les espaces en début et fin | `trim("  ok  ")` → `"ok"` |
| `split(s, sep)` | Découpe `s` sur `sep`, retourne un tableau | `split("a,b,c", ",")` → `["a","b","c"]` |
| `find(s, sub)` | Position de `sub` dans `s`, ou `-1` | `find("bonjour", "jour")` → `3` |
| `replace(s, ancien, nouveau)` | Remplace toutes les occurrences | `replace("aabaa", "a", "x")` → `"xxbxx"` |

```flemme
let phrase = "  Bonjour le Monde  ";
let t = trim(phrase);
let mots = split(t, " ");
print "mots : {mots}";                        # ["Bonjour", "le", "Monde"]
print "nb mots : {len(mots)}";                # 3
print "maj : {upper(t)}";                     # BONJOUR LE MONDE
print "pos Monde : {find(t, \"Monde\")}";     # 9
print "remplace : {replace(t, \"Monde\", \"Flemme\")}";
```

Si `sep` est une chaîne vide, `split` découpe caractère par caractère.

---

## 9. Bibliothèque standard

### 9.1 Entrée / sortie

| Fonction / instruction | Description |
|---|---|
| `print "..."` | Affiche une chaîne avec interpolation `{expr}`, puis retour à la ligne |
| `print_raw(v1, v2, ...)` | Affiche un ou plusieurs arguments **sans** retour à la ligne |
| `input()` | Lit une ligne depuis l'entrée standard, retourne une chaîne |
| `input("prompt")` | Affiche d'abord le prompt, puis lit la ligne |

```flemme
let prenom = input("Votre prenom : ");
print "Bonjour, {prenom} !";
```

> `input()` n'est pas disponible dans le Web IDE (remplacé par une erreur explicite).

### 9.2 Conversion de types

| Fonction | Description |
|---|---|
| `num(s)` | Convertit la chaîne `s` en nombre (erreur si impossible) |
| `str(x)` | Convertit `x` en sa représentation textuelle |
| `type(x)` | Retourne le type de `x` : `"nombre"`, `"chaine"`, `"tableau"`, `"objet"`, `"void"` |

```flemme
print "{type(42)}";       # nombre
print "{type(\"abc\")}";  # chaine
print "{type([1,2])}";    # tableau
```

### 9.3 Mathématiques

| Fonction | Description |
|---|---|
| `sin(x)` | Sinus (radians) |
| `cos(x)` | Cosinus (radians) |
| `tan(x)` | Tangente (radians) |
| `sqrt(x)` | Racine carrée |
| `exp(x)` | Exponentielle `e^x` |
| `ln(x)` | Logarithme naturel (`x > 0`) |
| `log10(x)` | Logarithme en base 10 (`x > 0`) |
| `abs(x)` | Valeur absolue |
| `floor(x)` | Partie entière inférieure |
| `ceil(x)` | Partie entière supérieure |
| `round(x)` | Arrondi à l'entier le plus proche |

```flemme
let pi = 3.14159265358979;
print "sin(pi/2) = {sin(pi / 2):.6f}";   # 1.000000
print "sqrt(2)   = {sqrt(2):.6f}";       # 1.414214
print "floor(3.7) = {floor(3.7)}";       # 3
print "ceil(3.2)  = {ceil(3.2)}";        # 4
```

### 9.4 Tableaux

| Fonction | Description |
|---|---|
| `len(t)` | Nombre d'éléments du tableau (ou longueur d'une chaîne) |
| `range(n)` | Retourne le tableau `[0, 1, ..., n-1]` |
| `push(t, v)` | Ajoute `v` en fin de `t` (modification en place, retourne `void`) |
| `pop(t)` | Retire et retourne le dernier élément de `t` |

### 9.5 Chaînes

| Fonction | Description |
|---|---|
| `upper(s)` | Majuscules |
| `lower(s)` | Minuscules |
| `trim(s)` | Supprime les espaces en début et fin |
| `split(s, sep)` | Découpe sur `sep`, retourne un tableau de chaînes |
| `find(s, sub)` | Retourne la position (0-indexé) ou `-1` |
| `replace(s, ancien, nouveau)` | Remplace toutes les occurrences |

### 9.6 Débogage et assertions

| Fonction | Description |
|---|---|
| `assert(cond)` | Lève une erreur si `cond` est fausse |
| `assert(cond, msg)` | Idem avec un message d'erreur personnalisé |
| `type(x)` | Retourne le type de `x` sous forme de chaîne |

`assert` considère comme faux : `0` et la chaîne vide `""`. Tout autre nombre ou chaîne non vide est vrai.

```flemme
let t = [1, 2, 3];
assert(len(t) == 3, "le tableau doit avoir 3 elements");
assert(t[0] == 1);
```

`assert` est particulièrement utile dans les scripts de test (voir le répertoire `tests/`).

---

## 10. Plugins C++

Flemme intègre un système de plugins permettant d'exposer des objets et fonctions C++ au script. Cela permet de piloter un code de simulation depuis un script sans recompiler l'interpréteur.

### 10.1 Plugins fournis

#### Timer — chronomètre

```flemme
let t = Timer();

# ... code à mesurer ...

let ms = t.elapsed() * 1000;
print "Temps : {ms:.2f} ms";

t.reset();   # remet à zéro
```

| Méthode | Description |
|---|---|
| `elapsed()` | Temps écoulé depuis la création ou le dernier `reset()`, en secondes |
| `reset()` | Remet le chronomètre à zéro |

#### Stats — statistiques descriptives

```flemme
let s = Stats();

for (let x in donnees) { s.add(x); }

print "n          = {s.count}";
print "moyenne    = {s.mean():.4f}";
print "ecart-type = {s.stddev():.4f}";
print "min / max  = {s.min()} / {s.max()}";

s.clear();   # réinitialise
```

| Méthode / Getter | Description |
|---|---|
| `add(x)` | Ajoute la valeur `x` |
| `clear()` | Supprime toutes les valeurs |
| `count` *(getter)* | Nombre de valeurs |
| `mean()` | Moyenne |
| `variance()` | Variance (populationnelle) |
| `stddev()` | Écart-type |
| `min()` | Minimum |
| `max()` | Maximum |

Toutes les méthodes sauf `add` et `clear` lèvent une erreur si aucune donnée n'a été ajoutée.

#### MockRockable — simulation DEM factice

Expose une interface identique au simulateur Rockable pour tester des scripts sans la vraie bibliothèque.

```flemme
let sim = Rockable();
sim.setIntegrator("velocityVerlet");
sim.dt   = 0.0001;
sim.tmax = 0.001;

while (sim.t < sim.tmax) {
  sim.velocityVerletStep();
}

print "t final = {sim.t:.6f}";
print "Ec cin  = {sim.getKineticEnergy():.2f}";
```

### 10.2 Désactiver un plugin

| Moment | Commande |
|---|---|
| Compilation (les deux binaires) | `make DISABLE_STATS=1` |
| Compilation (web IDE seulement) | `make WEBIDE_DISABLE_MOCKROCKABLE=1` |
| Exécution | `./flemme --no-plugin stats script.flm` |

Un plugin désactivé à l'exécution lève `Fonction inconnue` si le script essaie de l'utiliser.

### 10.3 Créer son propre plugin

Voir [`BINDING.md`](BINDING.md) pour le guide complet.

---

## 11. Exemples complets

### 11.1 Fibonacci récursif

```flemme
fn fib(n) {
  if (n <= 1) { return n; }
  return fib(n - 1) + fib(n - 2);
}

for (let i in range(10)) {
  print "fib({i}) = {fib(i)}";
}
```

### 11.2 Tri par insertion

```flemme
fn tri_insertion(t) {
  let n = len(t);
  for (let i = 1; i < n; i++) {
    let cle = t[i];
    let j = i - 1;
    while (j >= 0 && t[j] > cle) {
      t[j + 1] = t[j];
      j--;
    }
    t[j + 1] = cle;
  }
}

let donnees = [5, 2, 8, 1, 9, 3];
tri_insertion(donnees);
print "Trie : {donnees}";
```

### 11.3 Intégration numérique (méthode des rectangles)

```flemme
fn f(x) { return x ^ 2; }

let a = 0;
let b = 1;
let n = 1000;
let dx = (b - a) / n;
let somme = 0;

for (let k = 0; k < n; k++) {
  somme += f(a + k * dx) * dx;
}
print "integrale de x^2 sur [0,1] ≈ {somme:.6f}";   # ≈ 0.333333
```

### 11.4 Recherche dichotomique

```flemme
fn dichotomie(t, cible) {
  let bas = 0;
  let haut = len(t) - 1;
  while (bas <= haut) {
    let milieu = (bas + haut) // 2;
    if (t[milieu] == cible) { return milieu; }
    if (t[milieu] < cible)  { bas = milieu + 1; }
    else                    { haut = milieu - 1; }
  }
  return -1;
}

let t = [1, 3, 5, 7, 9, 11, 13, 15];
let pos = dichotomie(t, 7);
print "7 trouve a l'indice {pos}";   # 3
```

### 11.5 Jeu de devinette (avec `input`)

```flemme
let secret = 42;
let essai = 0;
let gagne = false;

while (!gagne) {
  let saisie = input("Devinez le nombre (1-100) : ");
  let n = num(saisie);
  essai++;
  if (n == secret) {
    gagne = true;
    print "Bravo en {essai} essai(s) !";
  } else if (n < secret) {
    print "Trop petit.";
  } else {
    print "Trop grand.";
  }
}
```

### 11.6 Mesure de performance avec Timer et Stats

```flemme
fn is_premier(n) {
  if (n < 2) { return false; }
  for (let i = 2; i * i <= n; i++) {
    if (n % i == 0) { return false; }
  }
  return true;
}

let t = Timer();
let s = Stats();
let nb = 0;

for (let i in range(10000)) {
  if (is_premier(i)) {
    nb += 1;
    s.add(i);
  }
}

let ms = t.elapsed() * 1000;
print "Premiers < 10000 : {nb}";
print "Plus grand        : {s.max()}";
print "Moyenne           : {s.mean():.2f}";
print "Temps             : {ms:.2f} ms";
```

### 11.7 Pilotage d'une simulation (MockRockable)

```flemme
let sim = Rockable();
sim.setVerboseLevel(1);
sim.setIntegrator("velocityVerlet");
sim.dt   = 0.0001;
sim.tmax = 0.001;

let pas = 0;
while (sim.t < sim.tmax) {
  sim.velocityVerletStep();
  pas++;
}

print "Simulation terminee en {pas} pas.";
print "t final = {sim.t:.6f}";
print "Ec cin  = {sim.getKineticEnergy():.4f}";
```

---

## 12. Résumé de la grammaire (BNF simplifié)

```
programme    ::= instruction*
instruction  ::= declaration | affectation | si | tantque | pour | pour_dans
               | brise | continue | retour | impression | bloc | expr_stmt
               | fn_decl
declaration  ::= "let" IDENT "=" expression ";"
affectation  ::= cible ("=" | "+=" | "-=" | "*=" | "/=") expression ";"
              |  IDENT ("++" | "--") ";"
cible        ::= IDENT | expression "." IDENT | expression "[" expression "]"
si           ::= "if" "(" expression ")" bloc ("else" (si | bloc))?
tantque      ::= "while" "(" expression ")" bloc
pour         ::= "for" "(" init ";" expression ";" expression ")" bloc
pour_dans    ::= "for" "(" "let" IDENT "in" expression ")" bloc
impression   ::= "print" CHAINE ";"
fn_decl      ::= "fn" IDENT "(" params? ")" bloc
params       ::= IDENT ("," IDENT)*
retour       ::= "return" expression? ";"
brise        ::= "break" ";"
continue     ::= "continue" ";"
bloc         ::= "{" instruction* "}"

expression   ::= logique_ou
logique_ou   ::= logique_et ("||" logique_et)*
logique_et   ::= comparaison ("&&" comparaison)*
comparaison  ::= addition (("==" | "!=" | "<" | ">" | "<=" | ">=") addition)?
addition     ::= terme (("+" | "-") terme)*
terme        ::= puissance (("*" | "/" | "%" | "//") puissance)*
puissance    ::= unaire ("^" puissance)?          # associatif à droite
unaire       ::= ("!" | "-") unaire | postfixe
postfixe     ::= primaire ("++" | "--" | "[" expression "]"
               | "." IDENT | "(" args? ")")*
primaire     ::= NOMBRE | CHAINE | IDENT | "true" | "false"
               | "[" args? "]" | "(" expression ")"
args         ::= expression ("," expression)*
```
