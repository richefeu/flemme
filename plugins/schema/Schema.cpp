#include "Schema.hpp"
#include <array>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

static std::string xmlEscape(const std::string& s) {
    std::string out; out.reserve(s.size());
    for (char c : s) {
        if      (c == '<') out += "&lt;";
        else if (c == '>') out += "&gt;";
        else if (c == '&') out += "&amp;";
        else               out += c;
    }
    return out;
}

static constexpr int SVG_W = 700;
static constexpr int SVG_H = 400;
static constexpr int MG    = 55;
static constexpr int PW    = SVG_W - 2 * MG;
static constexpr int PH    = SVG_H - 2 * MG;

// x-positions des 6 hachures (coord. locales, commun à tous les appuis)
static constexpr std::array<double,6> kHX = {-0.5,-0.3,-0.1,0.1,0.3,0.5};

static const char* strokeColor(Schema::Couleur c) {
    switch (c) {
        case Schema::Couleur::Blanc: return "#ffffff";
        case Schema::Couleur::Gris0: return "#aaaaaa";
        case Schema::Couleur::Gris1: return "#666666";
        case Schema::Couleur::Rouge: return "#cc2222";
        case Schema::Couleur::Bleu:  return "#2266cc";
        case Schema::Couleur::Vert:  return "#228833";
        default:                     return "#111111";
    }
}

// ── Constructeur / mutateurs ──────────────────────────────────────────────────

Schema::Schema() = default;

void Schema::setCouleur(const std::string& n) {
    if      (n == "blanc" || n == "white")                         cur_ = Couleur::Blanc;
    else if (n == "gris0" || n == "grey0" || n == "gray0")         cur_ = Couleur::Gris0;
    else if (n == "gris1" || n == "grey1" || n == "gray1"
          || n == "gris"  || n == "grey"  || n == "gray")          cur_ = Couleur::Gris1;
    else if (n == "rouge" || n == "red")                           cur_ = Couleur::Rouge;
    else if (n == "bleu"  || n == "blue")                          cur_ = Couleur::Bleu;
    else if (n == "vert"  || n == "green")                         cur_ = Couleur::Vert;
    else                                                            cur_ = Couleur::Noir;
}

void Schema::epaisseur(double v)      { cur_st_.lw = v; }
void Schema::tailleVecteur(double v)  { cur_st_.ah = v; }
void Schema::setOuvert(bool v)        { cur_st_.ouvert = v; }
void Schema::setPointilles(bool v)    { cur_st_.pointilles = v; }
void Schema::setAligne(int v)         { cur_st_.aligne = v; }
void Schema::setFontFamily(const std::string& f) { curFont_ = f; }
void Schema::setFontSize(double v)    { cur_st_.fontSize = v; }
void Schema::setFontBold(bool v)      { cur_st_.bold = v; }
void Schema::setFontItalic(bool v)    { cur_st_.italic = v; }
void Schema::setTaille(double v)      { cur_st_.taille = (v > 0.0) ? v : std::numeric_limits<double>::quiet_NaN(); }
void Schema::setOrientation(double v) { cur_st_.orient = v; }

double Schema::resolveAngle(double arg) const {
    double v = std::isnan(arg) ? cur_st_.orient : arg;
    if (std::isnan(v)) throw std::runtime_error("Schema : angle manquant — utiliser s.orientation(v) ou passer l'angle");
    return v;
}
double Schema::resolveSize(double arg) const {
    double v = std::isnan(arg) ? cur_st_.taille : arg;
    if (std::isnan(v)) throw std::runtime_error("Schema : taille manquante — utiliser s.taille(v) ou passer la taille");
    return v;
}

void Schema::poutre(double x0,double y0,double x1,double y1,double ep)
    { poutres_.push_back({x0,y0,x1,y1,ep,cur_,cur_st_}); }
void Schema::appui(double x,double y,double a,double sz)
    { supports_.push_back({x,y,resolveAngle(a),resolveSize(sz),SupportType::Appui,cur_,cur_st_}); }
void Schema::articulation(double x,double y,double a,double sz)
    { supports_.push_back({x,y,resolveAngle(a),resolveSize(sz),SupportType::Articulation,cur_,cur_st_}); }
void Schema::encastrement(double x,double y,double a,double sz)
    { supports_.push_back({x,y,resolveAngle(a),resolveSize(sz),SupportType::Encastrement,cur_,cur_st_}); }
void Schema::appuiContinu(double x,double y,double a,double sz)
    { supports_.push_back({x,y,resolveAngle(a),resolveSize(sz),SupportType::AppuiContinu,cur_,cur_st_}); }
void Schema::articul(double x,double y,double sz)
    { hinges_.push_back({x,y,resolveSize(sz),cur_,cur_st_}); }
void Schema::force(double x0,double y0,double x1,double y1)
    { forces_.push_back({x0,y0,x1,y1,cur_,cur_st_}); }
void Schema::moment(double x,double y,double r,double a0,double a1)
    { moments_.push_back({x,y,r,a0,a1,cur_,cur_st_}); }
void Schema::chargeRep(double x0,double ytip,double x1,double ybase)
    { chargesRep_.push_back({x0,ytip,x1,ybase,cur_,cur_st_}); }
void Schema::cote(double x1,double y1,double x2,double y2,const std::string& lbl)
    { cotes_.push_back({x1,y1,x2,y2,lbl,Cote::Type::Simple,cur_,cur_st_}); }
void Schema::coteH(double x1,double y1,double x2,double y2,const std::string& lbl)
    { cotes_.push_back({x1,y1,x2,y2,lbl,Cote::Type::H,cur_,cur_st_}); }
void Schema::coteV(double x1,double y1,double x2,double y2,const std::string& lbl)
    { cotes_.push_back({x1,y1,x2,y2,lbl,Cote::Type::V,cur_,cur_st_}); }
void Schema::demicoteH(double x1,double y1,double x2,double y2,const std::string& lbl)
    { cotes_.push_back({x1,y1,x2,y2,lbl,Cote::Type::DemiH,cur_,cur_st_}); }
void Schema::demicoteV(double x1,double y1,double x2,double y2,const std::string& lbl)
    { cotes_.push_back({x1,y1,x2,y2,lbl,Cote::Type::DemiV,cur_,cur_st_}); }
void Schema::repere(double x,double y,double a,double sz)
    { reperes_.push_back({x,y,resolveAngle(a),resolveSize(sz),cur_,cur_st_,false}); }
void Schema::repereZY(double x,double y,double sz)
    { reperes_.push_back({x,y,0,resolveSize(sz),cur_,cur_st_,true}); }
void Schema::point(double x,double y)
    { markers_.push_back({x,y,false,false,0.0,cur_}); }
void Schema::croix(double x,double y)
    { markers_.push_back({x,y,true,false,0.0,cur_}); }
void Schema::simplecroix(double x,double y,double sz)
    { markers_.push_back({x,y,true,true,sz,cur_}); }
void Schema::ligne(double x0,double y0,double x1,double y1)
    { lignes_.push_back({x0,y0,x1,y1,cur_,cur_st_}); }
void Schema::cercle(double x,double y,double r)
    { cercles_.push_back({x,y,r,false,cur_,cur_st_}); }
void Schema::disque(double x,double y,double r)
    { cercles_.push_back({x,y,r,true,cur_,cur_st_}); }
void Schema::rect(double x0,double y0,double x1,double y1)
    { rects_.push_back({x0,y0,x1,y1,false,cur_,cur_st_}); }
void Schema::boite(double x0,double y0,double x1,double y1)
    { rects_.push_back({x0,y0,x1,y1,true,cur_,cur_st_}); }

void Schema::solH(double x0,double y,double x1) {
    if(x1 < x0) std::swap(x0, x1);
    Style continu_st = cur_st_; continu_st.pointilles = false;
    lignes_.push_back({x0,y,x1,y,cur_,continu_st});
    double len = x1 - x0;
    double h   = std::max(len * 0.04, 0.03);
    double step = h * 1.8;
    Style thin_st = continu_st; thin_st.lw = cur_st_.lw * 0.6;
    for(double xi = x0; xi <= x1 + h*0.5; xi += step) {
        double xs = std::min(xi, x1);
        lignes_.push_back({xs, y, xs - h, y - h, cur_, thin_st});
    }
}

void Schema::texte(double x,double y,const std::string& text)
    { textes_.push_back({x,y,text,curFont_,cur_,cur_st_}); }
void Schema::equation(double x,double y,const std::string& latex)
    { equations_.push_back({x,y,latex,curFont_,cur_,cur_st_}); }
void Schema::upe(double x,double y,double a,double sz)
    { upes_.push_back({x,y,resolveAngle(a),resolveSize(sz),cur_,cur_st_}); }

void Schema::clear() {
    poutres_.clear(); supports_.clear(); upes_.clear(); forces_.clear();
    moments_.clear(); chargesRep_.clear(); hinges_.clear();
    cotes_.clear(); reperes_.clear(); markers_.clear();
    textes_.clear(); equations_.clear(); lignes_.clear(); cercles_.clear(); rects_.clear();
    cur_ = Couleur::Noir;
    cur_st_ = Style{};
    curFont_ = "sans-serif";
}

// ─── Mini-renderer LaTeX → SVG ───────────────────────────────────────────────
namespace MR {

static const char* symLookup(const std::string& cmd) {
    static const std::pair<const char*,const char*> T[] = {
        {"alpha","α"},{"beta","β"},{"gamma","γ"},{"delta","δ"},
        {"epsilon","ε"},{"varepsilon","ε"},{"zeta","ζ"},{"eta","η"},
        {"theta","θ"},{"vartheta","ϑ"},{"iota","ι"},{"kappa","κ"},
        {"lambda","λ"},{"mu","μ"},{"nu","ν"},{"xi","ξ"},
        {"pi","π"},{"varpi","ϖ"},{"rho","ρ"},{"varrho","ϱ"},
        {"sigma","σ"},{"varsigma","ς"},{"tau","τ"},{"upsilon","υ"},
        {"phi","φ"},{"varphi","ϕ"},{"chi","χ"},{"psi","ψ"},{"omega","ω"},
        {"Gamma","Γ"},{"Delta","Δ"},{"Theta","Θ"},{"Lambda","Λ"},
        {"Xi","Ξ"},{"Pi","Π"},{"Sigma","Σ"},{"Upsilon","Υ"},
        {"Phi","Φ"},{"Psi","Ψ"},{"Omega","Ω"},
        {"times","×"},{"cdot","⋅"},{"cdots","⋯"},{"ldots","…"},
        {"pm","±"},{"mp","∓"},{"div","÷"},
        {"leq","≤"},{"le","≤"},{"geq","≥"},{"ge","≥"},
        {"neq","≠"},{"ne","≠"},{"approx","≈"},{"sim","∼"},
        {"infty","∞"},{"partial","∂"},{"nabla","∇"},
        {"rightarrow","→"},{"to","→"},{"leftarrow","←"},{"gets","←"},
        {"Rightarrow","⇒"},{"Leftarrow","⇐"},{"leftrightarrow","↔"},
        {"star","★"},{"ast","∗"},{"bullet","•"},{"circ","∘"},
        {"quad"," "},{"qquad","  "},
        {nullptr,nullptr}
    };
    for(int i=0;T[i].first;++i) if(cmd==T[i].first) return T[i].second;
    return nullptr;
}

static int utfLen(const std::string& s){
    int n=0;
    for(size_t j=0;j<s.size();){
        unsigned char c=(unsigned char)s[j];
        if(c<0x80)j+=1; else if(c<0xE0)j+=2; else if(c<0xF0)j+=3; else j+=4;
        ++n;
    }
    return n;
}

static double strW(const std::string& s,double fs){ return utfLen(s)*fs*0.60; }

struct Node;
using Nodes=std::vector<Node>;
struct Node {
    enum T{Txt,Sup,Sub,Frac,Seq} t=Txt;
    std::string text;
    std::vector<Nodes> ch;
};

static Node parseGroup(const std::string& s,size_t& i);
static Node parseSeq(const std::string& s,size_t& i,char until);

static Node parseSeq(const std::string& s,size_t& i,char until){
    Node seq; seq.t=Node::Seq; seq.ch.push_back({});
    Nodes& C=seq.ch[0];
    while(i<s.size()&&s[i]!=until){
        char c=s[i];
        if(c=='{'){
            ++i; Node g=parseGroup(s,i);
            if(!g.ch.empty()) for(auto& n:g.ch[0]) C.push_back(n);
        } else if(c=='^'||c=='_'){
            bool sup=(c=='^'); ++i;
            Node ss; ss.t=sup?Node::Sup:Node::Sub; ss.ch.push_back({});
            if(i<s.size()&&s[i]=='{'){++i;Node g=parseGroup(s,i);ss.ch[0]=g.ch.empty()?Nodes{}:g.ch[0];}
            else if(i<s.size()){Node t;t.t=Node::Txt;t.text=std::string(1,s[i++]);ss.ch[0].push_back(t);}
            C.push_back(ss);
        } else if(c=='\\'){
            ++i; std::string cmd;
            if(i<s.size()&&!std::isalpha((unsigned char)s[i])){cmd=std::string(1,s[i++]);}
            else{ while(i<s.size()&&std::isalpha((unsigned char)s[i])) cmd+=s[i++];
                  while(i<s.size()&&s[i]==' ') ++i; }
            if(cmd=="frac"){
                Node fr; fr.t=Node::Frac;
                if(i<s.size()&&s[i]=='{'){++i;Node g=parseGroup(s,i);fr.ch.push_back(g.ch.empty()?Nodes{}:g.ch[0]);}
                else fr.ch.push_back({});
                if(i<s.size()&&s[i]=='{'){++i;Node g=parseGroup(s,i);fr.ch.push_back(g.ch.empty()?Nodes{}:g.ch[0]);}
                else fr.ch.push_back({});
                C.push_back(fr);
            } else if(cmd=="sqrt"){
                Node t;t.t=Node::Txt;t.text="√";C.push_back(t);
                if(i<s.size()&&s[i]=='{'){++i;Node g=parseGroup(s,i);if(!g.ch.empty())for(auto& n:g.ch[0])C.push_back(n);}
            } else if(cmd=="left"||cmd=="right"||cmd=="bigl"||cmd=="bigr"||cmd=="big"||cmd=="Big"){
                if(i<s.size()){
                    char d=s[i++]; std::string sym;
                    if(d=='(')sym="("; else if(d==')')sym=")";
                    else if(d=='[')sym="["; else if(d==']')sym="]";
                    else if(d=='|')sym="|"; else if(d=='.')sym="";
                    else sym=std::string(1,d);
                    if(!sym.empty()){Node t;t.t=Node::Txt;t.text=sym;C.push_back(t);}
                }
            } else if(cmd=="text"||cmd=="mathrm"||cmd=="textrm"||cmd=="textbf"||cmd=="mathbf"||cmd=="mathit"){
                if(i<s.size()&&s[i]=='{'){++i;Node g=parseGroup(s,i);if(!g.ch.empty())for(auto& n:g.ch[0])C.push_back(n);}
            } else {
                const char* uni=symLookup(cmd);
                Node t;t.t=Node::Txt;t.text=uni?uni:("\\"+cmd);C.push_back(t);
            }
        } else if(c==' '||c=='\t'){
            Node t;t.t=Node::Txt;t.text=" ";C.push_back(t);++i;
        } else {
            Node t;t.t=Node::Txt;t.text=std::string(1,c);C.push_back(t);++i;
        }
    }
    if(i<s.size()&&s[i]==until) ++i;
    return seq;
}

static Node parseGroup(const std::string& s,size_t& i){ return parseSeq(s,i,'}'); }

static double nodesW(const Nodes& ns,double fs);
static double nodeW(const Node& n,double fs){
    switch(n.t){
    case Node::Txt: return strW(n.text,fs);
    case Node::Sup: case Node::Sub: return n.ch.empty()?0:nodesW(n.ch[0],fs*0.72);
    case Node::Frac:{
        double nw=n.ch.size()>0?nodesW(n.ch[0],fs*0.85):0;
        double dw=n.ch.size()>1?nodesW(n.ch[1],fs*0.85):0;
        return std::max(nw,dw)+fs*0.3;
    }
    case Node::Seq: return n.ch.empty()?0:nodesW(n.ch[0],fs);
    }
    return 0;
}
static double nodesW(const Nodes& ns,double fs){double w=0;for(auto& n:ns)w+=nodeW(n,fs);return w;}

static std::string fontAttr(const std::string& fam,double fs,bool bold,bool italic,const char* col){
    std::ostringstream a; a<<std::fixed<<std::setprecision(2);
    a<<" font-family=\""<<(fam.empty()?"sans-serif":fam)<<"\""
     <<" font-size=\""<<fs<<"\""
     <<" fill=\""<<col<<"\"";
    if(bold)   a<<" font-weight=\"bold\"";
    if(italic) a<<" font-style=\"italic\"";
    return a.str();
}

static double renderNodes(const Nodes& ns,double x,double y,double fs,
    const char* col,const std::string& fam,bool bold,bool italic,std::ostringstream& o);

static double renderNode(const Node& n,double x,double y,double fs,
    const char* col,const std::string& fam,bool bold,bool italic,std::ostringstream& o){
    o<<std::fixed<<std::setprecision(2);
    switch(n.t){
    case Node::Txt:{
        if(n.text.empty())return x;
        double w=strW(n.text,fs);
        bool useIt=italic||(utfLen(n.text)==1&&std::isalpha((unsigned char)n.text[0]));
        o<<"  <text x=\""<<(x+w*0.5)<<"\" y=\""<<y<<"\""
         <<" text-anchor=\"middle\" dominant-baseline=\"middle\""
         <<fontAttr(fam,fs,bold,useIt,col)
         <<">"<<xmlEscape(n.text)<<"</text>\n";
        return x+w;
    }
    case Node::Sup:
        return n.ch.empty()?x:renderNodes(n.ch[0],x,y-fs*0.52,fs*0.72,col,fam,bold,italic,o);
    case Node::Sub:
        return n.ch.empty()?x:renderNodes(n.ch[0],x,y+fs*0.38,fs*0.72,col,fam,bold,italic,o);
    case Node::Frac:{
        double sfs=fs*0.85;
        double nw=n.ch.size()>0?nodesW(n.ch[0],sfs):0;
        double dw=n.ch.size()>1?nodesW(n.ch[1],sfs):0;
        double fw=std::max(nw,dw)+sfs*0.3;
        double xm=x+fw*0.5, gap=sfs*0.65;
        o<<"  <line x1=\""<<(x+sfs*0.05)<<"\" y1=\""<<y
         <<"\" x2=\""<<(x+fw-sfs*0.05)<<"\" y2=\""<<y
         <<"\" stroke=\""<<col<<"\" stroke-width=\"1\"/>\n";
        if(n.ch.size()>0&&!n.ch[0].empty())
            renderNodes(n.ch[0],xm-nw*0.5,y-gap,sfs,col,fam,bold,italic,o);
        if(n.ch.size()>1&&!n.ch[1].empty())
            renderNodes(n.ch[1],xm-dw*0.5,y+gap,sfs,col,fam,bold,italic,o);
        return x+fw;
    }
    case Node::Seq:
        return n.ch.empty()?x:renderNodes(n.ch[0],x,y,fs,col,fam,bold,italic,o);
    }
    return x;
}

static double renderNodes(const Nodes& ns,double x,double y,double fs,
    const char* col,const std::string& fam,bool bold,bool italic,std::ostringstream& o){
    for(auto& n:ns) x=renderNode(n,x,y,fs,col,fam,bold,italic,o);
    return x;
}

// Entry point: render latex at SVG pixel (xAnchor,y), align -1/0/+1
static void render(const std::string& latex,double xAnchor,double y,
                    double fontSize,const char* col,const std::string& fam,
                    bool bold,bool italic,int align,std::ostringstream& o){
    size_t i=0;
    Node root=parseSeq(latex,i,'\0');
    const Nodes& nodes=root.ch.empty()?Nodes{}:root.ch[0];
    double totalW=nodesW(nodes,fontSize);
    double x0=(align<0)?xAnchor:(align>0)?(xAnchor-totalW):(xAnchor-totalW*0.5);
    renderNodes(nodes,x0,y,fontSize,col,fam,bold,italic,o);
}

} // namespace MR

// ── Génération SVG ────────────────────────────────────────────────────────────
void Schema::save(const std::string& filename) const {

    // ── Boîte englobante ──────────────────────────────────────────────────────
    using Wpt = std::pair<double,double>;
    std::vector<Wpt> pts;

    for (auto& p : poutres_) {
        pts.push_back({p.x0,p.y0}); pts.push_back({p.x1,p.y1});
    }
    for (auto& s : supports_) {
        double ca=std::cos(s.angle), sa=std::sin(s.angle), sz=s.size;
        auto wl=[&](double lx,double ly)->Wpt{
            return {s.x+sz*(-lx*sa-ly*ca), s.y+sz*(lx*ca-ly*sa)};
        };
        pts.push_back(wl(0,0));
        if (s.type==SupportType::Appui)          { pts.push_back(wl(-0.7,-1.5)); pts.push_back(wl(0.7,-1.5)); pts.push_back(wl(-0.5,0.1)); pts.push_back(wl(0.5,0.1)); }
        else if (s.type==SupportType::Articulation){ pts.push_back(wl(-0.7,-1.3)); pts.push_back(wl(0.7,-1.3)); pts.push_back(wl(-0.5,0.5)); pts.push_back(wl(0.5,0.5)); }
        else if (s.type==SupportType::Encastrement){ pts.push_back(wl(-0.7,-0.4)); pts.push_back(wl(0.7,-0.4)); pts.push_back(wl(-0.5,0.1)); pts.push_back(wl(0.5,0.1)); }
        else /* AppuiContinu */                  { pts.push_back(wl(-0.7,-1.2)); pts.push_back(wl(0.7,-1.2)); pts.push_back(wl(-0.5,0.1)); pts.push_back(wl(0.5,0.1)); }
    }
    for (auto& f : forces_) { pts.push_back({f.x0,f.y0}); pts.push_back({f.x1,f.y1}); }
    for (auto& m : moments_) {
        pts.push_back({m.x-m.rayon,m.y-m.rayon}); pts.push_back({m.x+m.rayon,m.y-m.rayon});
        pts.push_back({m.x-m.rayon,m.y+m.rayon}); pts.push_back({m.x+m.rayon,m.y+m.rayon});
    }
    for (auto& c : chargesRep_) {
        pts.push_back({c.x0,c.ytip}); pts.push_back({c.x1,c.ytip});
        pts.push_back({c.x0,c.ybase}); pts.push_back({c.x1,c.ybase});
    }
    for (auto& h : hinges_) {
        pts.push_back({h.x-h.size*0.5,h.y-h.size*0.5});
        pts.push_back({h.x+h.size*0.5,h.y+h.size*0.5});
    }
    for (auto& c : cotes_) { pts.push_back({c.x1,c.y1}); pts.push_back({c.x2,c.y2}); }
    for (auto& r : reperes_) {
        double ca=std::cos(r.angle),sa=std::sin(r.angle),sz=r.size;
        auto wl=[&](double lx,double ly)->Wpt{ return {r.x+sz*(-lx*sa-ly*ca),r.y+sz*(lx*ca-ly*sa)}; };
        pts.push_back(wl(1.2,0)); pts.push_back(wl(0,1.2));
        pts.push_back(wl(-0.4,-0.4));
    }
    for (auto& m : markers_) pts.push_back({m.x,m.y});
    for (auto& l : lignes_) { pts.push_back({l.x0,l.y0}); pts.push_back({l.x1,l.y1}); }
    for (auto& c : cercles_) {
        pts.push_back({c.x-c.r,c.y-c.r}); pts.push_back({c.x+c.r,c.y+c.r});
    }
    for (auto& t : textes_) pts.push_back({t.x,t.y});
    for (auto& eq: equations_) pts.push_back({eq.x,eq.y});
    for (auto& r : rects_) { pts.push_back({r.x0,r.y0}); pts.push_back({r.x1,r.y1}); }
    for (auto& u : upes_) {
        double ca=std::cos(u.angle),sa=std::sin(u.angle),sz=u.size;
        auto wl=[&](double lx,double ly)->Wpt{
            return {u.x+sz*(-lx*sa-ly*ca), u.y+sz*(lx*ca-ly*sa)};
        };
        pts.push_back(wl(0,0)); pts.push_back(wl(2,0));
        pts.push_back(wl(0,1)); pts.push_back(wl(2,1));
    }

    if (pts.empty()) pts.push_back({0.0,0.0});

    double xLo=pts[0].first, xHi=pts[0].first;
    double yLo=pts[0].second,yHi=pts[0].second;
    for (auto& [x,y]:pts){ xLo=std::min(xLo,x); xHi=std::max(xHi,x); yLo=std::min(yLo,y); yHi=std::max(yHi,y); }

    double ext_r=std::max({xHi-xLo,yHi-yLo,1e-6});
    double pad=ext_r*0.12;
    xLo-=pad; xHi+=pad; yLo-=pad; yHi+=pad;
    if(xHi-xLo<1e-9){xLo-=0.5;xHi+=0.5;}
    if(yHi-yLo<1e-9){yLo-=0.5;yHi+=0.5;}

    // ── Transform (aspect ratio 1:1) ──────────────────────────────────────────
    double rangeX=xHi-xLo, rangeY=yHi-yLo;
    double scale=std::min(PW/rangeX,PH/rangeY);
    double usedW=rangeX*scale, usedH=rangeY*scale;
    double baseX=MG+(PW-usedW)/2.0;
    double baseY=MG+(PH-usedH)/2.0;

    auto xPx=[&](double x){ return baseX+(x-xLo)*scale; };
    auto yPx=[&](double y){ return baseY+usedH-(y-yLo)*scale; };

    // ── SVG helpers ───────────────────────────────────────────────────────────
    std::ostringstream o;
    o << std::fixed << std::setprecision(2);

    // Flèche (ax,ay)→(bx,by) ; ouv=false → tête triangulaire pleine, ouv=true → V ouvert angle-60
    auto drawVecteur=[&](double ax,double ay,double bx,double by,
                         const char* col,double sw,double ah_w=0.0,bool ouv=false){
        double dx=bx-ax,dy=by-ay;
        double len=std::sqrt(dx*dx+dy*dy); if(len<1e-12)return;
        double ux=dx/len,uy=dy/len,nx=-uy,ny=ux;
        double hl=(ah_w>0)?ah_w:len*0.22, hw=hl*0.45;
        if(ouv){
            o<<"  <line x1=\""<<xPx(ax)<<"\" y1=\""<<yPx(ay)
             <<"\" x2=\""<<xPx(bx)<<"\" y2=\""<<yPx(by)
             <<"\" stroke=\""<<col<<"\" stroke-width=\""<<sw<<"\" stroke-linecap=\"round\"/>\n";
            double cos30=0.866,sin30=0.5;
            double b1x=bx-hl*(cos30*ux-sin30*uy), b1y=by-hl*(cos30*uy+sin30*ux);
            double b2x=bx-hl*(cos30*ux+sin30*uy), b2y=by-hl*(cos30*uy-sin30*ux);
            o<<"  <polyline points=\""
             <<xPx(b1x)<<","<<yPx(b1y)<<" "
             <<xPx(bx)<<","<<yPx(by)<<" "
             <<xPx(b2x)<<","<<yPx(b2y)<<"\""
             <<" fill=\"none\" stroke=\""<<col<<"\" stroke-width=\""<<sw
             <<"\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n";
        }else{
            double sx_=ax+(len-hl)*ux, sy_=ay+(len-hl)*uy;
            o<<"  <line x1=\""<<xPx(ax)<<"\" y1=\""<<yPx(ay)
             <<"\" x2=\""<<xPx(sx_)<<"\" y2=\""<<yPx(sy_)
             <<"\" stroke=\""<<col<<"\" stroke-width=\""<<sw<<"\" stroke-linecap=\"round\"/>\n";
            o<<"  <polygon points=\""
             <<xPx(bx)<<","<<yPx(by)<<" "
             <<xPx(sx_+nx*hw)<<","<<yPx(sy_+ny*hw)<<" "
             <<xPx(sx_-nx*hw)<<","<<yPx(sy_-ny*hw)
             <<"\" fill=\""<<col<<"\" stroke=\"none\"/>\n";
        }
    };

    // Flèche ouverte en V à la pointe (tip_x,tip_y) ; tangente (tx,ty) normée en coords monde
    auto drawOpenArrow=[&](double tip_x,double tip_y,
                            double tx,double ty,
                            const char* col,double sw_px,double ah_w){
        double cos30=0.866,sin30=0.5;
        double b1x=tip_x-ah_w*(cos30*tx-sin30*ty);
        double b1y=tip_y-ah_w*(cos30*ty+sin30*tx);
        double b2x=tip_x-ah_w*(cos30*tx+sin30*ty);
        double b2y=tip_y-ah_w*(cos30*ty-sin30*tx);
        o<<"  <polyline points=\""
         <<xPx(b1x)<<","<<yPx(b1y)<<" "
         <<xPx(tip_x)<<","<<yPx(tip_y)<<" "
         <<xPx(b2x)<<","<<yPx(b2y)<<"\""
         <<" fill=\"none\" stroke=\""<<col<<"\" stroke-width=\""<<sw_px
         <<"\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n";
    };

    // Double-flèche <-> de (ax,ay) vers (bx,by), couleur sombre
    auto drawDimLine=[&](double ax,double ay,double bx,double by,const char* col){
        double dx=bx-ax,dy=by-ay;
        double len=std::sqrt(dx*dx+dy*dy); if(len<1e-12)return;
        double ux=dx/len,uy=dy/len,nx=-uy,ny=ux;
        double hl=std::min(len*0.10, 10.0/scale);
        double hw=hl*0.45;
        o<<"  <line x1=\""<<xPx(ax)<<"\" y1=\""<<yPx(ay)
         <<"\" x2=\""<<xPx(bx)<<"\" y2=\""<<yPx(by)
         <<"\" stroke=\""<<col<<"\" stroke-width=\"1\"/>\n";
        // flèche à (ax,ay) pointant vers l'extérieur (direction -ux,-uy)
        double bkx=ax+ux*hl,bky=ay+uy*hl;
        o<<"  <polygon points=\""
         <<xPx(ax)<<","<<yPx(ay)<<" "
         <<xPx(bkx+nx*hw)<<","<<yPx(bky+ny*hw)<<" "
         <<xPx(bkx-nx*hw)<<","<<yPx(bky-ny*hw)
         <<"\" fill=\""<<col<<"\" stroke=\"none\"/>\n";
        // flèche à (bx,by) pointant vers l'extérieur (direction +ux,+uy)
        bkx=bx-ux*hl; bky=by-uy*hl;
        o<<"  <polygon points=\""
         <<xPx(bx)<<","<<yPx(by)<<" "
         <<xPx(bkx+nx*hw)<<","<<yPx(bky+ny*hw)<<" "
         <<xPx(bkx-nx*hw)<<","<<yPx(bky-ny*hw)
         <<"\" fill=\""<<col<<"\" stroke=\"none\"/>\n";
    };

    // Texte SVG (coordonnées en pixels SVG)
    auto svgLabel=[&](double px,double py,const std::string& txt,
                      const char* anchor="middle",double rot=0.0){
        o<<"  <text x=\""<<px<<"\" y=\""<<py<<"\""
         <<" text-anchor=\""<<anchor<<"\""
         <<" dominant-baseline=\"middle\""
         <<" font-family=\"sans-serif\" font-size=\"11\" fill=\"#444\"";
        if(std::abs(rot)>0.1)
            o<<" transform=\"rotate("<<rot<<","<<px<<","<<py<<")\"";
        o<<">"<<xmlEscape(txt)<<"</text>\n";
    };

    // ── Début du fichier SVG ──────────────────────────────────────────────────
    o<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
     <<"<svg xmlns=\"http://www.w3.org/2000/svg\""
     <<" width=\""<<SVG_W<<"\" height=\""<<SVG_H<<"\""
     <<" viewBox=\"0 0 "<<SVG_W<<" "<<SVG_H<<"\">\n"
     <<"  <rect width=\""<<SVG_W<<"\" height=\""<<SVG_H<<"\" fill=\"white\"/>\n";

    // ── Rectangles (boites remplies / contours) ───────────────────────────────
    for(auto& r:rects_){
        double rx1=std::min(xPx(r.x0),xPx(r.x1));
        double ry1=std::min(yPx(r.y0),yPx(r.y1));
        double rw=std::abs(xPx(r.x1)-xPx(r.x0));
        double rh=std::abs(yPx(r.y1)-yPx(r.y0));
        const char* col=strokeColor(r.c);
        o<<"  <rect x=\""<<rx1<<"\" y=\""<<ry1
         <<"\" width=\""<<rw<<"\" height=\""<<rh<<"\"";
        if(r.filled)
            o<<" fill=\""<<col<<"\" stroke=\"#111111\" stroke-width=\""<<1.5*r.st.lw<<"\"";
        else
            o<<" fill=\"none\" stroke=\""<<col<<"\" stroke-width=\""<<1.5*r.st.lw<<"\"";
        o<<"/>\n";
    }

    // ── Lignes fines (contours de sections) ──────────────────────────────────
    for(auto& l:lignes_){
        o<<"  <line x1=\""<<xPx(l.x0)<<"\" y1=\""<<yPx(l.y0)
         <<"\" x2=\""<<xPx(l.x1)<<"\" y2=\""<<yPx(l.y1)
         <<"\" stroke=\""<<strokeColor(l.c)<<"\" stroke-width=\""<<1.5*l.st.lw
         <<"\" stroke-linecap=\"round\"";
        if(l.st.pointilles) o<<" stroke-dasharray=\"6,3\"";
        o<<"/>\n";
    }

    // ── Cercles / Disques ─────────────────────────────────────────────────────
    for(auto& c:cercles_){
        double rr=c.r*scale;
        o<<"  <circle cx=\""<<xPx(c.x)<<"\" cy=\""<<yPx(c.y)<<"\" r=\""<<rr<<"\"";
        if(c.filled)
            o<<" fill=\""<<strokeColor(c.c)<<"\" stroke=\"none\"";
        else
            o<<" fill=\"white\" stroke=\""<<strokeColor(c.c)
             <<"\" stroke-width=\""<<1.5*c.st.lw<<"\"";
        o<<"/>\n";
    }

    // ── Poutres : trait épais avec extrémités arrondies ───────────────────────
    for(auto& p:poutres_){
        double dx=p.x1-p.x0,dy=p.y1-p.y0;
        if(std::sqrt(dx*dx+dy*dy)<1e-12)continue;
        const char* col=strokeColor(p.c);
        double sw=std::max(2.5,p.ep*scale)*p.st.lw;
        o<<"  <line x1=\""<<xPx(p.x0)<<"\" y1=\""<<yPx(p.y0)
         <<"\" x2=\""<<xPx(p.x1)<<"\" y2=\""<<yPx(p.y1)
         <<"\" stroke=\""<<col<<"\" stroke-width=\""<<sw
         <<"\" stroke-linecap=\"round\"/>\n";
    }

    // ── Appuis / Articulations / Encastrements / AppuiContinu ─────────────────
    for(auto& s:supports_){
        double ca=std::cos(s.angle),sa=std::sin(s.angle),sz=s.size;
        const char* col=strokeColor(s.c);

        // Coordonnées locales → pixels SVG
        auto svgpt=[&](double lx,double ly)->std::pair<double,double>{
            double wx=s.x+sz*(-lx*sa-ly*ca);
            double wy=s.y+sz*( lx*ca-ly*sa);
            return {xPx(wx),yPx(wy)};
        };
        double ssw=1.5*s.st.lw;
        auto drawBaseLine=[&](double yl){
            auto[l1x,l1y]=svgpt(-0.5,yl); auto[r1x,r1y]=svgpt(0.5,yl);
            o<<"  <line x1=\""<<l1x<<"\" y1=\""<<l1y<<"\" x2=\""<<r1x<<"\" y2=\""<<r1y
             <<"\" stroke=\""<<col<<"\" stroke-width=\""<<ssw<<"\"/>\n";
        };
        auto drawHatches=[&](double hy,double hdx,double hdy){
            for(double hxi:kHX){
                auto[h0x,h0y]=svgpt(hxi,hy); auto[h1x,h1y]=svgpt(hxi+hdx,hy+hdy);
                o<<"  <line x1=\""<<h0x<<"\" y1=\""<<h0y<<"\" x2=\""<<h1x<<"\" y2=\""<<h1y
                 <<"\" stroke=\""<<col<<"\" stroke-width=\""<<s.st.lw<<"\"/>\n";
            }
        };

        if(s.type==SupportType::Appui){
            // Triangle apex→base
            auto[ax,ay]=svgpt(0,0); auto[blx,bly]=svgpt(-0.5,-0.866); auto[brx,bry]=svgpt(0.5,-0.866);
            o<<"  <polygon points=\""<<ax<<","<<ay<<" "<<blx<<","<<bly<<" "<<brx<<","<<bry
             <<"\" fill=\"white\" stroke=\""<<col<<"\" stroke-width=\""<<ssw<<"\" stroke-linejoin=\"round\"/>\n";
            // 3 rouleaux
            double rr=sz*0.175*scale;
            for(int k=-1;k<=1;k++){
                auto[cx,cy]=svgpt(0.35*k,-1.041);
                o<<"  <circle cx=\""<<cx<<"\" cy=\""<<cy<<"\" r=\""<<rr
                 <<"\" fill=\"white\" stroke=\""<<col<<"\" stroke-width=\""<<s.st.lw<<"\"/>\n";
            }
            drawBaseLine(-1.216);
            drawHatches(-1.216,-0.2,-0.2);

        }else if(s.type==SupportType::Articulation){
            // Grand cercle
            auto[cx,cy]=svgpt(0,-0.5);
            o<<"  <circle cx=\""<<cx<<"\" cy=\""<<cy<<"\" r=\""<<sz*0.5*scale
             <<"\" fill=\"white\" stroke=\""<<col<<"\" stroke-width=\""<<ssw<<"\"/>\n";
            drawBaseLine(-1.0);
            drawHatches(-1.0,-0.2,-0.2);

        }else if(s.type==SupportType::Encastrement){
            // Talon
            auto[s0x,s0y]=svgpt(0,0); auto[s1x,s1y]=svgpt(0,-0.1);
            o<<"  <line x1=\""<<s0x<<"\" y1=\""<<s0y<<"\" x2=\""<<s1x<<"\" y2=\""<<s1y
             <<"\" stroke=\""<<col<<"\" stroke-width=\""<<2.5*s.st.lw<<"\"/>\n";
            drawBaseLine(-0.1);
            drawHatches(-0.1,-0.2,-0.2);

        }else{ // AppuiContinu : triangle + hachures directes (pas de rouleaux ni sol)
            auto[ax,ay]=svgpt(0,0); auto[blx,bly]=svgpt(-0.5,-0.866); auto[brx,bry]=svgpt(0.5,-0.866);
            o<<"  <polygon points=\""<<ax<<","<<ay<<" "<<blx<<","<<bly<<" "<<brx<<","<<bry
             <<"\" fill=\"white\" stroke=\""<<col<<"\" stroke-width=\""<<ssw<<"\" stroke-linejoin=\"round\"/>\n";
            drawHatches(-0.866,-0.2,-0.2);
        }
    }

    // ── Profilés UPE (canal) ─────────────────────────────────────────────────
    // Contour simplifié (sans arcs aux congés) en coords locales 0..2 × 0..1
    static constexpr double kUpe[][2] = {
        {0,0},{2,0},{2,1},{1.8,1},{1.8,0.2},{0.2,0.2},{0.2,1},{0,1},{0,0}
    };
    for(auto& u:upes_){
        double ca=std::cos(u.angle),sa=std::sin(u.angle),sz=u.size;
        auto wl=[&](double lx,double ly)->std::pair<double,double>{
            return {u.x+sz*(-lx*sa-ly*ca), u.y+sz*(lx*ca-ly*sa)};
        };
        const char* col=strokeColor(u.c);
        double sw=1.5*u.st.lw;
        for(int i=0;i<8;++i){
            auto[ax,ay]=wl(kUpe[i][0],kUpe[i][1]);
            auto[bx,by]=wl(kUpe[i+1][0],kUpe[i+1][1]);
            o<<"  <line x1=\""<<xPx(ax)<<"\" y1=\""<<yPx(ay)
             <<"\" x2=\""<<xPx(bx)<<"\" y2=\""<<yPx(by)
             <<"\" stroke=\""<<col<<"\" stroke-width=\""<<sw<<"\" stroke-linecap=\"round\"/>\n";
        }
    }

    // ── Forces ────────────────────────────────────────────────────────────────
    for(auto& f:forces_)
        drawVecteur(f.x0,f.y0,f.x1,f.y1,strokeColor(f.c),2.2*f.st.lw,f.st.ah,f.st.ouvert);

    // ── Moments : arc avec flèche ouverte ────────────────────────────────────
    for(auto& m:moments_){
        double a0=m.angleDeb,a1=m.angleFin,r=m.rayon;
        const char* col=strokeColor(m.c);
        double sw=2.0*m.st.lw;
        double sx_m=m.x+r*std::cos(a0),sy_m=m.y+r*std::sin(a0);
        double ex_m=m.x+r*std::cos(a1),ey_m=m.y+r*std::sin(a1);
        bool ccw=(a1>=a0);
        double span=ccw?(a1-a0):(a0-a1);
        int large=(span>M_PI)?1:0, sweep=ccw?0:1;
        o<<"  <path d=\"M "<<xPx(sx_m)<<" "<<yPx(sy_m)
         <<" A "<<r*scale<<" "<<r*scale<<" 0 "<<large<<" "<<sweep
         <<" "<<xPx(ex_m)<<" "<<yPx(ey_m)<<"\""
         <<" fill=\"none\" stroke=\""<<col<<"\" stroke-width=\""<<sw<<"\"/>\n";
        double tx=ccw?-std::sin(a1):std::sin(a1), ty=ccw?std::cos(a1):-std::cos(a1);
        double ah=(m.st.ah>0)?m.st.ah:std::max(r*0.18,6.0/scale);
        drawOpenArrow(ex_m,ey_m,tx,ty,col,sw,ah);
    }

    // ── Charges réparties ─────────────────────────────────────────────────────
    for(auto& c:chargesRep_){
        const char* col=strokeColor(c.c);
        double sw=1.3*c.st.lw;
        double dx=c.x1-c.x0;
        o<<"  <line x1=\""<<xPx(c.x0)<<"\" y1=\""<<yPx(c.ybase)
         <<"\" x2=\""<<xPx(c.x1)<<"\" y2=\""<<yPx(c.ybase)
         <<"\" stroke=\""<<col<<"\" stroke-width=\""<<sw<<"\"/>\n";
        for(int k=0;k<=4;k++){
            double xi=c.x0+dx*k/4.0;
            drawVecteur(xi,c.ybase,xi,c.ytip,col,sw,c.st.ah,c.st.ouvert);
        }
    }

    // ── Articulations internes (cercles seuls) ────────────────────────────────
    for(auto& h:hinges_){
        o<<"  <circle cx=\""<<xPx(h.x)<<"\" cy=\""<<yPx(h.y)
         <<"\" r=\""<<h.size*0.5*scale
         <<"\" fill=\"white\" stroke=\""<<strokeColor(h.c)
         <<"\" stroke-width=\""<<1.5*h.st.lw<<"\"/>\n";
    }

    // ── Cotes ─────────────────────────────────────────────────────────────────
    static const char* kCoteCol="#555555";
    for(auto& c:cotes_){
        if(c.type==Cote::Type::Simple){
            double mx=(c.x1+c.x2)*0.5, my=(c.y1+c.y2)*0.5;
            double dx=c.x2-c.x1,dy=c.y2-c.y1;
            double len=std::sqrt(dx*dx+dy*dy); if(len<1e-12)continue;
            double ux=dx/len,uy=dy/len;
            drawDimLine(c.x1,c.y1,c.x2,c.y2,kCoteCol);
            // Perpendiculaire vers le "haut" SVG : offset (-uy,-ux) en pixels
            svgLabel(xPx(mx)-uy*12, yPx(my)-ux*12, c.label);

        }else if(c.type==Cote::Type::H){
            // Lignes d'extension verticales, double-flèche horizontale à y2
            double thin=0.5;
            o<<"  <line x1=\""<<xPx(c.x1)<<"\" y1=\""<<yPx(c.y1)
             <<"\" x2=\""<<xPx(c.x1)<<"\" y2=\""<<yPx(c.y2)
             <<"\" stroke=\""<<kCoteCol<<"\" stroke-width=\""<<thin<<"\"/>\n";
            o<<"  <line x1=\""<<xPx(c.x2)<<"\" y1=\""<<yPx(c.y1)
             <<"\" x2=\""<<xPx(c.x2)<<"\" y2=\""<<yPx(c.y2)
             <<"\" stroke=\""<<kCoteCol<<"\" stroke-width=\""<<thin<<"\"/>\n";
            drawDimLine(c.x1,c.y2,c.x2,c.y2,kCoteCol);
            svgLabel(xPx((c.x1+c.x2)*0.5),yPx(c.y2)-14,c.label);

        }else if(c.type==Cote::Type::V){
            // Lignes d'extension horizontales, double-flèche verticale à x2
            double thin=0.5;
            o<<"  <line x1=\""<<xPx(c.x1)<<"\" y1=\""<<yPx(c.y1)
             <<"\" x2=\""<<xPx(c.x2)<<"\" y2=\""<<yPx(c.y1)
             <<"\" stroke=\""<<kCoteCol<<"\" stroke-width=\""<<thin<<"\"/>\n";
            o<<"  <line x1=\""<<xPx(c.x1)<<"\" y1=\""<<yPx(c.y2)
             <<"\" x2=\""<<xPx(c.x2)<<"\" y2=\""<<yPx(c.y2)
             <<"\" stroke=\""<<kCoteCol<<"\" stroke-width=\""<<thin<<"\"/>\n";
            drawDimLine(c.x2,c.y1,c.x2,c.y2,kCoteCol);
            double mpy=yPx((c.y1+c.y2)*0.5);
            svgLabel(xPx(c.x2)+16,mpy,c.label,"middle",-90);

        }else if(c.type==Cote::Type::DemiH){
            // Extensions + tiret de départ + flèche simple x1→x2
            double thin=0.5;
            o<<"  <line x1=\""<<xPx(c.x1)<<"\" y1=\""<<yPx(c.y1)
             <<"\" x2=\""<<xPx(c.x1)<<"\" y2=\""<<yPx(c.y2)
             <<"\" stroke=\""<<kCoteCol<<"\" stroke-width=\""<<thin<<"\"/>\n";
            o<<"  <line x1=\""<<xPx(c.x2)<<"\" y1=\""<<yPx(c.y1)
             <<"\" x2=\""<<xPx(c.x2)<<"\" y2=\""<<yPx(c.y2)
             <<"\" stroke=\""<<kCoteCol<<"\" stroke-width=\""<<thin<<"\"/>\n";
            // tiret à l'origine (x1,y2)
            o<<"  <line x1=\""<<xPx(c.x1)<<"\" y1=\""<<(yPx(c.y2)-4)
             <<"\" x2=\""<<xPx(c.x1)<<"\" y2=\""<<(yPx(c.y2)+4)
             <<"\" stroke=\""<<kCoteCol<<"\" stroke-width=\"2\"/>\n";
            drawVecteur(c.x1,c.y2,c.x2,c.y2,kCoteCol,1.0);
            svgLabel(xPx((c.x1+c.x2)*0.5),yPx(c.y2)-14,c.label);

        }else{ // DemiV
            // Extensions + tiret de départ + flèche simple y1→y2
            double thin=0.5;
            o<<"  <line x1=\""<<xPx(c.x1)<<"\" y1=\""<<yPx(c.y1)
             <<"\" x2=\""<<xPx(c.x2)<<"\" y2=\""<<yPx(c.y1)
             <<"\" stroke=\""<<kCoteCol<<"\" stroke-width=\""<<thin<<"\"/>\n";
            o<<"  <line x1=\""<<xPx(c.x1)<<"\" y1=\""<<yPx(c.y2)
             <<"\" x2=\""<<xPx(c.x2)<<"\" y2=\""<<yPx(c.y2)
             <<"\" stroke=\""<<kCoteCol<<"\" stroke-width=\""<<thin<<"\"/>\n";
            // tiret à l'origine (x2,y1)
            o<<"  <line x1=\""<<(xPx(c.x2)-4)<<"\" y1=\""<<yPx(c.y1)
             <<"\" x2=\""<<(xPx(c.x2)+4)<<"\" y2=\""<<yPx(c.y1)
             <<"\" stroke=\""<<kCoteCol<<"\" stroke-width=\"2\"/>\n";
            drawVecteur(c.x2,c.y1,c.x2,c.y2,kCoteCol,1.0);
            double mpy=yPx((c.y1+c.y2)*0.5);
            svgLabel(xPx(c.x2)+16,mpy,c.label,"middle",-90);
        }
    }

    // ── Repères d'axes ────────────────────────────────────────────────────────
    for(auto& r:reperes_){
        const char* col=strokeColor(r.c);
        double sz=r.size;

        if(r.zy){
            // Repère ZY : Z pointe à gauche, Y pointe en haut (axes de section)
            drawVecteur(r.x,r.y, r.x-sz,r.y,  col,1.2,sz*0.18);
            drawVecteur(r.x,r.y, r.x,r.y+sz,  col,1.2,sz*0.18);
            svgLabel(xPx(r.x-sz*1.15),yPx(r.y),"Z");
            svgLabel(xPx(r.x),yPx(r.y+sz*1.15),"Y");
        } else {
            double ca=std::cos(r.angle),sa=std::sin(r.angle);
            auto wl=[&](double lx,double ly)->std::pair<double,double>{
                return {r.x+sz*(-lx*sa-ly*ca), r.y+sz*(lx*ca-ly*sa)};
            };
            auto[ox,oy]=wl(0,0);
            auto[xx,xy]=wl(1,0); drawVecteur(ox,oy,xx,xy,col,1.2,sz*0.18);
            auto[yx,yy]=wl(0,1); drawVecteur(ox,oy,yx,yy,col,1.2,sz*0.18);
            // Arc CCW : −45° à 135° en local
            auto[arcsx,arcsy]=wl( 0.3536,-0.3536);
            auto[arcex,arcey]=wl(-0.3536, 0.3536);
            double eps=0.01;
            auto[p1wx,p1wy]=wl(0.5*std::cos(3*M_PI/4-eps),0.5*std::sin(3*M_PI/4-eps));
            auto[p2wx,p2wy]=wl(0.5*std::cos(3*M_PI/4+eps),0.5*std::sin(3*M_PI/4+eps));
            double tdx=p2wx-p1wx,tdy=p2wy-p1wy,tlen=std::sqrt(tdx*tdx+tdy*tdy);
            double tux=tdx/tlen,tuy=tdy/tlen,tnx=-tuy,tny=tux;
            double arcrpx=sz*0.5*scale,ahl=sz*0.12,ahw=ahl*0.4;
            o<<"  <path d=\"M "<<xPx(arcsx)<<" "<<yPx(arcsy)
             <<" A "<<arcrpx<<" "<<arcrpx<<" 0 0 0 "<<xPx(arcex)<<" "<<yPx(arcey)<<"\""
             <<" fill=\"none\" stroke=\""<<col<<"\" stroke-width=\"1.2\"/>\n";
            o<<"  <polygon points=\""
             <<xPx(arcex)<<","<<yPx(arcey)<<" "
             <<xPx(arcex-tux*ahl+tnx*ahw)<<","<<yPx(arcey-tuy*ahl+tny*ahw)<<" "
             <<xPx(arcex-tux*ahl-tnx*ahw)<<","<<yPx(arcey-tuy*ahl-tny*ahw)
             <<"\" fill=\""<<col<<"\" stroke=\"none\"/>\n";
            auto[lxx,lxy]=wl(1.18,-0.06);
            auto[lyx,lyy]=wl(-0.06,1.18);
            svgLabel(xPx(lxx),yPx(lxy),"X");
            svgLabel(xPx(lyx),yPx(lyy),"Y");
        }
    }

    // ── Marqueurs : point/croix (orange) et simplecroix (couleur courante) ──────
    for(auto& m:markers_){
        double px=xPx(m.x),py=yPx(m.y);
        if(m.simple){
            // simplecroix : couleur courante, taille en unités monde
            double d=m.size>0 ? m.size*scale : std::max(3.0,0.06*scale);
            const char* col=strokeColor(m.c);
            o<<"  <line x1=\""<<(px-d)<<"\" y1=\""<<(py-d)
             <<"\" x2=\""<<(px+d)<<"\" y2=\""<<(py+d)
             <<"\" stroke=\""<<col<<"\" stroke-width=\"2\" stroke-linecap=\"round\"/>\n";
            o<<"  <line x1=\""<<(px-d)<<"\" y1=\""<<(py+d)
             <<"\" x2=\""<<(px+d)<<"\" y2=\""<<(py-d)
             <<"\" stroke=\""<<col<<"\" stroke-width=\"2\" stroke-linecap=\"round\"/>\n";
        }else if(!m.cross){
            double pr=std::max(3.0,0.06*scale);
            o<<"  <circle cx=\""<<px<<"\" cy=\""<<py<<"\" r=\""<<pr
             <<"\" fill=\"none\" stroke=\"#ff8000\" stroke-width=\"1.5\"/>\n";
        }else{
            double d=std::max(3.0,0.06*scale)*0.85;
            o<<"  <line x1=\""<<(px-d)<<"\" y1=\""<<(py-d)
             <<"\" x2=\""<<(px+d)<<"\" y2=\""<<(py+d)
             <<"\" stroke=\"#ff8000\" stroke-width=\"2\" stroke-linecap=\"round\"/>\n";
            o<<"  <line x1=\""<<(px-d)<<"\" y1=\""<<(py+d)
             <<"\" x2=\""<<(px+d)<<"\" y2=\""<<(py-d)
             <<"\" stroke=\"#ff8000\" stroke-width=\"2\" stroke-linecap=\"round\"/>\n";
        }
    }

    // ── Textes ────────────────────────────────────────────────────────────────
    for(auto& t:textes_){
        const char* anchor=(t.st.aligne<0)?"start":(t.st.aligne>0)?"end":"middle";
        double fs = (t.st.fontSize>0) ? t.st.fontSize : 12.0;
        const std::string& fam = t.fontFamily.empty() ? "sans-serif" : t.fontFamily;
        o<<"  <text x=\""<<xPx(t.x)<<"\" y=\""<<yPx(t.y)<<"\""
         <<" text-anchor=\""<<anchor<<"\" dominant-baseline=\"middle\""
         <<" font-family=\""<<fam<<"\" font-size=\""<<fs<<"\""
         <<" fill=\""<<strokeColor(t.c)<<"\"";
        if(t.st.bold)   o<<" font-weight=\"bold\"";
        if(t.st.italic) o<<" font-style=\"italic\"";
        o<<">"<<xmlEscape(t.text)<<"</text>\n";
    }

    // ── Équations (LaTeX → SVG) ───────────────────────────────────────────────
    for(auto& eq:equations_){
        double fs=(eq.st.fontSize>0)?eq.st.fontSize:14.0;
        MR::render(eq.latex, xPx(eq.x), yPx(eq.y), fs,
                   strokeColor(eq.c), eq.fontFamily,
                   eq.st.bold, eq.st.italic, eq.st.aligne, o);
    }

    o<<"</svg>\n";

    std::ofstream file(filename);
    if(!file.is_open())
        throw std::runtime_error("Schema.save : impossible d'écrire dans : "+filename);
    file<<o.str();
}
