// timer_plugin.hpp — plugin Timer pour flemme
// Prérequis : interpreter.hpp inclus avant ce fichier (garanti par plugins.hpp)
//
// Usage dans un script .flm :
//   let t = Timer();
//   # ... calcul ...
//   let dt = t.elapsed();
//   print "Durée : {dt:.4f} s";
//   t.reset();
#pragma once
#include "FlemmeTimer.hpp"
#include <memory>

inline void registerTimerPlugin(Interpreter &interp) {
  interp.registerConstructor("Timer", []() -> Value {
    auto obj = std::make_shared<NativeObject>("Timer");
    auto tim = std::make_shared<FlemmeTimer>();

    obj->methods["reset"] = [tim](std::vector<Value>) -> Value {
      tim->reset();
      return {};
    };
    obj->methods["elapsed"] = [tim](std::vector<Value>) -> Value {
      return Value{tim->elapsed()};
    };

    return Value{obj};
  });
}
