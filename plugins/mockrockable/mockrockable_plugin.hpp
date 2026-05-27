// mockrockable_plugin.hpp — plugin MockRockable pour flemme
// Expose MockRockable comme "Rockable" dans le langage, compatible avec les
// scripts écrits pour le vrai plugin Rockable.
// Prérequis : interpreter.hpp inclus avant ce fichier (garanti par plugins.hpp)
#pragma once
#include "MockRockable.hpp"
#include <memory>
#include <stdexcept>

inline void registerMockRockablePlugin(Interpreter &interp) {
  interp.registerConstructor("Rockable", []() -> Value {
    auto obj = std::make_shared<NativeObject>("Rockable");
    auto sim = std::make_shared<MockRockable>();

    obj->methods["setVerboseLevel"] = [sim](std::vector<Value> args) -> Value {
      if (args.empty())
        throw std::runtime_error("setVerboseLevel : argument manquant");
      if (std::holds_alternative<double>(args[0]))
        sim->setVerboseLevel(static_cast<int>(asDouble(args[0])));
      else
        sim->setVerboseLevel(asString(args[0]));
      return {};
    };
    obj->methods["setOpenMPThreads"] = [sim](std::vector<Value> args) -> Value {
      if (args.empty())
        throw std::runtime_error("setOpenMPThreads : argument manquant");
      sim->setOpenMPThreads(static_cast<int>(asDouble(args[0])));
      return {};
    };
    obj->methods["setIntegrator"] = [sim](std::vector<Value> args) -> Value {
      if (args.empty())
        throw std::runtime_error("setIntegrator : argument manquant");
      sim->setIntegrator(asString(args[0]));
      return {};
    };
    obj->methods["setUpdateNL"] = [sim](std::vector<Value> args) -> Value {
      if (args.empty())
        throw std::runtime_error("setUpdateNL : argument manquant");
      sim->setUpdateNL(asString(args[0]));
      return {};
    };
    obj->methods["initialChecks"] = [sim](std::vector<Value>) -> Value {
      sim->initialChecks();
      return {};
    };
    obj->methods["loadConf"] = [sim](std::vector<Value> args) -> Value {
      if (args.empty())
        throw std::runtime_error("loadConf : argument manquant");
      if (std::holds_alternative<double>(args[0]))
        sim->loadConf(static_cast<int>(asDouble(args[0])));
      else
        sim->loadConf(asString(args[0]).c_str());
      return {};
    };
    obj->methods["loadShapes"] = [sim](std::vector<Value> args) -> Value {
      if (args.empty())
        throw std::runtime_error("loadShapes : argument manquant");
      sim->loadShapes(asString(args[0]).c_str());
      return {};
    };
    obj->methods["saveConf"] = [sim](std::vector<Value> args) -> Value {
      if (args.empty())
        throw std::runtime_error("saveConf : argument manquant");
      if (std::holds_alternative<double>(args[0]))
        sim->saveConf(static_cast<int>(asDouble(args[0])));
      else
        sim->saveConf(asString(args[0]).c_str());
      return {};
    };

    obj->methods["integrate"] = [sim](std::vector<Value>) -> Value {
      sim->integrate();
      return {};
    };
    obj->methods["velocityVerletStep"] = [sim](std::vector<Value>) -> Value {
      sim->velocityVerletStep();
      return {};
    };
    obj->methods["EulerStep"] = [sim](std::vector<Value>) -> Value {
      sim->EulerStep();
      return {};
    };
    obj->methods["BeemanStep"] = [sim](std::vector<Value>) -> Value {
      sim->BeemanStep();
      return {};
    };
    obj->methods["RungeKutta4Step"] = [sim](std::vector<Value>) -> Value {
      sim->RungeKutta4Step();
      return {};
    };

    obj->methods["getKineticEnergy"] = [sim](std::vector<Value>) -> Value {
      double Etrans = 0.0, Erot = 0.0;
      sim->getKineticEnergy(Etrans, Erot);
      return Value{Etrans};
    };
    obj->methods["getErot"] = [sim](std::vector<Value>) -> Value {
      double Etrans = 0.0, Erot = 0.0;
      sim->getKineticEnergy(Etrans, Erot);
      return Value{Erot};
    };
    obj->methods["getCriticalTimeStep"] = [sim](std::vector<Value>) -> Value {
      double dtc = 0.0;
      sim->getCriticalTimeStep(dtc);
      return Value{dtc};
    };
    obj->methods["getResultantQuickStats"] =
        [sim](std::vector<Value>) -> Value {
      double Fmax = 0.0, F_fnmax = 0.0, Fmean = 0.0, Fstddev = 0.0;
      sim->getResultantQuickStats(Fmax, F_fnmax, Fmean, Fstddev);
      return Value{Fmax};
    };

    obj->getters["t"] = [sim]() -> Value { return Value{sim->t}; };
    obj->getters["tmax"] = [sim]() -> Value { return Value{sim->tmax}; };
    obj->getters["dt"] = [sim]() -> Value { return Value{sim->dt}; };
    obj->getters["numericalDampingCoeff"] = [sim]() -> Value {
      return Value{sim->numericalDampingCoeff};
    };
    obj->getters["velocityBarrier"] = [sim]() -> Value {
      return Value{sim->velocityBarrier};
    };
    obj->getters["nDriven"] = [sim]() -> Value {
      return Value{static_cast<double>(sim->nDriven)};
    };

    obj->setters["t"] = [sim](Value v) { sim->t = asDouble(v); };
    obj->setters["tmax"] = [sim](Value v) { sim->tmax = asDouble(v); };
    obj->setters["dt"] = [sim](Value v) { sim->dt = asDouble(v); };
    obj->setters["numericalDampingCoeff"] = [sim](Value v) {
      sim->numericalDampingCoeff = asDouble(v);
    };
    obj->setters["velocityBarrier"] = [sim](Value v) {
      sim->velocityBarrier = asDouble(v);
    };
    obj->setters["nDriven"] = [sim](Value v) {
      sim->nDriven = static_cast<int>(asDouble(v));
    };

    return Value{obj};
  });
}
