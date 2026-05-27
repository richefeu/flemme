#pragma once
#include <vector>

// Collecteur de statistiques descriptives en ligne.
// Utilisé depuis flemme via : let s = Stats();
class Stats {
    std::vector<double> data_;
public:
    void   add(double x);
    void   clear();
    int    count()    const;
    double mean()     const;
    double variance() const;  // variance populationnelle
    double stddev()   const;
    double min()      const;
    double max()      const;
};
