# Système de plugins C++ ↔ Flemme

Ce document explique comment exposer des objets et fonctions C++ au langage Flemme via le système de plugins.

---

## Vue d'ensemble

L'interpréteur Flemme est instancié depuis le C++ via la classe `Interpreter` (`interpreter.hpp`).
Deux mécanismes d'exposition sont disponibles :

| Mécanisme | Usage |
|---|---|
| `registerFunction(name, fn)` | Fonction native standalone |
| `registerConstructor(name, ctor)` | Constructeur d'objet (`NativeObject`) |

Un **plugin** regroupe ces enregistrements dans un fichier `*_plugin.hpp` et compile la logique C++ dans une bibliothèque statique `.a` indépendante.

---

## 1. Fonctions natives

```cpp
#include "interpreter.hpp"

Interpreter interp;

interp.registerFunction("carre", [](std::vector<Value> args) -> Value {
    if (args.size() != 1 || !std::holds_alternative<double>(args[0]))
        throw std::runtime_error("carre : attend un nombre");
    double x = std::get<double>(args[0]);
    return Value{x * x};
});
```

Côté Flemme :
```flemme
let y = carre(7);
print "{y}";   # affiche 49
```

---

## 2. Objets C++ (`NativeObject`)

Un `NativeObject` est une struct qui contient trois maps de lambdas :
- `getters` : `nom → () → Value`
- `setters` : `nom → (Value) → void`
- `methods` : `nom → (vector<Value>) → Value`

### Exemple : exposer une classe C++

```cpp
#include "interpreter.hpp"

struct Simulation {
    double tmax = 10.0;
    int    steps = 100;
    std::vector<double> data;

    void run() {
        data.clear();
        double dt = tmax / steps;
        for (int i = 0; i <= steps; ++i)
            data.push_back(i * dt);
    }

    double getResult(int i) const { return data.at(i); }
};
```

### Enregistrement

```cpp
interp.registerConstructor("Simulation", []() -> Value {
    auto obj = std::make_shared<NativeObject>("Simulation");
    auto sim = std::make_shared<Simulation>();

    obj->getters["tmax"]  = [sim]() -> Value { return Value{sim->tmax}; };
    obj->getters["steps"] = [sim]() -> Value { return Value{static_cast<double>(sim->steps)}; };

    obj->setters["tmax"]  = [sim](Value v) { sim->tmax  = asDouble(v); };
    obj->setters["steps"] = [sim](Value v) { sim->steps = static_cast<int>(asDouble(v)); };

    obj->methods["run"] = [sim](std::vector<Value>) -> Value {
        sim->run(); return {};
    };
    obj->methods["getResult"] = [sim](std::vector<Value> args) -> Value {
        return Value{sim->getResult(static_cast<int>(asDouble(args[0])))};
    };

    return Value{obj};
});
```

Côté Flemme :
```flemme
let sim = Simulation();
sim.tmax  = 5;
sim.steps = 50;
sim.run();
print "valeur centrale : {sim.getResult(25)}";
```

---

## 3. Créer un plugin complet

Un plugin autonome se compose de quatre fichiers dans `plugins/monoutil/` :

### `MonUtil.hpp` / `MonUtil.cpp`
La classe C++ pure, sans dépendance à Flemme.

### `Makefile`
```makefile
CXX  := g++
AR   := ar
CXXFLAGS := -std=c++17 -O2 -Wall

LIB := libflemme-monoutil.a
OBJ := MonUtil.o

all: $(LIB)
$(LIB): $(OBJ)
	$(AR) rcs $@ $^
$(OBJ): MonUtil.cpp MonUtil.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(OBJ) $(LIB)
```

### `monoutil_plugin.hpp`
```cpp
#pragma once
#include "MonUtil.hpp"
// Prérequis : interpreter.hpp inclus avant ce fichier (garanti par plugins.hpp)

inline void registerMonUtilPlugin(Interpreter& interp) {
    interp.registerConstructor("MonUtil", []() -> Value {
        auto obj = std::make_shared<NativeObject>("MonUtil");
        auto u   = std::make_shared<MonUtil>();
        // ... obj->methods["foo"] = ...
        return Value{obj};
    });
}
```

### Activation dans `plugins.hpp`
```cpp
#ifdef FLEMME_PLUGIN_MONOUTIL
#  include "plugins/monoutil/monoutil_plugin.hpp"
#endif

inline void registerPlugins(Interpreter& interp,
                            const std::unordered_set<std::string>& disabled = {}) {
    // ...
#ifdef FLEMME_PLUGIN_MONOUTIL
    if (!disabled.count("monoutil")) registerMonUtilPlugin(interp);
#endif
}
```

### Activation dans le `Makefile` principal
```makefile
PLUGIN_MONOUTIL_DIR := plugins/monoutil
PLUGIN_MONOUTIL_LIB := $(PLUGIN_MONOUTIL_DIR)/libflemme-monoutil.a

ifneq ($(DISABLE_MONOUTIL),1)
  PLUGIN_LIBS += $(PLUGIN_MONOUTIL_LIB)
  PLUGIN_INCS += -I$(PLUGIN_MONOUTIL_DIR)
  PLUGIN_DEFS += -DFLEMME_PLUGIN_MONOUTIL
endif

$(PLUGIN_MONOUTIL_LIB):
	$(MAKE) -C $(PLUGIN_MONOUTIL_DIR)
```

---

## 4. Désactiver un plugin

| Moment | Commande |
|---|---|
| Compilation (binaire complet) | `make DISABLE_MONOUTIL=1` |
| Compilation (web IDE seulement) | `make WEBIDE_DISABLE_MONOUTIL=1` |
| Exécution | `./flemme --no-plugin monoutil script.flm` |

---

## 5. Valeurs échangeables

| Type Flemme | Type C++ (dans `Value`) |
|---|---|
| nombre | `double` |
| chaîne | `std::string` |
| tableau | `std::shared_ptr<ArrayObject>` |
| objet natif | `std::shared_ptr<NativeObject>` |
| void | `std::monostate` |

### Helpers disponibles dans `interpreter.hpp`

```cpp
double      asDouble(const Value& v);   // extrait un double
std::string asString(const Value& v);   // extrait une string
ArrayObject* getArray(const Value& v);  // retourne nullptr si pas un tableau
```

### Créer un tableau depuis C++

```cpp
std::vector<Value> elems;
for (double v : data) elems.push_back(Value{v});
return Value{std::make_shared<ArrayObject>(std::move(elems))};
```

---

## 6. Exécuter un script depuis C++

```cpp
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "plugins.hpp"

std::string source = "let sim = Simulation(); sim.run();";

Interpreter interp;
registerPlugins(interp);   // ou registerPlugins(interp, {"monoutil"}) pour désactiver

try {
    Lexer  lexer(source);
    Parser parser(lexer.tokenize());
    interp.run(parser.parse().get());
} catch (const std::exception& e) {
    std::cerr << "Erreur : " << e.what() << "\n";
}
```

---

## 7. Plugins fournis

| Plugin | Classe | Constructeur | Méthodes / Getters |
|---|---|---|---|
| `timer` | `FlemmeTimer` | `Timer()` | `elapsed()` · `reset()` |
| `stats` | `Stats` | `Stats()` | `add(x)` · `clear()` · `mean()` · `variance()` · `stddev()` · `min()` · `max()` · getter `count` |
| `mockrockable` | `MockRockable` | `Rockable()` | `setIntegrator` · `velocityVerletStep` · `getKineticEnergy` · getters/setters `t` `dt` `tmax` … |

Chaque plugin possède un script de test dans son répertoire (`test_timer.flm`, `test_stats.flm`, `test_mockrockable.flm`).
