#pragma once
#include <string>
#include <vector>

class PlotSVG {
public:
    struct Series {
        std::vector<double> xs, ys;
        std::string color;
        bool scatter;
        std::string label;
        std::string fill;       // fill area to y=0; "" = no fill
        std::string lineStyle;  // "solid" | "dashed" | "dotted"
        double lineWidth;
        double pointSize;
    };

    struct RefLine {
        double value;
        std::string color;
        std::string style;
    };

    PlotSVG();

    // Ajout de séries
    void plot(std::vector<double> xs, std::vector<double> ys);
    void scatter(std::vector<double> xs, std::vector<double> ys);

    // Étiquettes globales
    void setTitle(const std::string& s);
    void setLabelX(const std::string& s);
    void setLabelY(const std::string& s);

    // Style de la prochaine série (color/label/fill consommés après plot/scatter)
    void setColor(const std::string& color);
    void setLabel(const std::string& label);
    void setFill(const std::string& color);

    // Style persistant (reste actif jusqu'au prochain appel ou clear())
    void setLineStyle(const std::string& style);  // "solid"|"dashed"|"dotted"
    void setLineWidth(double w);
    void setPointSize(double r);

    // Lignes de référence
    void hline(double y,
               const std::string& color = "#888888",
               const std::string& style = "dashed");
    void vline(double x,
               const std::string& color = "#888888",
               const std::string& style = "dashed");

    // Plage manuelle
    void setXRange(double lo, double hi);
    void setYRange(double lo, double hi);

    void save(const std::string& filename) const;
    void clear();

    static const std::vector<std::string> kPalette;

private:
    std::vector<Series>  series_;
    std::vector<RefLine> hlines_, vlines_;

    std::string title_, labelX_, labelY_;
    bool   hasXRange_ = false, hasYRange_ = false;
    double xLo_ = 0.0, xHi_ = 1.0, yLo_ = 0.0, yHi_ = 1.0;

    // Consommés par addSeries
    std::string nextColor_;
    std::string nextLabel_;
    std::string nextFill_;

    // Persistants jusqu'à clear() ou nouvel appel
    std::string nextLineStyle_ = "solid";
    double      nextLineWidth_ = 2.0;
    double      nextPointSize_ = 3.5;

    int colorIdx_ = 0;

    std::string pickColor();
    void addSeries(std::vector<double> xs, std::vector<double> ys, bool sc);
};
