# Makefile — flemme : interpréteur + plugins
#
# Utilisation :
#   make                           compile les plugins actifs puis flemme
#   make DISABLE_TIMER=1           compile sans le plugin Timer
#   make DISABLE_STATS=1           compile sans le plugin Stats
#   make DISABLE_MOCKROCKABLE=1    compile sans le plugin MockRockable
#   make DISABLE_PLOTSVG=1         compile sans le plugin PlotSVG
#   make plugins                   compile uniquement les .a des plugins actifs
#   make test                      lance ./flemme test.flm
#   make test SCRIPT=foo.flm       lance ./flemme foo.flm
#   make web                       build + lance le Web IDE
#   make clean                     supprime tous les fichiers générés

# ------------------------------------------------
# Outils et fonctions
# ------------------------------------------------
CXX      := g++
AR       := ar
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

# Fonction helper : convertir en majuscules
upper = $(shell echo $(1) | tr a-z A-Z)

# ------------------------------------------------
# Interpréteur (headers-only sauf main.cpp)
# ------------------------------------------------
INTERP_HEADERS := lexer.hpp parser.hpp interpreter.hpp plugins.hpp
INTERP_SRC     := main.cpp
INTERP_OBJ     := main.o
BIN            := flemme

# ------------------------------------------------
# Plugins — liste et config généralisée
# ------------------------------------------------
# Pour ajouter un plugin : ajouter son nom à PLUGINS ci-dessous,
# puis créer plugins/<name>/ avec Makefile et <name>_plugin.hpp
PLUGINS := timer stats mockrockable plotsvg schema

# Désactiver un plugin avec DISABLE_<NAME>=1 (majuscules, ex: DISABLE_TIMER=1)
$(foreach P,$(PLUGINS),$(eval DISABLE_$(call upper,$(P)) ?= 0))

# Générer les chemins et libs pour chaque plugin
$(foreach P,$(PLUGINS),\
  $(eval PLUGIN_$(call upper,$(P))_DIR := plugins/$(P))\
  $(eval PLUGIN_$(call upper,$(P))_LIB := plugins/$(P)/libflemme-$(P).a)\
)

# Générer les flags (libs, includes, defines) pour les plugins actifs
PLUGIN_LIBS :=
PLUGIN_INCS :=
PLUGIN_DEFS :=

$(foreach P,$(PLUGINS),\
  $(if $(filter 0,$(DISABLE_$(call upper,$(P)))),\
    $(eval PLUGIN_LIBS += $(PLUGIN_$(call upper,$(P))_LIB))\
    $(eval PLUGIN_INCS += -I$(PLUGIN_$(call upper,$(P))_DIR))\
    $(eval PLUGIN_DEFS += -DFLEMME_PLUGIN_$(call upper,$(P)))\
  )\
)

# ------------------------------------------------
# Flags finaux (interpréteur)
# ------------------------------------------------
ALL_CXXFLAGS := $(CXXFLAGS) $(PLUGIN_INCS) $(PLUGIN_DEFS)

# ------------------------------------------------
# Plugins désactivés pour flemme-web seulement
# Indépendants des DISABLE_* (qui agissent sur les deux binaires).
# Un plugin WEBIDE_DISABLE peut rester compilé/lié mais ne sera pas
# enregistré par le serveur web. Utiliser les majuscules: WEBIDE_DISABLE_TIMER=1
# ------------------------------------------------
# Valeurs par défaut (mockrockable est désactivé dans le web par défaut)
$(foreach P,$(PLUGINS),\
  $(eval WEBIDE_DISABLE_$(call upper,$(P)) ?= $(if $(call upper,$(P)),0))\
)
# Oui, mockrockable=1 par défaut (exception)
WEBIDE_DISABLE_MOCKROCKABLE := 1

WEBIDE_PLUGIN_DEFS :=
$(foreach P,$(PLUGINS),\
  $(if $(filter 1,$(WEBIDE_DISABLE_$(call upper,$(P)))),\
    $(eval WEBIDE_PLUGIN_DEFS += -DWEBIDE_NO_PLUGIN_$(call upper,$(P)))\
  )\
)

# ------------------------------------------------
# Web IDE (flemme-web)
# ------------------------------------------------
VENDOR_DIR  := vendor
HTTPLIB_H   := $(VENDOR_DIR)/httplib.h
HTTPLIB_URL ?= https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h

WEBIDE_BIN  := flemme-web
WEBIDE_SRC  := webide.cpp
WEBIDE_OBJ  := webide.o

UNAME_S := $(shell uname -s 2>/dev/null || echo Unknown)
ifeq ($(UNAME_S),Linux)
  WEBIDE_LDFLAGS := -lpthread
else
  WEBIDE_LDFLAGS :=
endif

# ------------------------------------------------
# Cibles
# ------------------------------------------------
.PHONY: all plugins test web clean help

all: $(BIN)

## Compile les .a des plugins actifs
plugins: $(PLUGIN_LIBS)

## Lien final
$(BIN): $(INTERP_OBJ) $(PLUGIN_LIBS)
	$(CXX) $(ALL_CXXFLAGS) $^ -o $@
	@echo "→ $(BIN) compilé"

## Compilation de main.cpp
PLUGIN_HEADERS := $(wildcard plugins/*/*.hpp)

$(INTERP_OBJ): $(INTERP_SRC) $(INTERP_HEADERS) $(PLUGIN_HEADERS)
	$(CXX) $(ALL_CXXFLAGS) -c $(INTERP_SRC) -o $@

## Bibliothèques de plugins — cibles générées dynamiquement
# Pour chaque plugin, générer une cible explicite <lib>: <dir>/Makefile
define PLUGIN_BUILD_RULE
$(PLUGIN_$(call upper,$(1))_LIB):
	$$(MAKE) -C plugins/$(1)
endef

$(foreach P,$(PLUGINS),$(eval $(call PLUGIN_BUILD_RULE,$(P))))

## Lancer un script de test
SCRIPT ?= test.flm
test: $(BIN)
	./$(BIN) $(SCRIPT)

## Web IDE ─────────────────────────────────────────────────────────────────────

$(HTTPLIB_H):
	@mkdir -p $(VENDOR_DIR)
	@echo "→ Téléchargement de cpp-httplib…"
	curl -fsSL "$(HTTPLIB_URL)" -o $@
	@echo "→ $(HTTPLIB_H) prêt"

$(WEBIDE_OBJ): $(WEBIDE_SRC) $(INTERP_HEADERS) $(HTTPLIB_H)
	$(CXX) $(ALL_CXXFLAGS) $(WEBIDE_PLUGIN_DEFS) -I$(VENDOR_DIR) -c $(WEBIDE_SRC) -o $@

$(WEBIDE_BIN): $(WEBIDE_OBJ) $(PLUGIN_LIBS)
	$(CXX) $(CXXFLAGS) $^ $(WEBIDE_LDFLAGS) -o $@
	@echo "→ $(WEBIDE_BIN) compilé"

web: plugins $(WEBIDE_BIN)
	./$(WEBIDE_BIN)

## Nettoyage ───────────────────────────────────────────────────────────────────
clean:
	rm -f $(INTERP_OBJ) $(BIN) $(WEBIDE_OBJ) $(WEBIDE_BIN)
	@for plugin in $(PLUGINS); do $(MAKE) -C plugins/$$plugin clean; done

## Aide ────────────────────────────────────────────────────────────────────────
help:
	@echo "Usage :"
	@echo "  make                           compile plugins actifs + flemme"
	@echo ""
	@echo "  Désactiver un plugin à la compilation :"
	@echo "    make DISABLE_<plugin>=1      ex: make DISABLE_TIMER=1"
	@echo "    Plugins disponibles: $(PLUGINS)"
	@echo ""
	@echo "  Désactiver un plugin dans flemme-web seulement :"
	@echo "    make WEBIDE_DISABLE_<plugin>=1 ou =0"
	@echo ""
	@echo "  Autres cibles :"
	@echo "    make plugins                compile uniquement les .a actifs"
	@echo "    make test                   lance ./flemme test.flm"
	@echo "    make test SCRIPT=foo.flm    lance ./flemme foo.flm"
	@echo "    make web                    build + lance le Web IDE"
	@echo "    make clean                  supprime tous les fichiers générés"
	@echo ""
	@echo "  Pour ajouter un plugin :"
	@echo "    1. Créer plugins/<name>/Makefile et <name>_plugin.hpp"
	@echo "    2. Ajouter '<name>' à PLUGINS dans ce Makefile"
