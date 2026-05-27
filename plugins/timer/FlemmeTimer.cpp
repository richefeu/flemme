#include "FlemmeTimer.hpp"

FlemmeTimer::FlemmeTimer() { reset(); }

void FlemmeTimer::reset() {
    start_ = std::chrono::steady_clock::now();
}

double FlemmeTimer::elapsed() const {
    return std::chrono::duration<double>(
        std::chrono::steady_clock::now() - start_
    ).count();
}
