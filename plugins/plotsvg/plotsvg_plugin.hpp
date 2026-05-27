// plotsvg_plugin.hpp — plugin PlotSVG pour flemme
// Prérequis : interpreter.hpp inclus avant ce fichier (garanti par plugins.hpp)
//
// Usage dans un script .flm :
//   let plt = PlotSVG();
//   let x = [0.0, 1.0, 2.0, 3.0];
//   let y = [0.0, 1.0, 4.0, 9.0];
//   plt.setTitle("Ma courbe");
//   plt.setLabelX("x");
//   plt.setLabelY("f(x)");
//   plt.plot(x, y);
//   plt.save("out.svg");
//
//   plt.clear();
//   plt.setColor("#e74c3c");
//   plt.scatter(x, y);
//   plt.save("nuage.svg");
#pragma once
#include "PlotSVG.hpp"
#include <memory>
#include <stdexcept>

inline void registerPlotSVGPlugin(Interpreter& interp) {

    // Helper: extraire un vecteur de doubles depuis une Value tableau
    auto extractVec = [](const Value& v, const std::string& ctx) -> std::vector<double> {
        ArrayObject* arr = getArray(v);
        if (!arr) throw std::runtime_error(ctx + " : attend un tableau de nombres");
        std::vector<double> out;
        out.reserve(arr->elements.size());
        for (auto& e : arr->elements)
            out.push_back(asDouble(e));
        return out;
    };

    interp.registerConstructor("PlotSVG", [extractVec]() -> Value {
        auto obj = std::make_shared<NativeObject>("PlotSVG");
        auto plt = std::make_shared<PlotSVG>();

        // plt.plot(xs, ys)
        obj->methods["plot"] = [plt, extractVec](std::vector<Value> args) -> Value {
            if (args.size() < 2)
                throw std::runtime_error("PlotSVG.plot : attend deux tableaux (xs, ys)");
            plt->plot(extractVec(args[0], "PlotSVG.plot xs"),
                      extractVec(args[1], "PlotSVG.plot ys"));
            return {};
        };

        // plt.scatter(xs, ys)
        obj->methods["scatter"] = [plt, extractVec](std::vector<Value> args) -> Value {
            if (args.size() < 2)
                throw std::runtime_error("PlotSVG.scatter : attend deux tableaux (xs, ys)");
            plt->scatter(extractVec(args[0], "PlotSVG.scatter xs"),
                         extractVec(args[1], "PlotSVG.scatter ys"));
            return {};
        };

        // plt.setTitle(s)
        obj->methods["setTitle"] = [plt](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("PlotSVG.setTitle : attend une chaîne");
            plt->setTitle(std::get<std::string>(args[0]));
            return {};
        };

        // plt.setLabelX(s)
        obj->methods["setLabelX"] = [plt](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("PlotSVG.setLabelX : attend une chaîne");
            plt->setLabelX(std::get<std::string>(args[0]));
            return {};
        };

        // plt.setLabelY(s)
        obj->methods["setLabelY"] = [plt](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("PlotSVG.setLabelY : attend une chaîne");
            plt->setLabelY(std::get<std::string>(args[0]));
            return {};
        };

        // plt.setColor(s)  — s'applique à la prochaine série ajoutée
        obj->methods["setColor"] = [plt](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("PlotSVG.setColor : attend une chaîne (couleur CSS)");
            plt->setColor(std::get<std::string>(args[0]));
            return {};
        };

        // plt.setLabel(s)  — s'applique à la prochaine série ajoutée
        obj->methods["setLabel"] = [plt](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("PlotSVG.setLabel : attend une chaîne");
            plt->setLabel(std::get<std::string>(args[0]));
            return {};
        };

        // plt.setFill(color)  — aire sous la prochaine courbe (jusqu'à y=0)
        obj->methods["setFill"] = [plt](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("PlotSVG.setFill : attend une chaîne (couleur CSS)");
            plt->setFill(std::get<std::string>(args[0]));
            return {};
        };

        // plt.setLineStyle(style)  — "solid" | "dashed" | "dotted" (persistant)
        obj->methods["setLineStyle"] = [plt](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("PlotSVG.setLineStyle : attend \"solid\", \"dashed\" ou \"dotted\"");
            plt->setLineStyle(std::get<std::string>(args[0]));
            return {};
        };

        // plt.setLineWidth(w)  — épaisseur du trait (persistant, défaut 2.0)
        obj->methods["setLineWidth"] = [plt](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<double>(args[0]))
                throw std::runtime_error("PlotSVG.setLineWidth : attend un nombre");
            plt->setLineWidth(asDouble(args[0]));
            return {};
        };

        // plt.setPointSize(r)  — rayon des points scatter (persistant, défaut 3.5)
        obj->methods["setPointSize"] = [plt](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<double>(args[0]))
                throw std::runtime_error("PlotSVG.setPointSize : attend un nombre");
            plt->setPointSize(asDouble(args[0]));
            return {};
        };

        // plt.hline(y [, color [, style]])
        obj->methods["hline"] = [plt](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<double>(args[0]))
                throw std::runtime_error("PlotSVG.hline : attend au moins un nombre (y)");
            std::string color = "#888888", style = "dashed";
            if (args.size() >= 2 && std::holds_alternative<std::string>(args[1]))
                color = std::get<std::string>(args[1]);
            if (args.size() >= 3 && std::holds_alternative<std::string>(args[2]))
                style = std::get<std::string>(args[2]);
            plt->hline(asDouble(args[0]), color, style);
            return {};
        };

        // plt.vline(x [, color [, style]])
        obj->methods["vline"] = [plt](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<double>(args[0]))
                throw std::runtime_error("PlotSVG.vline : attend au moins un nombre (x)");
            std::string color = "#888888", style = "dashed";
            if (args.size() >= 2 && std::holds_alternative<std::string>(args[1]))
                color = std::get<std::string>(args[1]);
            if (args.size() >= 3 && std::holds_alternative<std::string>(args[2]))
                style = std::get<std::string>(args[2]);
            plt->vline(asDouble(args[0]), color, style);
            return {};
        };

        // plt.setXRange(lo, hi)
        obj->methods["setXRange"] = [plt](std::vector<Value> args) -> Value {
            if (args.size() < 2)
                throw std::runtime_error("PlotSVG.setXRange : attend deux nombres (lo, hi)");
            plt->setXRange(asDouble(args[0]), asDouble(args[1]));
            return {};
        };

        // plt.setYRange(lo, hi)
        obj->methods["setYRange"] = [plt](std::vector<Value> args) -> Value {
            if (args.size() < 2)
                throw std::runtime_error("PlotSVG.setYRange : attend deux nombres (lo, hi)");
            plt->setYRange(asDouble(args[0]), asDouble(args[1]));
            return {};
        };

        // plt.save(filename)
        obj->methods["save"] = [plt](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("PlotSVG.save : attend un nom de fichier");
            plt->save(std::get<std::string>(args[0]));
            return {};
        };

        // plt.clear()
        obj->methods["clear"] = [plt](std::vector<Value>) -> Value {
            plt->clear();
            return {};
        };

        return Value{obj};
    });
}
