# Plugin Schema — schémas RDM en SVG

Génère des schémas de **résistance des matériaux** (poutres, appuis, charges, cotes) directement depuis un script `.flm`. Inspiré des conventions de tracé TikZ de `drawingTool.tex`.

## Utilisation rapide

```
let s = Schema();

s.poutre(0, 0, 6, 0, 0.2);
s.appui(0, 0, -1.5708, 0.5);    # -pi/2 = appui en bas
s.articulation(6, 0, -1.5708, 0.5);
s.rouge;
s.chargeRep(0, 0, 6, 1.5);
s.coteH(0, -0.5, 6, -0.8, "L");
s.save("poutre.svg");
```

---

## Référence des méthodes

### Style courant

Toutes les instructions de style s'appliquent aux éléments ajoutés **ensuite** et se réinitialisent avec `s.clear()`.

#### Couleur

| Instruction | Hex       | Apparence        |
|-------------|-----------|------------------|
| `s.blanc`   | `#ffffff` | blanc            |
| `s.gris0`   | `#aaaaaa` | gris clair       |
| `s.gris1`   | `#666666` | gris moyen       |
| `s.noir`    | `#111111` | noir (défaut)    |
| `s.rouge`   | `#cc2222` | rouge            |
| `s.bleu`    | `#2266cc` | bleu             |
| `s.vert`    | `#228833` | vert             |

> `s.gris` reste valide comme alias de `s.gris1`.

#### Vecteur

| Instruction | Effet                                   |
|-------------|-----------------------------------------|
| `s.plein`   | tête triangulaire pleine (défaut)       |
| `s.ouvert`  | tête en V ouvert (style angle-60)       |

#### Ligne pointillée / continue

| Instruction    | Effet                                          |
|----------------|------------------------------------------------|
| `s.pointilles` | active les tirets pour les `ligne()` suivants  |
| `s.continu`    | rétablit les lignes continues (défaut)         |

S'applique uniquement à `s.ligne()`.

#### Alignement texte

| Instruction | Effet                      |
|-------------|----------------------------|
| `s.gauche`  | ancrage à gauche           |
| `s.centre`  | ancrage centré (défaut)    |
| `s.droite`  | ancrage à droite           |

#### Police et style de texte

| Instruction       | Effet                                       |
|-------------------|---------------------------------------------|
| `s.serif`         | police serif (ex. Times)                    |
| `s.sansSerif`     | police sans-serif (défaut)                  |
| `s.mono`          | police monospace                            |
| `s.police("nom")` | police arbitraire (nom CSS valide)          |
| `s.fontSize(px)`  | taille en pixels (0 = défaut 12/14)         |
| `s.gras`          | texte en gras                               |
| `s.italique`      | texte en italique                           |
| `s.normal`        | annule gras et italique                     |

S'applique à `s.texte()` et `s.equation()`.

```
s.serif;
s.gras;
s.fontSize(16);
s.texte(3, 2, "Titre");
s.normal;
s.sansSerif;
s.fontSize(0);   # revient au défaut
```

#### Épaisseur et taille de vecteur

| Méthode               | Effet                                              |
|-----------------------|----------------------------------------------------|
| `s.epaisseur(v)`      | multiplie l'épaisseur de trait par `v` (défaut 1)  |
| `s.tailleVecteur(v)`  | taille de tête de flèche en unités monde (0 = auto)|

#### Taille et orientation par défaut des liaisons

Permet d'éviter de répéter `angle` et `size` dans chaque appel structural :

```
s.taille(0.4);
s.orientation(-1.5708);   # sol en bas
s.appui(0, 0);            # équivalent à s.appui(0, 0, -1.5708, 0.4)
s.articulation(6, 0);
```

| Méthode             | Effet                                         |
|---------------------|-----------------------------------------------|
| `s.taille(v)`       | taille par défaut des liaisons (0 = effacer)  |
| `s.orientation(v)`  | angle par défaut des liaisons en radians      |

---

### Éléments structuraux

#### `s.poutre(x0, y0, x1, y1, ep)`
Trait épais avec extrémités arrondies, d'épaisseur `ep` (en unités monde).

```
s.poutre(0, 0, 6, 0, 0.2);       # poutre horizontale
s.poutre(0, 0, 0, 4, 0.15);      # poteau vertical
```

---

### Appuis

Tous les appuis partagent le même paramètre `angle` :
- **angle** = direction de l'apex vers le sol, en radians
- `-pi/2` (≈ −1.5708) → sol en bas (cas usuel)
- `pi` (≈ 3.14159) → sol à gauche
- `0` → sol à droite

Les paramètres `angle` et `size` sont **optionnels** si `s.orientation()` et `s.taille()` ont été fixés au préalable.

#### `s.appui(x, y[, angle, size])`
Appui à rouleaux : triangle + 3 cercles + ligne de sol + hachures.

#### `s.articulation(x, y[, angle, size])`
Articulation encastrée : grand cercle + ligne de sol + hachures.

#### `s.encastrement(x, y[, angle, size])`
Encastrement : talon court + ligne de sol + hachures.

#### `s.appuiContinu(x, y[, angle, size])`
Appui continu : triangle + hachures directement (sans rouleaux ni ligne de sol).

#### `s.articul(x, y[, size])`
Articulation interne (nœud de charnière) : cercle seul, sans sol.

```
s.appui(0, 0, -1.5708, 0.5);
s.articulation(6, 0, -1.5708, 0.5);
s.encastrement(0, 0, 3.14159, 0.5);
s.appuiContinu(3, 0, -1.5708, 0.4);
s.articul(3, 4, 0.2);
```

---

### Charges

#### `s.force(x0, y0, x1, y1)`
Flèche de `(x0,y0)` vers `(x1,y1)`. Le style de tête (`s.plein` / `s.ouvert`) s'applique.

#### `s.moment(x, y, rayon, angleDeb, angleFin)`
Arc circulaire avec flèche ouverte à l'extrémité. Sens trigonométrique si `angleFin > angleDeb`.
- `angleDeb`, `angleFin` en radians

```
s.moment(0, 4, 0.6, -1.5708, 1.5708);   # demi-cercle CCW
s.moment(5, 0, 0.7, 3.14159, 0.0);      # demi-cercle CW
```

#### `s.chargeRep(x0, ytip, x1, ybase)`
Charge répartie verticale : ligne de base à `y = ybase`, 5 flèches jusqu'à `y = ytip`. Le style de tête s'applique.

```
s.chargeRep(0, 0, 6, 1.5);    # charge descendante (ybase > ytip)
s.chargeRep(0, 2, 4, 0);      # charge ascendante
```

---

### Cotes (dimensions)

#### `s.cote(x1, y1, x2, y2, "label")`
Double flèche `↔` entre deux points quelconques. Label centré et décalé perpendiculairement.

#### `s.coteH(x1, y1, x2, y2, "label")`
Cote horizontale : lignes d'extension verticales de `y1` à `y2`, double flèche horizontale à `y2`.
- `y2 > y1` : cote au-dessus ; `y2 < y1` : cote en dessous

```
s.coteH(0, -0.5, 6, -0.8, "L");     # portée L sous la poutre
```

#### `s.coteV(x1, y1, x2, y2, "label")`
Cote verticale : lignes d'extension horizontales de `x1` à `x2`, double flèche verticale à `x2`.
- `x2 > x1` : cote à droite ; `x2 < x1` : cote à gauche

```
s.coteV(-1, 0, -1.5, 4, "H");       # hauteur H à gauche
```

#### `s.demicoteH(x1, y1, x2, y2, "label")`
Cote horizontale avec tiret à l'origine et flèche simple de `x1` vers `x2` à hauteur `y2`.

#### `s.demicoteV(x1, y1, x2, y2, "label")`
Cote verticale avec tiret à l'origine et flèche simple de `y1` vers `y2` à abscisse `x2`.

```
s.demicoteV(4.5, -1, 6.5, 0, "YG");   # depuis y=-1 vers y=0
s.demicoteH(0, 0, 4, 0.5, "a");
```

---

### Repère d'axes

#### `s.repere(x, y[, angle, size])`
Trièdre 2D : flèches X et Y + arc indiquant le sens positif (CCW).
- `angle` : rotation du repère (radians, 0 = axes alignés sur X/Y monde)
- `size` : longueur des flèches en unités monde

```
s.repere(7, -0.5, 0.0, 0.8);
```

#### `s.repereZY(x, y[, size])`
Axes Z (gauche) et Y (haut) sans arc, convention Navier pour les sections droites.

```
s.repereZY(0, 0, 1.5);
```

---

### Profilé UPE

#### `s.upe(x, y[, angle, size])`
Contour d'un profilé UPE (canal en C), coordonnées locales 0..2 × 0..1.
- `angle = -pi/2` → orientation standard (semelles en haut/bas)
- `angle = 0` → profil tourné à 90°

```
s.upe(-1, -0.3, -1.5708, 1);
```

---

### Primitives graphiques

#### `s.ligne(x0, y0, x1, y1)`
Segment de la couleur courante. Respecte `s.pointilles` / `s.continu`.

#### `s.cercle(x, y, r)`
Cercle creux de rayon `r` (unités monde), remplissage blanc.

#### `s.disque(x, y, r)`
Disque plein de rayon `r` de la couleur courante.

#### `s.rect(x0, y0, x1, y1)`
Rectangle contour seulement (couleur courante, sans remplissage).

#### `s.boite(x0, y0, x1, y1)`
Rectangle rempli de la couleur courante, contour noir.

```
s.gris0;
s.boite(0, 0, 8, 2);     # bloc gris opaque
s.bleu;
s.boite(0, 2, 8, 4);     # bloc bleu opaque
s.gris1;
s.rect(8.5, 2.7, 9.5, 3.3);   # capteur (contour seul)
```

#### `s.solH(x0, y, x1)`
Sol haché horizontal : ligne + hachures à −45° sous la ligne, sur toute la largeur `x0`..`x1`.

```
s.gris0;
s.solH(-0.3, 0, 8.3);
```

#### `s.texte(x, y, "texte")`
Texte en `(x, y)`. Respecte la couleur courante, `s.gauche/centre/droite`, `s.serif/sansSerif/mono`, `s.fontSize()`, `s.gras`, `s.italique`.

```
s.serif; s.gras; s.fontSize(16);
s.texte(3, 2, "Titre");
s.normal; s.sansSerif; s.fontSize(0);
```

#### `s.equation(x, y, "latex")`
Formule mathématique en syntaxe LaTeX, rendue en SVG nativement (pas de dépendance externe). Respecte la couleur, l'alignement, la police et la taille courantes.

**Sous-ensemble LaTeX supporté :**

| Syntaxe          | Rendu    | Description              |
|------------------|----------|--------------------------|
| `\alpha` … `\omega` | α … ω | Lettres grecques minuscules |
| `\Gamma` … `\Omega` | Γ … Ω | Lettres grecques majuscules |
| `_{x}` ou `_x`   | subscript | Indice                   |
| `^{x}` ou `^x`   | superscript | Exposant               |
| `\frac{a}{b}`    | a/b      | Fraction (barre + empilement) |
| `\sqrt{x}`       | √x       | Racine carrée (symbole √) |
| `\times`         | ×        | Multiplication           |
| `\cdot`          | ⋅        | Produit scalaire         |
| `\pm`            | ±        | Plus ou moins            |
| `\leq` `\geq` `\neq` | ≤ ≥ ≠ | Comparaisons          |
| `\approx`        | ≈        | Approximation            |
| `\infty`         | ∞        | Infini                   |
| `\partial`       | ∂        | Dérivée partielle        |
| `\rightarrow` / `\to` | →  | Flèche droite            |
| `\star`          | ★        | Étoile                   |
| `\left(` `\right)` | ( )   | Délimiteurs              |

> **Note syntaxe flemme :** un seul `\` dans les chaînes (pas `\\`).
> `"\sigma_x"` ✓ — `"\\sigma_x"` ✗

```
s.noir;
s.equation(0, 4, "\sigma_x = \frac{N}{A}");
s.equation(0, 3, "I_z = \frac{bh^3}{12}");
s.equation(0, 2, "\sigma_{eq} = \sqrt{\sigma_x^2 + 3\tau_{xy}^2}");

# centrée, en rouge, police serif
s.rouge; s.centre; s.serif; s.fontSize(14);
s.equation(0, 0, "N = (m^{\star} + m^{+} - m^{-}) \cdot g");
```

Chaque lettre isolée est automatiquement rendue en italique (convention mathématique). Les nombres et les opérateurs restent droits.

---

### Marqueurs

#### `s.point(x, y)`
Cercle orange creux.

#### `s.croix(x, y)`
Croix orange (×).

#### `s.simplecroix(x, y, size)`
Croix (×) de la couleur courante, de demi-taille `size` en unités monde.

---

### Contrôle

#### `s.save("fichier.svg")`
Génère le fichier SVG (700 × 400 px, ratio 1:1 automatique, fond blanc).

#### `s.clear()`
Efface tous les éléments et réinitialise couleur, style et police aux valeurs par défaut.

---

## Paramètre `angle` — aide-mémoire

```
angle = -pi/2  ≈ -1.5708  →  sol en bas    (poutre horizontale usuelle)
angle =  pi/2  ≈  1.5708  →  sol en haut   (poutre suspendue)
angle =  0                →  sol à droite  (poteau, appui latéral gauche)
angle =  pi    ≈  3.14159 →  sol à gauche  (encastrement gauche)
```

---

## Fichiers

| Fichier                   | Rôle                                               |
|---------------------------|----------------------------------------------------|
| `Schema.hpp`              | Interface C++ de la classe `Schema`                |
| `Schema.cpp`              | Rendu SVG + renderer LaTeX (namespace `MR`)        |
| `schema_plugin.hpp`       | Liaison flemme ↔ `Schema`                          |
| `Makefile`                | Compile `libflemme-schema.a`                       |
| `demo_schema.flm`         | Démonstration liaisons, charges, cotes, styles     |
| `demo_dispositif.flm`     | Schéma de principe dispositif de frottement        |
| `test_schema.flm`         | Script de démonstration historique                 |
| `exo1.flm` … `exo5.flm`  | DS MS2 2026                                        |
| `drawingTool.tex`         | Conventions TikZ de référence                      |

## Fonctionnalités prévues (non encore implémentées)

- `forceRep` inclinée (charge répartie sur poutre oblique)
- `coteA` (cote angulaire)
- Export TikZ (`s.saveTikz(...)`)
- Renderer LaTeX : fractions imbriquées, `\sum`, `\int`
