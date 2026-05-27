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
# Outils
# ------------------------------------------------
CXX      := g++
AR       := ar
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

# ------------------------------------------------
# Interpréteur (headers-only sauf main.cpp)
# ------------------------------------------------
INTERP_HEADERS := lexer.hpp parser.hpp interpreter.hpp plugins.hpp
INTERP_SRC     := main.cpp
INTERP_OBJ     := main.o
BIN            := flemme

# ------------------------------------------------
# Plugins — désactiver avec DISABLE_X=1
# ------------------------------------------------
DISABLE_TIMER        ?= 0
DISABLE_STATS        ?= 0
DISABLE_MOCKROCKABLE ?= 0
DISABLE_PLOTSVG      ?= 0
DISABLE_SCHEMA       ?= 0

PLUGIN_TIMER_DIR        := plugins/timer
PLUGIN_STATS_DIR        := plugins/stats
PLUGIN_MOCKROCKABLE_DIR := plugins/mockrockable
PLUGIN_PLOTSVG_DIR      := plugins/plotsvg
PLUGIN_SCHEMA_DIR       := plugins/schema

PLUGIN_TIMER_LIB        := $(PLUGIN_TIMER_DIR)/libflemme-timer.a
PLUGIN_STATS_LIB        := $(PLUGIN_STATS_DIR)/libflemme-stats.a
PLUGIN_MOCKROCKABLE_LIB := $(PLUGIN_MOCKROCKABLE_DIR)/libflemme-mockrockable.a
PLUGIN_PLOTSVG_LIB      := $(PLUGIN_PLOTSVG_DIR)/libflemme-plotsvg.a
PLUGIN_SCHEMA_LIB       := $(PLUGIN_SCHEMA_DIR)/libflemme-schema.a

PLUGIN_LIBS :=
PLUGIN_INCS :=
PLUGIN_DEFS :=

ifneq ($(DISABLE_TIMER),1)
  PLUGIN_LIBS += $(PLUGIN_TIMER_LIB)
  PLUGIN_INCS += -I$(PLUGIN_TIMER_DIR)
  PLUGIN_DEFS += -DFLEMME_PLUGIN_TIMER
endif
ifneq ($(DISABLE_STATS),1)
  PLUGIN_LIBS += $(PLUGIN_STATS_LIB)
  PLUGIN_INCS += -I$(PLUGIN_STATS_DIR)
  PLUGIN_DEFS += -DFLEMME_PLUGIN_STATS
endif
ifneq ($(DISABLE_MOCKROCKABLE),1)
  PLUGIN_LIBS += $(PLUGIN_MOCKROCKABLE_LIB)
  PLUGIN_INCS += -I$(PLUGIN_MOCKROCKABLE_DIR)
  PLUGIN_DEFS += -DFLEMME_PLUGIN_MOCKROCKABLE
endif
ifneq ($(DISABLE_PLOTSVG),1)
  PLUGIN_LIBS += $(PLUGIN_PLOTSVG_LIB)
  PLUGIN_INCS += -I$(PLUGIN_PLOTSVG_DIR)
  PLUGIN_DEFS += -DFLEMME_PLUGIN_PLOTSVG
endif
ifneq ($(DISABLE_SCHEMA),1)
  PLUGIN_LIBS += $(PLUGIN_SCHEMA_LIB)
  PLUGIN_INCS += -I$(PLUGIN_SCHEMA_DIR)
  PLUGIN_DEFS += -DFLEMME_PLUGIN_SCHEMA
endif

# ------------------------------------------------
# Flags finaux (interpréteur)
# ------------------------------------------------
ALL_CXXFLAGS := $(CXXFLAGS) $(PLUGIN_INCS) $(PLUGIN_DEFS)

# ------------------------------------------------
# Plugins désactivés pour flemme-web uniquement
# Indépendants des DISABLE_* (qui agissent sur les deux binaires).
# Un plugin WEBIDE_DISABLE peut rester compilé/lié mais ne sera pas
# enregistré par le serveur web.
# ------------------------------------------------
WEBIDE_DISABLE_TIMER        ?= 0
WEBIDE_DISABLE_STATS        ?= 0
WEBIDE_DISABLE_MOCKROCKABLE ?= 1
WEBIDE_DISABLE_PLOTSVG      ?= 0

WEBIDE_PLUGIN_DEFS :=
ifneq ($(WEBIDE_DISABLE_TIMER),0)
  WEBIDE_PLUGIN_DEFS += -DWEBIDE_NO_PLUGIN_TIMER
endif
ifneq ($(WEBIDE_DISABLE_STATS),0)
  WEBIDE_PLUGIN_DEFS += -DWEBIDE_NO_PLUGIN_STATS
endif
ifneq ($(WEBIDE_DISABLE_MOCKROCKABLE),0)
  WEBIDE_PLUGIN_DEFS += -DWEBIDE_NO_PLUGIN_MOCKROCKABLE
endif
ifneq ($(WEBIDE_DISABLE_PLOTSVG),0)
  WEBIDE_PLUGIN_DEFS += -DWEBIDE_NO_PLUGIN_PLOTSVG
endif

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

## Bibliothèques de plugins (chacune déclenchée par ses dépendants)
$(PLUGIN_TIMER_LIB):
	$(MAKE) -C $(PLUGIN_TIMER_DIR)

$(PLUGIN_STATS_LIB):
	$(MAKE) -C $(PLUGIN_STATS_DIR)

$(PLUGIN_MOCKROCKABLE_LIB):
	$(MAKE) -C $(PLUGIN_MOCKROCKABLE_DIR)

$(PLUGIN_PLOTSVG_LIB):
	$(MAKE) -C $(PLUGIN_PLOTSVG_DIR)

$(PLUGIN_SCHEMA_LIB):
	$(MAKE) -C $(PLUGIN_SCHEMA_DIR)

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
	$(MAKE) -C $(PLUGIN_TIMER_DIR) clean
	$(MAKE) -C $(PLUGIN_STATS_DIR) clean
	$(MAKE) -C $(PLUGIN_MOCKROCKABLE_DIR) clean
	$(MAKE) -C $(PLUGIN_PLOTSVG_DIR) clean
	$(MAKE) -C $(PLUGIN_SCHEMA_DIR) clean

## Aide ────────────────────────────────────────────────────────────────────────
help:
	@echo "Usage :"
	@echo "  make                           compile plugins actifs + flemme"
	@echo "  make DISABLE_TIMER=1           compile sans le plugin Timer"
	@echo "  make DISABLE_STATS=1           compile sans le plugin Stats"
	@echo "  make DISABLE_MOCKROCKABLE=1    compile sans le plugin MockRockable"
	@echo "  make DISABLE_PLOTSVG=1         compile sans le plugin PlotSVG"
	@echo "  make WEBIDE_DISABLE_STATS=1          desactive Stats dans flemme-web seulement"
	@echo "  make WEBIDE_DISABLE_MOCKROCKABLE=0   reactive MockRockable dans flemme-web"
	@echo "  make plugins                         compile uniquement les .a actifs"
	@echo "  make test                      lance ./flemme test.flm"
	@echo "  make test SCRIPT=foo.flm       lance ./flemme foo.flm"
	@echo "  make web                       build + lance le Web IDE"
	@echo "  make clean                     supprime tous les fichiers générés"
