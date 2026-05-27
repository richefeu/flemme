#include "MockRockable.hpp"
#include <iostream>
#include <string>

static void log(const std::string& msg) {
    std::cout << "[MockRockable] " << msg << "\n";
}

MockRockable::MockRockable() { log("Rockable() — simulation créée"); }

void MockRockable::setVerboseLevel(int v)                      { log("setVerboseLevel(" + std::to_string(v) + ")"); }
void MockRockable::setVerboseLevel(const std::string& name)    { log("setVerboseLevel(\"" + name + "\")"); }
void MockRockable::setOpenMPThreads(int n)                     { log("setOpenMPThreads(" + std::to_string(n) + ")"); }
void MockRockable::setIntegrator(const std::string& name)      { log("setIntegrator(\"" + name + "\")"); }
void MockRockable::setUpdateNL(const std::string& name)        { log("setUpdateNL(\"" + name + "\")"); }
void MockRockable::initialChecks()                             { log("initialChecks() — OK"); }

void MockRockable::loadConf(int i)              { log("loadConf(" + std::to_string(i) + ")"); }
void MockRockable::loadConf(const char* f)      { log(std::string("loadConf(\"") + f + "\")"); }
void MockRockable::loadShapes(const char* f)    { log(std::string("loadShapes(\"") + f + "\")"); }
void MockRockable::saveConf(int i)              { log("saveConf(" + std::to_string(i) + ")"); }
void MockRockable::saveConf(const char* f)      { log(std::string("saveConf(\"") + f + "\")"); }

void MockRockable::integrate() {
    log("integrate() — t=" + std::to_string(t) + " → tmax=" + std::to_string(tmax));
}

void MockRockable::velocityVerletStep() { t += dt; log("velocityVerletStep() — t=" + std::to_string(t)); }
void MockRockable::EulerStep()          { t += dt; log("EulerStep() — t=" + std::to_string(t)); }
void MockRockable::BeemanStep()         { t += dt; log("BeemanStep() — t=" + std::to_string(t)); }
void MockRockable::RungeKutta4Step()    { t += dt; log("RungeKutta4Step() — t=" + std::to_string(t)); }

void MockRockable::getKineticEnergy(double& Etrans, double& Erot, int first, int last) const {
    Etrans = 42.0; Erot = 7.0;
    log("getKineticEnergy() → Etrans=" + std::to_string(Etrans) + ", Erot=" + std::to_string(Erot));
}

void MockRockable::getCriticalTimeStep(double& dtc) const {
    dtc = 1e-5;
    log("getCriticalTimeStep() → dtc=" + std::to_string(dtc));
}

void MockRockable::getResultantQuickStats(double& Fmax, double& F_fnmax,
                                          double& Fmean, double& Fstddev,
                                          int first, int last) const {
    Fmax = 100.0; F_fnmax = 80.0; Fmean = 50.0; Fstddev = 10.0;
    log("getResultantQuickStats() → Fmax=" + std::to_string(Fmax) + ", Fmean=" + std::to_string(Fmean));
}
