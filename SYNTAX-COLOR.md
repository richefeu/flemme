# Coloration syntaxique pour Flemme

Deux grammaires sont disponibles dans ce dépôt :

| Éditeur | Répertoire | Extension reconnue |
|---|---|---|
| TextMate 2 / Sublime Text | `flemme.tmbundle/` | `.flm` |
| Vim 8+ / Neovim | `vim/` | `.flm` |

---

## TextMate 2

### Installation

La méthode recommandée est un **lien symbolique** : toute modification du fichier de grammaire dans le dépôt est immédiatement prise en compte sans réinstallation.

```bash
mkdir -p ~/Library/Application\ Support/TextMate/Bundles

ln -s /chemin/vers/flemme/flemme.tmbundle \
      ~/Library/Application\ Support/TextMate/Bundles/flemme.tmbundle
```

Remplacez `/chemin/vers/flemme/` par le chemin réel vers ce dépôt.

Ensuite, rechargez les bundles dans TextMate 2 :
- soit en **relançant l'application**,
- soit via le menu **Bundles → Edit Bundles…** (fermer la fenêtre force un rechargement).

### Vérification

Ouvrez n'importe quel fichier `.flm`. Le sélecteur de langage en bas à droite de la fenêtre doit afficher **Flemme**. Si ce n'est pas le cas, cliquez dessus et sélectionnez Flemme manuellement — TextMate proposera de mémoriser ce choix pour tous les fichiers `.flm`.

### Désinstallation

```bash
rm ~/Library/Application\ Support/TextMate/Bundles/flemme.tmbundle
```

---

## Vim 8+ et Neovim

### Option 1 — Packages natifs (recommandée, sans plugin manager)

Vim 8 et Neovim intègrent un système de packages qui charge automatiquement tout ce qui se trouve dans `pack/*/start/`. Un lien symbolique suffit :

```bash
# Vim
mkdir -p ~/.vim/pack/flemme/start/flemme
ln -s /chemin/vers/flemme/vim/syntax   ~/.vim/pack/flemme/start/flemme/syntax
ln -s /chemin/vers/flemme/vim/ftdetect ~/.vim/pack/flemme/start/flemme/ftdetect

# Neovim
mkdir -p ~/.config/nvim/pack/flemme/start/flemme
ln -s /chemin/vers/flemme/vim/syntax   ~/.config/nvim/pack/flemme/start/flemme/syntax
ln -s /chemin/vers/flemme/vim/ftdetect ~/.config/nvim/pack/flemme/start/flemme/ftdetect
```

Aucune ligne à ajouter dans `.vimrc` ou `init.vim`. Relancer Vim suffit.

### Option 2 — vim-plug

Dans `.vimrc` (ou `init.vim` pour Neovim) :

```vim
Plug '/chemin/vers/flemme/vim'
```

Puis dans Vim : `:PlugInstall`.

### Option 3 — Copie manuelle

Si vous ne souhaitez pas utiliser de lien symbolique :

```bash
# Vim
cp /chemin/vers/flemme/vim/syntax/flemme.vim   ~/.vim/syntax/
cp /chemin/vers/flemme/vim/ftdetect/flemme.vim ~/.vim/ftdetect/

# Neovim
cp /chemin/vers/flemme/vim/syntax/flemme.vim   ~/.config/nvim/syntax/
cp /chemin/vers/flemme/vim/ftdetect/flemme.vim ~/.config/nvim/ftdetect/
```

Avec cette méthode, toute mise à jour de la grammaire devra être recopiée manuellement.

### Vérification

Ouvrez un fichier `.flm` dans Vim :

```bash
vim test_priority1.flm
```

- `:set filetype?` doit répondre `filetype=flemme`
- `:syntax list` liste tous les groupes de syntaxe actifs

Pour identifier le groupe appliqué à un token spécifique, placez le curseur dessus et tapez :

```vim
:echo synIDattr(synID(line('.'),col('.'),1),'name')
```

### Désinstallation

Supprimez le lien symbolique (option 1) ou les fichiers copiés (option 3) :

```bash
# Option 1 — lien symbolique
rm -r ~/.vim/pack/flemme/start/flemme

# Option 3 — copie manuelle
rm ~/.vim/syntax/flemme.vim ~/.vim/ftdetect/flemme.vim
```

---

## Structure des fichiers de grammaire

```
flemme.tmbundle/
├── info.plist                    # métadonnées du bundle (nom, UUID)
└── Syntaxes/
    └── flemme.tmLanguage         # grammaire plist XML (TextMate / Sublime Text)

vim/
├── syntax/
│   └── flemme.vim                # règles de coloration (Vim / Neovim)
└── ftdetect/
    └── flemme.vim                # association automatique *.flm → filetype=flemme
```
