#pragma once
#include <string>

// Implémentation autonome de Rockable pour tester le plugin sans la vraie lib
// DEM.
class MockRockable {
public:
  double t{0.0};
  double tmax{1.0};
  double dt{1e-6};
  double numericalDampingCoeff{0.0};
  double velocityBarrier{0.0};
  int nDriven{0};

  MockRockable();

  void setVerboseLevel(int v);
  void setVerboseLevel(const std::string &levelName);
  void setOpenMPThreads(int nbThreads);
  void setIntegrator(const std::string &name);
  void setUpdateNL(const std::string &name);
  void initialChecks();

  void loadConf(int i);
  void loadConf(const char *filename);
  void loadShapes(const char *filename);
  void saveConf(int i);
  void saveConf(const char *filename);

  void integrate();
  void velocityVerletStep();
  void EulerStep();
  void BeemanStep();
  void RungeKutta4Step();

  void getKineticEnergy(double &Etrans, double &Erot, int first = 0,
                        int last = 0) const;
  void getCriticalTimeStep(double &dtc) const;
  void getResultantQuickStats(double &Fmax, double &F_fnmax, double &Fmean,
                              double &Fstddev, int first = 0,
                              int last = 0) const;
};
