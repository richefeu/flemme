#pragma once
#include <cmath>
#include <limits>
#include <string>
#include <vector>

class Schema {
public:
    enum class Couleur     { Noir, Blanc, Gris0, Gris1, Rouge, Bleu, Vert };
    enum class SupportType { Appui, Articulation, Encastrement, AppuiContinu };

    // Style courant (persistant jusqu'au prochain appel ou clear)
    struct Style {
        double lw         = 1.0;   // épaisseur de trait (multiplicateur)
        double ah         = 0.0;   // taille tête de vecteur en unités monde (0=auto)
        bool   ouvert     = false; // vecteur ouvert (V angle-60) ou plein (triangle)
        bool   pointilles = false; // ligne en tirets
        double taille = std::numeric_limits<double>::quiet_NaN(); // taille liaison (NaN=non fixée)
        double orient = std::numeric_limits<double>::quiet_NaN(); // angle par défaut  (NaN=non fixé)
        int    aligne  = 0;    // alignement texte : -1=gauche, 0=centre, +1=droite
        double fontSize = 0.0; // taille du texte en px (0 = défaut 12)
        bool   bold     = false;
        bool   italic   = false;
    };

    Schema();
    void setCouleur(const std::string& nom);
    void epaisseur(double v);       // multiplicateur d'épaisseur de trait
    void tailleVecteur(double v);   // taille tête de vecteur (0=auto)
    void setTaille(double v);       // taille par défaut des liaisons (0=effacer)
    void setOrientation(double v);  // angle par défaut des liaisons
    void setOuvert(bool v);          // vecteur ouvert (true) ou plein (false)
    void setPointilles(bool v);      // ligne en tirets (true) ou continue (false)
    void setAligne(int v);           // alignement texte -1/0/+1
    void setFontFamily(const std::string& f); // famille de police
    void setFontSize(double v);      // taille en px (0 = défaut)
    void setFontBold(bool v);
    void setFontItalic(bool v);

    // ── Éléments structuraux ──────────────────────────────────────────────────
    // angle et size : NaN → résoudre via cur_st_.orient / cur_st_.taille
    void poutre(double x0, double y0, double x1, double y1, double ep);
    void appui(double x, double y,
               double angle = std::numeric_limits<double>::quiet_NaN(),
               double size  = std::numeric_limits<double>::quiet_NaN());
    void articulation(double x, double y,
                      double angle = std::numeric_limits<double>::quiet_NaN(),
                      double size  = std::numeric_limits<double>::quiet_NaN());
    void encastrement(double x, double y,
                      double angle = std::numeric_limits<double>::quiet_NaN(),
                      double size  = std::numeric_limits<double>::quiet_NaN());
    void appuiContinu(double x, double y,
                      double angle = std::numeric_limits<double>::quiet_NaN(),
                      double size  = std::numeric_limits<double>::quiet_NaN());
    void articul(double x, double y,
                 double size = std::numeric_limits<double>::quiet_NaN());
    void upe(double x, double y,
             double angle = std::numeric_limits<double>::quiet_NaN(),
             double size  = std::numeric_limits<double>::quiet_NaN());

    // ── Charges ───────────────────────────────────────────────────────────────
    void force(double x0, double y0, double x1, double y1);
    void moment(double x, double y, double rayon, double angleDeb, double angleFin);
    void chargeRep(double x0, double ytip, double x1, double ybase);

    // ── Cotes ────────────────────────────────────────────────────────────────
    void cote(double x1, double y1, double x2, double y2, const std::string& label);
    void coteH(double x1, double y1, double x2, double y2, const std::string& label);
    void coteV(double x1, double y1, double x2, double y2, const std::string& label);
    void demicoteH(double x1, double y1, double x2, double y2, const std::string& label);
    void demicoteV(double x1, double y1, double x2, double y2, const std::string& label);

    // ── Primitives graphiques ─────────────────────────────────────────────────
    void ligne(double x0, double y0, double x1, double y1);
    void cercle(double x, double y, double r);
    void disque(double x, double y, double r);
    void rect(double x0, double y0, double x1, double y1);   // rectangle contour seulement
    void boite(double x0, double y0, double x1, double y1);  // rectangle rempli (couleur courante)
    void solH(double x0, double y, double x1);               // sol haché horizontal
    void texte(double x, double y, const std::string& text);
    void equation(double x, double y, const std::string& latex); // formule LaTeX → SVG

    // ── Repère / Marqueurs ────────────────────────────────────────────────────
    void repere(double x, double y,
                double angle = std::numeric_limits<double>::quiet_NaN(),
                double size  = std::numeric_limits<double>::quiet_NaN());
    void repereZY(double x, double y,
                  double size = std::numeric_limits<double>::quiet_NaN());
    void point(double x, double y);
    void croix(double x, double y);
    void simplecroix(double x, double y, double size);

    void save(const std::string& filename) const;
    void clear();

private:
    double resolveAngle(double arg) const;
    double resolveSize(double arg) const;

    struct Poutre    { double x0,y0,x1,y1,ep; Couleur c; Style st; };
    struct Support   { double x,y,angle,size; SupportType type; Couleur c; Style st; };
    struct Upe       { double x,y,angle,size; Couleur c; Style st; };
    struct Force     { double x0,y0,x1,y1; Couleur c; Style st; };
    struct Moment    { double x,y,rayon,angleDeb,angleFin; Couleur c; Style st; };
    struct ChargeRep { double x0,ytip,x1,ybase; Couleur c; Style st; };
    struct Hinge     { double x,y,size; Couleur c; Style st; };
    struct Cote {
        double x1,y1,x2,y2;
        std::string label;
        enum class Type { Simple, H, V, DemiH, DemiV } type;
        Couleur c; Style st;
    };
    struct Repere  { double x,y,angle,size; Couleur c; Style st; bool zy = false; };
    struct Marker  { double x,y; bool cross; bool simple; double size; Couleur c; };
    struct Texte    { double x,y; std::string text; std::string fontFamily; Couleur c; Style st; };
    struct Equation { double x,y; std::string latex; std::string fontFamily; Couleur c; Style st; };
    struct Ligne    { double x0,y0,x1,y1; Couleur c; Style st; };
    struct Cercle   { double x,y,r; bool filled; Couleur c; Style st; };
    struct Rect     { double x0,y0,x1,y1; bool filled; Couleur c; Style st; };

    Couleur     cur_ = Couleur::Noir;
    Style       cur_st_;
    std::string curFont_ = "sans-serif";

    std::vector<Poutre>    poutres_;
    std::vector<Support>   supports_;
    std::vector<Upe>       upes_;
    std::vector<Force>     forces_;
    std::vector<Moment>    moments_;
    std::vector<ChargeRep> chargesRep_;
    std::vector<Hinge>     hinges_;
    std::vector<Cote>      cotes_;
    std::vector<Repere>    reperes_;
    std::vector<Marker>    markers_;
    std::vector<Texte>     textes_;
    std::vector<Equation>  equations_;
    std::vector<Ligne>     lignes_;
    std::vector<Cercle>    cercles_;
    std::vector<Rect>      rects_;
};
