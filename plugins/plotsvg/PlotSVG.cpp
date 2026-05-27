#include "PlotSVG.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

// ── Layout constants ──────────────────────────────────────────────────────────
static constexpr int W     = 760;
static constexpr int H     = 480;
static constexpr int mL    = 70;
static constexpr int mR    = 25;
static constexpr int mT    = 45;
static constexpr int mB    = 60;
static constexpr int plotW = W - mL - mR;
static constexpr int plotH = H - mT - mB;

// ── Palette ───────────────────────────────────────────────────────────────────
const std::vector<std::string> PlotSVG::kPalette = {
    "#1f77b4", "#ff7f0e", "#2ca02c", "#d62728",
    "#9467bd", "#8c564b", "#e377c2", "#7f7f7f"
};

// ── Helpers ───────────────────────────────────────────────────────────────────
static std::string xmlEsc(const std::string& s) {
    std::string r;
    r.reserve(s.size());
    for (char c : s) {
        if      (c == '&') r += "&amp;";
        else if (c == '<') r += "&lt;";
        else if (c == '>') r += "&gt;";
        else if (c == '"') r += "&quot;";
        else r += c;
    }
    return r;
}

static std::string fmtTick(double v) {
    if (std::abs(v) < 1e-12) return "0";
    double av = std::abs(v);
    if (std::abs(v - std::round(v)) < 1e-9 * (av < 1.0 ? 1.0 : av)) {
        std::ostringstream oss;
        oss << static_cast<long long>(std::round(v));
        return oss.str();
    }
    std::ostringstream oss;
    oss << std::setprecision(4) << v;
    return oss.str();
}

static std::vector<double> niceTicks(double lo, double hi, int target) {
    double range = hi - lo;
    if (range <= 0.0) return {lo, hi};
    double rough  = range / target;
    double expv   = std::floor(std::log10(rough));
    double step10 = std::pow(10.0, expv);
    double norm   = rough / step10;
    double step;
    if      (norm < 1.5) step = step10;
    else if (norm < 3.5) step = 2.0 * step10;
    else if (norm < 7.5) step = 5.0 * step10;
    else                 step = 10.0 * step10;
    double start = std::ceil(lo / step) * step;
    std::vector<double> ticks;
    for (double t = start; t <= hi + step * 1e-9; t += step)
        ticks.push_back(t);
    return ticks;
}

static std::string dashAttr(const std::string& style) {
    if (style == "dashed") return " stroke-dasharray=\"6,3\"";
    if (style == "dotted") return " stroke-dasharray=\"2,3\"";
    return "";
}

// ── Public interface ──────────────────────────────────────────────────────────
PlotSVG::PlotSVG() = default;

std::string PlotSVG::pickColor() {
    if (!nextColor_.empty()) {
        std::string c = nextColor_; nextColor_.clear(); return c;
    }
    return kPalette[static_cast<size_t>(colorIdx_++) % kPalette.size()];
}

void PlotSVG::addSeries(std::vector<double> xs, std::vector<double> ys, bool sc) {
    std::string lbl  = nextLabel_; nextLabel_.clear();
    std::string fill = nextFill_;  nextFill_.clear();
    series_.push_back({std::move(xs), std::move(ys),
                       pickColor(), sc,
                       std::move(lbl), std::move(fill),
                       nextLineStyle_, nextLineWidth_, nextPointSize_});
}

void PlotSVG::plot(std::vector<double> xs, std::vector<double> ys)    { addSeries(std::move(xs), std::move(ys), false); }
void PlotSVG::scatter(std::vector<double> xs, std::vector<double> ys) { addSeries(std::move(xs), std::move(ys), true);  }
void PlotSVG::setTitle(const std::string& s)       { title_  = s; }
void PlotSVG::setLabelX(const std::string& s)      { labelX_ = s; }
void PlotSVG::setLabelY(const std::string& s)      { labelY_ = s; }
void PlotSVG::setColor(const std::string& c)       { nextColor_     = c; }
void PlotSVG::setLabel(const std::string& l)       { nextLabel_     = l; }
void PlotSVG::setFill(const std::string& c)        { nextFill_      = c; }
void PlotSVG::setLineStyle(const std::string& s)   { nextLineStyle_ = s; }
void PlotSVG::setLineWidth(double w)               { nextLineWidth_ = w; }
void PlotSVG::setPointSize(double r)               { nextPointSize_ = r; }
void PlotSVG::setXRange(double lo, double hi)      { xLo_ = lo; xHi_ = hi; hasXRange_ = true; }
void PlotSVG::setYRange(double lo, double hi)      { yLo_ = lo; yHi_ = hi; hasYRange_ = true; }

void PlotSVG::hline(double y, const std::string& color, const std::string& style) {
    hlines_.push_back({y, color, style});
}
void PlotSVG::vline(double x, const std::string& color, const std::string& style) {
    vlines_.push_back({x, color, style});
}

void PlotSVG::clear() {
    series_.clear(); hlines_.clear(); vlines_.clear();
    title_.clear(); labelX_.clear(); labelY_.clear();
    hasXRange_ = hasYRange_ = false;
    nextColor_.clear(); nextLabel_.clear(); nextFill_.clear();
    nextLineStyle_ = "solid"; nextLineWidth_ = 2.0; nextPointSize_ = 3.5;
    colorIdx_ = 0;
}

// ── SVG generation ────────────────────────────────────────────────────────────
void PlotSVG::save(const std::string& filename) const {

    // ── Data range ────────────────────────────────────────────────────────
    double xLo = xLo_, xHi = xHi_, yLo = yLo_, yHi = yHi_;
    if (!hasXRange_ || !hasYRange_) {
        bool firstX = true, firstY = true;
        for (auto& s : series_) {
            size_t n = std::min(s.xs.size(), s.ys.size());
            for (size_t i = 0; i < n; i++) {
                if (!hasXRange_) {
                    if (firstX || s.xs[i] < xLo) xLo = s.xs[i];
                    if (firstX || s.xs[i] > xHi) xHi = s.xs[i];
                    firstX = false;
                }
                if (!hasYRange_) {
                    if (firstY || s.ys[i] < yLo) yLo = s.ys[i];
                    if (firstY || s.ys[i] > yHi) yHi = s.ys[i];
                    firstY = false;
                }
            }
        }
        if (!hasXRange_) {
            double pad = (xHi - xLo) * 0.05;
            if (pad == 0.0) pad = 0.5;
            xLo -= pad; xHi += pad;
        }
        if (!hasYRange_) {
            double pad = (yHi - yLo) * 0.05;
            if (pad == 0.0) pad = 0.5;
            yLo -= pad; yHi += pad;
        }
    }

    // ── Coordinate transforms ─────────────────────────────────────────────
    auto xPx = [&](double x) -> double {
        return mL + (x - xLo) / (xHi - xLo) * plotW;
    };
    auto yPx = [&](double y) -> double {
        return mT + (1.0 - (y - yLo) / (yHi - yLo)) * plotH;
    };

    // ── SVG output ────────────────────────────────────────────────────────
    std::ostringstream o;
    o << std::fixed << std::setprecision(2);

    o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      << "<svg xmlns=\"http://www.w3.org/2000/svg\""
      << " width=\""  << W << "\" height=\"" << H << "\""
      << " viewBox=\"0 0 " << W << " " << H << "\">\n";

    // Background
    o << "  <rect width=\"" << W << "\" height=\"" << H << "\" fill=\"white\"/>\n";

    // Clip path
    o << "  <defs>\n"
      << "    <clipPath id=\"pa\">\n"
      << "      <rect x=\"" << mL << "\" y=\"" << mT
      << "\" width=\"" << plotW << "\" height=\"" << plotH << "\"/>\n"
      << "    </clipPath>\n"
      << "  </defs>\n";

    // Plot area
    o << "  <rect x=\"" << mL << "\" y=\"" << mT
      << "\" width=\"" << plotW << "\" height=\"" << plotH
      << "\" fill=\"#f9f9f9\" stroke=\"#bbbbbb\" stroke-width=\"1\"/>\n";

    // Grid
    auto xTicks = niceTicks(xLo, xHi, 6);
    auto yTicks = niceTicks(yLo, yHi, 5);
    for (double t : xTicks) {
        double px = xPx(t);
        o << "  <line x1=\"" << px << "\" y1=\"" << mT
          << "\" x2=\"" << px << "\" y2=\"" << (mT + plotH)
          << "\" stroke=\"#e0e0e0\" stroke-width=\"1\"/>\n";
    }
    for (double t : yTicks) {
        double py = yPx(t);
        o << "  <line x1=\"" << mL << "\" y1=\"" << py
          << "\" x2=\"" << (mL + plotW) << "\" y2=\"" << py
          << "\" stroke=\"#e0e0e0\" stroke-width=\"1\"/>\n";
    }

    // X ticks + labels
    for (double t : xTicks) {
        double px = xPx(t);
        o << "  <line x1=\"" << px << "\" y1=\"" << (mT + plotH)
          << "\" x2=\"" << px << "\" y2=\"" << (mT + plotH + 5)
          << "\" stroke=\"#444444\" stroke-width=\"1\"/>\n";
        o << "  <text x=\"" << px << "\" y=\"" << (mT + plotH + 18)
          << "\" text-anchor=\"middle\" font-family=\"sans-serif\" font-size=\"11\" fill=\"#333333\">"
          << xmlEsc(fmtTick(t)) << "</text>\n";
    }

    // Y ticks + labels
    for (double t : yTicks) {
        double py = yPx(t);
        o << "  <line x1=\"" << (mL - 5) << "\" y1=\"" << py
          << "\" x2=\"" << mL << "\" y2=\"" << py
          << "\" stroke=\"#444444\" stroke-width=\"1\"/>\n";
        o << "  <text x=\"" << (mL - 8) << "\" y=\"" << (py + 4)
          << "\" text-anchor=\"end\" font-family=\"sans-serif\" font-size=\"11\" fill=\"#333333\">"
          << xmlEsc(fmtTick(t)) << "</text>\n";
    }

    // Axes
    o << "  <line x1=\"" << mL << "\" y1=\"" << (mT + plotH)
      << "\" x2=\"" << (mL + plotW) << "\" y2=\"" << (mT + plotH)
      << "\" stroke=\"#333333\" stroke-width=\"1.5\"/>\n";
    o << "  <line x1=\"" << mL << "\" y1=\"" << mT
      << "\" x2=\"" << mL << "\" y2=\"" << (mT + plotH)
      << "\" stroke=\"#333333\" stroke-width=\"1.5\"/>\n";

    // ── Clipped group ─────────────────────────────────────────────────────
    o << "  <g clip-path=\"url(#pa)\">\n";

    // 1. Fill areas (behind everything)
    for (auto& s : series_) {
        if (s.fill.empty() || s.scatter) continue;
        size_t n = std::min(s.xs.size(), s.ys.size());
        if (n < 2) continue;
        double yZero = std::max(yLo, std::min(yHi, 0.0));
        double yzPx  = yPx(yZero);
        o << "    <path d=\"M" << xPx(s.xs[0]) << "," << yzPx;
        for (size_t i = 0; i < n; i++)
            o << " L" << xPx(s.xs[i]) << "," << yPx(s.ys[i]);
        o << " L" << xPx(s.xs[n - 1]) << "," << yzPx << " Z\""
          << " fill=\"" << s.fill << "\" fill-opacity=\"0.2\" stroke=\"none\"/>\n";
    }

    // 2. Zero axes (si y=0 ou x=0 est dans la plage visible)
    if (yLo < 0.0 && 0.0 < yHi) {
        double py = yPx(0.0);
        o << "    <line x1=\"" << mL << "\" y1=\"" << py
          << "\" x2=\"" << (mL + plotW) << "\" y2=\"" << py
          << "\" stroke=\"#555555\" stroke-width=\"1\"/>\n";
    }
    if (xLo < 0.0 && 0.0 < xHi) {
        double px = xPx(0.0);
        o << "    <line x1=\"" << px << "\" y1=\"" << mT
          << "\" x2=\"" << px << "\" y2=\"" << (mT + plotH)
          << "\" stroke=\"#555555\" stroke-width=\"1\"/>\n";
    }

    // 3. Lignes de référence hline / vline
    for (auto& hl : hlines_) {
        if (hl.value <= yLo || hl.value >= yHi) continue;
        double py = yPx(hl.value);
        o << "    <line x1=\"" << mL << "\" y1=\"" << py
          << "\" x2=\"" << (mL + plotW) << "\" y2=\"" << py
          << "\" stroke=\"" << hl.color << "\" stroke-width=\"1.5\""
          << dashAttr(hl.style) << "/>\n";
    }
    for (auto& vl : vlines_) {
        if (vl.value <= xLo || vl.value >= xHi) continue;
        double px = xPx(vl.value);
        o << "    <line x1=\"" << px << "\" y1=\"" << mT
          << "\" x2=\"" << px << "\" y2=\"" << (mT + plotH)
          << "\" stroke=\"" << vl.color << "\" stroke-width=\"1.5\""
          << dashAttr(vl.style) << "/>\n";
    }

    // 4. Données
    for (auto& s : series_) {
        size_t n = std::min(s.xs.size(), s.ys.size());
        if (n == 0) continue;
        if (s.scatter) {
            for (size_t i = 0; i < n; i++) {
                o << "    <circle cx=\"" << xPx(s.xs[i]) << "\" cy=\"" << yPx(s.ys[i])
                  << "\" r=\"" << s.pointSize
                  << "\" fill=\"" << s.color << "\" opacity=\"0.85\"/>\n";
            }
        } else {
            o << "    <path d=\"";
            for (size_t i = 0; i < n; i++)
                o << (i == 0 ? 'M' : 'L') << xPx(s.xs[i]) << ',' << yPx(s.ys[i]) << ' ';
            o << "\" fill=\"none\" stroke=\"" << s.color
              << "\" stroke-width=\"" << s.lineWidth << "\""
              << dashAttr(s.lineStyle) << "/>\n";
        }
    }

    o << "  </g>\n";

    // Title
    if (!title_.empty()) {
        o << "  <text x=\"" << (mL + plotW / 2) << "\" y=\"" << (mT - 14)
          << "\" text-anchor=\"middle\" font-family=\"sans-serif\""
          << " font-size=\"14\" font-weight=\"bold\" fill=\"#222222\">"
          << xmlEsc(title_) << "</text>\n";
    }

    // X label
    if (!labelX_.empty()) {
        o << "  <text x=\"" << (mL + plotW / 2) << "\" y=\"" << (H - 6)
          << "\" text-anchor=\"middle\" font-family=\"sans-serif\""
          << " font-size=\"13\" fill=\"#444444\">"
          << xmlEsc(labelX_) << "</text>\n";
    }

    // Y label (rotated)
    if (!labelY_.empty()) {
        int cx = 14, cy = mT + plotH / 2;
        o << "  <text x=\"" << cx << "\" y=\"" << cy
          << "\" text-anchor=\"middle\" font-family=\"sans-serif\""
          << " font-size=\"13\" fill=\"#444444\""
          << " transform=\"rotate(-90," << cx << "," << cy << ")\">"
          << xmlEsc(labelY_) << "</text>\n";
    }

    // Legend
    {
        std::vector<const Series*> labeled;
        for (auto& s : series_)
            if (!s.label.empty()) labeled.push_back(&s);

        if (!labeled.empty()) {
            const int pad  = 8;
            const int smpW = 22;
            const int gap  = 6;
            const int entH = 20;
            const int charW = 7;

            size_t maxChars = 0;
            for (auto* s : labeled)
                maxChars = std::max(maxChars, s->label.size());
            int boxW = 2 * pad + smpW + gap + static_cast<int>(maxChars) * charW;
            int boxH = 2 * pad + static_cast<int>(labeled.size()) * entH;

            int bx = mL + plotW - 10 - boxW;
            int by = mT + 10;

            o << "  <rect x=\"" << bx << "\" y=\"" << by
              << "\" width=\"" << boxW << "\" height=\"" << boxH
              << "\" rx=\"4\" fill=\"white\" fill-opacity=\"0.88\""
              << " stroke=\"#bbbbbb\" stroke-width=\"1\"/>\n";

            for (size_t i = 0; i < labeled.size(); i++) {
                const Series* s = labeled[i];
                int ey = by + pad + static_cast<int>(i) * entH + entH / 2;
                int sx = bx + pad;
                int tx = sx + smpW + gap;

                if (s->scatter) {
                    o << "  <circle cx=\"" << (sx + smpW / 2) << "\" cy=\"" << ey
                      << "\" r=\"" << s->pointSize << "\" fill=\"" << s->color << "\"/>\n";
                } else {
                    o << "  <line x1=\"" << sx << "\" y1=\"" << ey
                      << "\" x2=\"" << (sx + smpW) << "\" y2=\"" << ey
                      << "\" stroke=\"" << s->color
                      << "\" stroke-width=\"" << s->lineWidth << "\""
                      << dashAttr(s->lineStyle) << "/>\n";
                }
                o << "  <text x=\"" << tx << "\" y=\"" << (ey + 4)
                  << "\" font-family=\"sans-serif\" font-size=\"12\" fill=\"#222222\">"
                  << xmlEsc(s->label) << "</text>\n";
            }
        }
    }

    o << "</svg>\n";

    std::ofstream f(filename);
    if (!f.is_open())
        throw std::runtime_error("PlotSVG.save : impossible d'écrire dans : " + filename);
    f << o.str();
}
