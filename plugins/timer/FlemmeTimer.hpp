#pragma once
#include <chrono>

// Chronomètre haute résolution.
// Utilisé depuis flemme via : let t = Timer();
class FlemmeTimer {
    std::chrono::steady_clock::time_point start_;
public:
    FlemmeTimer();
    void   reset();
    double elapsed() const;  // secondes écoulées depuis reset()
};
