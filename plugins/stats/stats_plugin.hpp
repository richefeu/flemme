// stats_plugin.hpp — plugin Stats pour flemme
// Prérequis : interpreter.hpp inclus avant ce fichier (garanti par plugins.hpp)
//
// Usage dans un script .flm :
//   let s = Stats();
//   for (let i in range(100)) { s.add(i * i); }
//   print "n        = {s.count}";
//   print "moyenne  = {s.mean():.4f}";
//   print "écart-type = {s.stddev():.4f}";
//   print "min / max = {s.min()} / {s.max()}";
//   s.clear();
#pragma once
#include "Stats.hpp"
#include <memory>

inline void registerStatsPlugin(Interpreter& interp) {
    interp.registerConstructor("Stats", []() -> Value {
        auto obj = std::make_shared<NativeObject>("Stats");
        auto st  = std::make_shared<Stats>();

        obj->methods["add"] = [st](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<double>(args[0]))
                throw std::runtime_error("Stats.add : attend un nombre");
            st->add(std::get<double>(args[0]));
            return {};
        };
        obj->methods["clear"]    = [st](std::vector<Value>) -> Value { st->clear(); return {}; };
        obj->methods["mean"]     = [st](std::vector<Value>) -> Value { return Value{st->mean()}; };
        obj->methods["variance"] = [st](std::vector<Value>) -> Value { return Value{st->variance()}; };
        obj->methods["stddev"]   = [st](std::vector<Value>) -> Value { return Value{st->stddev()}; };
        obj->methods["min"]      = [st](std::vector<Value>) -> Value { return Value{st->min()}; };
        obj->methods["max"]      = [st](std::vector<Value>) -> Value { return Value{st->max()}; };

        obj->getters["count"] = [st]() -> Value {
            return Value{static_cast<double>(st->count())};
        };

        return Value{obj};
    });
}
