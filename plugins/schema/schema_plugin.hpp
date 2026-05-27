// schema_plugin.hpp — plugin Schema (RDM) pour flemme
// Prérequis : interpreter.hpp inclus avant ce fichier (garanti par plugins.hpp)
//
// Voir README.md dans plugins/schema/ pour la documentation complète.
#pragma once
#include "Schema.hpp"
#include <limits>
#include <memory>
#include <stdexcept>

inline void registerSchemaPlugin(Interpreter& interp) {

    const double kNaN = std::numeric_limits<double>::quiet_NaN();

    interp.registerConstructor("Schema", [kNaN]() -> Value {
        auto obj = std::make_shared<NativeObject>("Schema");
        auto sch = std::make_shared<Schema>();

        // ── Couleur courante (getter avec effet de bord) ───────────────────
        obj->getters["blanc"] = [sch]() -> Value { sch->setCouleur("blanc"); return {}; };
        obj->getters["gris0"] = [sch]() -> Value { sch->setCouleur("gris0"); return {}; };
        obj->getters["gris1"] = [sch]() -> Value { sch->setCouleur("gris1"); return {}; };
        obj->getters["gris"]  = [sch]() -> Value { sch->setCouleur("gris1"); return {}; };  // alias
        obj->getters["noir"]  = [sch]() -> Value { sch->setCouleur("noir");  return {}; };
        obj->getters["rouge"] = [sch]() -> Value { sch->setCouleur("rouge"); return {}; };
        obj->getters["bleu"]  = [sch]() -> Value { sch->setCouleur("bleu");  return {}; };
        obj->getters["vert"]  = [sch]() -> Value { sch->setCouleur("vert");  return {}; };

        // ── Vecteur ouvert / plein (getter avec effet de bord) ─────────────
        obj->getters["ouvert"] = [sch]() -> Value { sch->setOuvert(true);  return {}; };
        obj->getters["plein"]  = [sch]() -> Value { sch->setOuvert(false); return {}; };

        // ── Ligne pointillée / continue (getter avec effet de bord) ────────
        obj->getters["pointilles"] = [sch]() -> Value { sch->setPointilles(true);  return {}; };
        obj->getters["continu"]    = [sch]() -> Value { sch->setPointilles(false); return {}; };

        // ── Alignement texte (getter avec effet de bord) ───────────────────
        obj->getters["gauche"] = [sch]() -> Value { sch->setAligne(-1); return {}; };
        obj->getters["centre"] = [sch]() -> Value { sch->setAligne(0);  return {}; };
        obj->getters["droite"] = [sch]() -> Value { sch->setAligne(1);  return {}; };

        // ── Police (getter avec effet de bord) ────────────────────────────
        obj->getters["serif"]    = [sch]() -> Value { sch->setFontFamily("serif");      return {}; };
        obj->getters["sansSerif"]= [sch]() -> Value { sch->setFontFamily("sans-serif"); return {}; };
        obj->getters["mono"]     = [sch]() -> Value { sch->setFontFamily("monospace");  return {}; };
        obj->getters["gras"]     = [sch]() -> Value { sch->setFontBold(true);   return {}; };
        obj->getters["italique"] = [sch]() -> Value { sch->setFontItalic(true); return {}; };
        obj->getters["normal"]   = [sch]() -> Value { sch->setFontBold(false); sch->setFontItalic(false); return {}; };

        // ── Style courant ──────────────────────────────────────────────────
        obj->methods["fontSize"] = [sch](std::vector<Value> args) -> Value {
            if (args.empty()) throw std::runtime_error("Schema.fontSize : attend (pixels)");
            sch->setFontSize(asDouble(args[0])); return {};
        };
        obj->methods["police"] = [sch](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("Schema.police : attend (nom)");
            sch->setFontFamily(std::get<std::string>(args[0])); return {};
        };
        obj->methods["epaisseur"] = [sch](std::vector<Value> args) -> Value {
            if (args.empty()) throw std::runtime_error("Schema.epaisseur : attend (valeur)");
            sch->epaisseur(asDouble(args[0])); return {};
        };
        obj->methods["tailleVecteur"] = [sch](std::vector<Value> args) -> Value {
            if (args.empty()) throw std::runtime_error("Schema.tailleVecteur : attend (valeur)");
            sch->tailleVecteur(asDouble(args[0])); return {};
        };
        // alias de compatibilité
        obj->methods["tailleF"] = obj->methods["tailleVecteur"];

        obj->methods["taille"] = [sch](std::vector<Value> args) -> Value {
            if (args.empty()) throw std::runtime_error("Schema.taille : attend (valeur)");
            sch->setTaille(asDouble(args[0])); return {};
        };
        obj->methods["orientation"] = [sch](std::vector<Value> args) -> Value {
            if (args.empty()) throw std::runtime_error("Schema.orientation : attend (angle)");
            sch->setOrientation(asDouble(args[0])); return {};
        };
        obj->methods["aligne"] = [sch](std::vector<Value> args) -> Value {
            if (args.empty()) throw std::runtime_error("Schema.aligne : attend (-1, 0 ou 1)");
            sch->setAligne(static_cast<int>(asDouble(args[0]))); return {};
        };

        // ── Poutres / barres ───────────────────────────────────────────────
        obj->methods["poutre"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 5) throw std::runtime_error("Schema.poutre : attend (x0,y0,x1,y1,ep)");
            sch->poutre(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]),asDouble(args[3]),asDouble(args[4]));
            return {};
        };

        // ── Appuis (angle et size optionnels si s.orientation/s.taille fixés) ──
        obj->methods["appui"] = [sch,kNaN](std::vector<Value> args) -> Value {
            if (args.size() < 2) throw std::runtime_error("Schema.appui : attend (x,y[,angle,size])");
            double a  = (args.size() >= 3) ? asDouble(args[2]) : kNaN;
            double sz = (args.size() >= 4) ? asDouble(args[3]) : kNaN;
            sch->appui(asDouble(args[0]),asDouble(args[1]),a,sz);
            return {};
        };
        obj->methods["articulation"] = [sch,kNaN](std::vector<Value> args) -> Value {
            if (args.size() < 2) throw std::runtime_error("Schema.articulation : attend (x,y[,angle,size])");
            double a  = (args.size() >= 3) ? asDouble(args[2]) : kNaN;
            double sz = (args.size() >= 4) ? asDouble(args[3]) : kNaN;
            sch->articulation(asDouble(args[0]),asDouble(args[1]),a,sz);
            return {};
        };
        obj->methods["encastrement"] = [sch,kNaN](std::vector<Value> args) -> Value {
            if (args.size() < 2) throw std::runtime_error("Schema.encastrement : attend (x,y[,angle,size])");
            double a  = (args.size() >= 3) ? asDouble(args[2]) : kNaN;
            double sz = (args.size() >= 4) ? asDouble(args[3]) : kNaN;
            sch->encastrement(asDouble(args[0]),asDouble(args[1]),a,sz);
            return {};
        };
        obj->methods["appuiContinu"] = [sch,kNaN](std::vector<Value> args) -> Value {
            if (args.size() < 2) throw std::runtime_error("Schema.appuiContinu : attend (x,y[,angle,size])");
            double a  = (args.size() >= 3) ? asDouble(args[2]) : kNaN;
            double sz = (args.size() >= 4) ? asDouble(args[3]) : kNaN;
            sch->appuiContinu(asDouble(args[0]),asDouble(args[1]),a,sz);
            return {};
        };
        obj->methods["articul"] = [sch,kNaN](std::vector<Value> args) -> Value {
            if (args.size() < 2) throw std::runtime_error("Schema.articul : attend (x,y[,size])");
            double sz = (args.size() >= 3) ? asDouble(args[2]) : kNaN;
            sch->articul(asDouble(args[0]),asDouble(args[1]),sz);
            return {};
        };

        // ── Profilé UPE ────────────────────────────────────────────────────
        obj->methods["upe"] = [sch,kNaN](std::vector<Value> args) -> Value {
            if (args.size() < 2) throw std::runtime_error("Schema.upe : attend (x,y[,angle,size])");
            double a  = (args.size() >= 3) ? asDouble(args[2]) : kNaN;
            double sz = (args.size() >= 4) ? asDouble(args[3]) : kNaN;
            sch->upe(asDouble(args[0]),asDouble(args[1]),a,sz);
            return {};
        };

        // ── Charges ────────────────────────────────────────────────────────
        obj->methods["force"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 4) throw std::runtime_error("Schema.force : attend (x0,y0,x1,y1)");
            sch->force(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]),asDouble(args[3]));
            return {};
        };
        obj->methods["moment"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 5) throw std::runtime_error("Schema.moment : attend (x,y,rayon,angleDeb,angleFin)");
            sch->moment(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]),asDouble(args[3]),asDouble(args[4]));
            return {};
        };
        obj->methods["chargeRep"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 4) throw std::runtime_error("Schema.chargeRep : attend (x0,ytip,x1,ybase)");
            sch->chargeRep(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]),asDouble(args[3]));
            return {};
        };

        // ── Cotes ──────────────────────────────────────────────────────────
        obj->methods["cote"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 5 || !std::holds_alternative<std::string>(args[4]))
                throw std::runtime_error("Schema.cote : attend (x1,y1,x2,y2,label)");
            sch->cote(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]),asDouble(args[3]),
                      std::get<std::string>(args[4]));
            return {};
        };
        obj->methods["coteH"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 5 || !std::holds_alternative<std::string>(args[4]))
                throw std::runtime_error("Schema.coteH : attend (x1,y1,x2,y2,label)");
            sch->coteH(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]),asDouble(args[3]),
                       std::get<std::string>(args[4]));
            return {};
        };
        obj->methods["coteV"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 5 || !std::holds_alternative<std::string>(args[4]))
                throw std::runtime_error("Schema.coteV : attend (x1,y1,x2,y2,label)");
            sch->coteV(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]),asDouble(args[3]),
                       std::get<std::string>(args[4]));
            return {};
        };
        obj->methods["demicoteH"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 5 || !std::holds_alternative<std::string>(args[4]))
                throw std::runtime_error("Schema.demicoteH : attend (x1,y1,x2,y2,label)");
            sch->demicoteH(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]),asDouble(args[3]),
                           std::get<std::string>(args[4]));
            return {};
        };
        obj->methods["demicoteV"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 5 || !std::holds_alternative<std::string>(args[4]))
                throw std::runtime_error("Schema.demicoteV : attend (x1,y1,x2,y2,label)");
            sch->demicoteV(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]),asDouble(args[3]),
                           std::get<std::string>(args[4]));
            return {};
        };
        // alias de compatibilité
        obj->methods["halfcoteH"] = obj->methods["demicoteH"];
        obj->methods["halfcoteV"] = obj->methods["demicoteV"];

        // ── Primitives graphiques ──────────────────────────────────────────
        obj->methods["ligne"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 4) throw std::runtime_error("Schema.ligne : attend (x0,y0,x1,y1)");
            sch->ligne(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]),asDouble(args[3]));
            return {};
        };
        obj->methods["cercle"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 3) throw std::runtime_error("Schema.cercle : attend (x,y,r)");
            sch->cercle(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]));
            return {};
        };
        obj->methods["disque"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 3) throw std::runtime_error("Schema.disque : attend (x,y,r)");
            sch->disque(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]));
            return {};
        };
        obj->methods["rect"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 4) throw std::runtime_error("Schema.rect : attend (x0,y0,x1,y1)");
            sch->rect(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]),asDouble(args[3]));
            return {};
        };
        obj->methods["boite"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 4) throw std::runtime_error("Schema.boite : attend (x0,y0,x1,y1)");
            sch->boite(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]),asDouble(args[3]));
            return {};
        };
        obj->methods["solH"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 3) throw std::runtime_error("Schema.solH : attend (x0,y,x1)");
            sch->solH(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]));
            return {};
        };
        obj->methods["texte"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 3 || !std::holds_alternative<std::string>(args[2]))
                throw std::runtime_error("Schema.texte : attend (x,y,texte)");
            sch->texte(asDouble(args[0]),asDouble(args[1]),std::get<std::string>(args[2]));
            return {};
        };
        obj->methods["equation"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 3 || !std::holds_alternative<std::string>(args[2]))
                throw std::runtime_error("Schema.equation : attend (x,y,latex)");
            sch->equation(asDouble(args[0]),asDouble(args[1]),std::get<std::string>(args[2]));
            return {};
        };

        // ── Repère ─────────────────────────────────────────────────────────
        obj->methods["repere"] = [sch,kNaN](std::vector<Value> args) -> Value {
            if (args.size() < 2) throw std::runtime_error("Schema.repere : attend (x,y[,angle,size])");
            double a  = (args.size() >= 3) ? asDouble(args[2]) : kNaN;
            double sz = (args.size() >= 4) ? asDouble(args[3]) : kNaN;
            sch->repere(asDouble(args[0]),asDouble(args[1]),a,sz);
            return {};
        };
        obj->methods["repereZY"] = [sch,kNaN](std::vector<Value> args) -> Value {
            if (args.size() < 2) throw std::runtime_error("Schema.repereZY : attend (x,y[,size])");
            double sz = (args.size() >= 3) ? asDouble(args[2]) : kNaN;
            sch->repereZY(asDouble(args[0]),asDouble(args[1]),sz);
            return {};
        };

        // ── Marqueurs ──────────────────────────────────────────────────────
        obj->methods["point"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 2) throw std::runtime_error("Schema.point : attend (x,y)");
            sch->point(asDouble(args[0]),asDouble(args[1]));
            return {};
        };
        obj->methods["croix"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 2) throw std::runtime_error("Schema.croix : attend (x,y)");
            sch->croix(asDouble(args[0]),asDouble(args[1]));
            return {};
        };
        obj->methods["simplecroix"] = [sch](std::vector<Value> args) -> Value {
            if (args.size() < 3) throw std::runtime_error("Schema.simplecroix : attend (x,y,size)");
            sch->simplecroix(asDouble(args[0]),asDouble(args[1]),asDouble(args[2]));
            return {};
        };

        // ── I/O ────────────────────────────────────────────────────────────
        obj->methods["save"] = [sch](std::vector<Value> args) -> Value {
            if (args.empty() || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("Schema.save : attend un nom de fichier");
            sch->save(std::get<std::string>(args[0]));
            return {};
        };
        obj->methods["clear"] = [sch](std::vector<Value>) -> Value {
            sch->clear(); return {};
        };

        return Value{obj};
    });
}
