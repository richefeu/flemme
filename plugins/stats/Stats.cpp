#include "Stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

void Stats::add(double x)  { data_.push_back(x); }
void Stats::clear()        { data_.clear(); }
int  Stats::count()  const { return static_cast<int>(data_.size()); }

double Stats::mean() const {
    if (data_.empty()) throw std::runtime_error("Stats.mean : aucune donnée");
    return std::accumulate(data_.begin(), data_.end(), 0.0) / data_.size();
}

double Stats::variance() const {
    if (data_.empty()) throw std::runtime_error("Stats.variance : aucune donnée");
    double m = mean(), s = 0.0;
    for (double x : data_) s += (x - m) * (x - m);
    return s / data_.size();
}

double Stats::stddev() const { return std::sqrt(variance()); }

double Stats::min() const {
    if (data_.empty()) throw std::runtime_error("Stats.min : aucune donnée");
    return *std::min_element(data_.begin(), data_.end());
}

double Stats::max() const {
    if (data_.empty()) throw std::runtime_error("Stats.max : aucune donnée");
    return *std::max_element(data_.begin(), data_.end());
}
