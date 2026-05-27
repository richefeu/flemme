// lexer.hpp
#pragma once
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// ------------------------------------------------
// TOKENS
// ------------------------------------------------
enum class TokenType {
  // Mots-clés
  LET,
  FN,
  RETURN,
  WHILE,
  FOR,
  IN,
  BREAK,
  CONTINUE,
  IF,
  ELSE,
  PRINT,

  // Littéraux
  NOMBRE_ENTIER,  // 42
  NOMBRE_DECIMAL, // 3.14
  CHAINE,         // "hello"

  // Identifiant
  IDENTIFIANT, // x, add, maVariable

  // Opérateurs arithmétiques
  PLUS,        // +
  MOINS,       // -
  ETOILE,      // *
  SLASH,       // /
  MODULO,      // %
  PUISSANCE,   // ^
  SLASH_SLASH, // // (division entière)

  // Opérateurs d'affectation composée
  PLUS_EGAL,   // +=
  MOINS_EGAL,  // -=
  ETOILE_EGAL, // *=
  SLASH_EGAL,  // /=

  // Incrémentation / décrémentation
  PLUS_PLUS,   // ++
  MOINS_MOINS, // --

  // Opérateurs logiques
  NON_LOGIQUE, // !
  ET_LOGIQUE,  // &&
  OU_LOGIQUE,  // ||

  // Opérateurs de comparaison
  EGAL,      // ==
  DIFFERENT, // !=
  INF,       // <
  SUP,       // >
  INF_EGAL,  // <=
  SUP_EGAL,  // >=

  // Affectation
  AFFECTATION, // =

  // Ponctuation
  PARENTHESE_OUV,  // (
  PARENTHESE_FERM, // )
  ACCOLADE_OUV,    // {
  ACCOLADE_FERM,   // }
  CROCHET_OUV,     // [
  CROCHET_FERM,    // ]
  VIRGULE,         // ,
  POINT_VIRGULE,   // ;
  POINT,           // .

  // Spéciaux
  FIN,    // fin du fichier
  INCONNU // caractère non reconnu
};

// ------------------------------------------------
// TOKEN
// ------------------------------------------------
struct Token {
  TokenType type;
  std::string valeur;
  int ligne;
  int colonne;
};

// ------------------------------------------------
// LEXER
// ------------------------------------------------
class Lexer {
public:
  Lexer(const std::string &source)
      : source(source), pos(0), ligne(1), colonne(1) {}

  std::vector<Token> tokenize() {
    std::vector<Token> tokens;
    while (!fini()) {
      Token tok = prochainToken();
      if (tok.type != TokenType::INCONNU) {
        tokens.push_back(tok);
      } else
        throw std::runtime_error("Caractère inconnu '" + tok.valeur +
                                 "' ligne " + std::to_string(tok.ligne) +
                                 ", colonne " + std::to_string(tok.colonne));
    }
    tokens.push_back(makeToken(TokenType::FIN, ""));
    return tokens;
  }

private:
  std::string source;
  size_t pos;
  int ligne;
  int colonne;

  // --- Mots-clés ---
  static const std::unordered_map<std::string, TokenType> &motsCles() {
    static const std::unordered_map<std::string, TokenType> table = {
        {"let", TokenType::LET},       {"fn", TokenType::FN},
        {"return", TokenType::RETURN}, {"while", TokenType::WHILE},
        {"for", TokenType::FOR},       {"in", TokenType::IN},
        {"break", TokenType::BREAK},   {"continue", TokenType::CONTINUE},
        {"if", TokenType::IF},         {"else", TokenType::ELSE},
        {"print", TokenType::PRINT},
    };
    return table;
  }

  // --- Utilitaires ---
  bool fini() const { return pos >= source.size(); }
  char courant() const { return source[pos]; }
  char suivant() const {
    return pos + 1 < source.size() ? source[pos + 1] : '\0';
  }

  char avancer() {
    char c = source[pos++];
    if (c == '\n') {
      ligne++;
      colonne = 1;
    } else {
      colonne++;
    }
    return c;
  }

  void ignorerEspaces() {
    while (!fini() && std::isspace(courant()))
      avancer();
  }

  void ignorerCommentaire() {
    while (!fini() && courant() != '\n')
      avancer();
  }

  Token makeToken(TokenType type, const std::string &val) {
    return {type, val, ligne, colonne};
  }

  // --- Lecture des littéraux ---

  Token lireNombre() {
    std::string val;
    bool decimal = false;
    int l = ligne, c = colonne;

    while (!fini() && (std::isdigit(courant()) || courant() == '.')) {
      if (courant() == '.') {
        if (decimal)
          break; // deuxième point → on arrête
        decimal = true;
      }
      val += avancer();
    }
    return {decimal ? TokenType::NOMBRE_DECIMAL : TokenType::NOMBRE_ENTIER, val,
            l, c};
  }

  Token lireChaine() {
    std::string val;
    int l = ligne, c = colonne;
    avancer(); // consomme le "
    while (!fini() && courant() != '"') {
      if (courant() == '\\' && suivant() == '"') {
        avancer(); // consomme le backslash
      }
      val += avancer();
    }
    if (fini())
      throw std::runtime_error("Chaîne non fermée ligne " + std::to_string(l));
    avancer(); // consomme le " fermant
    return {TokenType::CHAINE, val, l, c};
  }

  Token lireIdentifiantOuMotCle() {
    std::string val;
    int l = ligne, c = colonne;
    while (!fini() && (std::isalnum(courant()) || courant() == '_'))
      val += avancer();

    auto it = motsCles().find(val);
    TokenType type =
        (it != motsCles().end()) ? it->second : TokenType::IDENTIFIANT;
    // Littéraux booléens : true → 1, false → 0
    if (val == "true")
      return {TokenType::NOMBRE_ENTIER, "1", l, c};
    if (val == "false")
      return {TokenType::NOMBRE_ENTIER, "0", l, c};
    return {type, val, l, c};
  }

  // --- Token principal ---

  Token prochainToken() {
    ignorerEspaces();
    if (fini())
      return makeToken(TokenType::FIN, "");

    // Commentaire de ligne : # uniquement
    if (courant() == '#') {
      ignorerCommentaire();
      return prochainToken();
    }

    int l = ligne, c = colonne;

    // Division entière // : toujours un opérateur (les commentaires n'utilisent
    // que #)
    if (courant() == '/' && suivant() == '/') {
      avancer();
      avancer();
      return {TokenType::SLASH_SLASH, "//", l, c};
    }

    // Littéraux
    if (std::isdigit(courant()))
      return lireNombre();
    if (courant() == '"')
      return lireChaine();
    if (std::isalpha(courant()) || courant() == '_')
      return lireIdentifiantOuMotCle();

    // Opérateurs à 2 caractères (logiques)
    if (courant() == '&' && suivant() == '&') {
      avancer();
      avancer();
      return {TokenType::ET_LOGIQUE, "&&", l, c};
    }
    if (courant() == '|' && suivant() == '|') {
      avancer();
      avancer();
      return {TokenType::OU_LOGIQUE, "||", l, c};
    }

    // Opérateurs d'affectation composée et ++/--
    if (courant() == '+' && suivant() == '+') {
      avancer();
      avancer();
      return {TokenType::PLUS_PLUS, "++", l, c};
    }
    if (courant() == '+' && suivant() == '=') {
      avancer();
      avancer();
      return {TokenType::PLUS_EGAL, "+=", l, c};
    }
    if (courant() == '-' && suivant() == '-') {
      avancer();
      avancer();
      return {TokenType::MOINS_MOINS, "--", l, c};
    }
    if (courant() == '-' && suivant() == '=') {
      avancer();
      avancer();
      return {TokenType::MOINS_EGAL, "-=", l, c};
    }
    if (courant() == '*' && suivant() == '=') {
      avancer();
      avancer();
      return {TokenType::ETOILE_EGAL, "*=", l, c};
    }
    if (courant() == '/' && suivant() == '=') {
      avancer();
      avancer();
      return {TokenType::SLASH_EGAL, "/=", l, c};
    }

    // Opérateurs et ponctuation
    char ch = avancer();
    switch (ch) {
    case '+':
      return {TokenType::PLUS, "+", l, c};
    case '-':
      return {TokenType::MOINS, "-", l, c};
    case '*':
      return {TokenType::ETOILE, "*", l, c};
    case '/':
      return {TokenType::SLASH, "/", l, c};
    case '%':
      return {TokenType::MODULO, "%", l, c};
    case '^':
      return {TokenType::PUISSANCE, "^", l, c};
    case '(':
      return {TokenType::PARENTHESE_OUV, "(", l, c};
    case ')':
      return {TokenType::PARENTHESE_FERM, ")", l, c};
    case '{':
      return {TokenType::ACCOLADE_OUV, "{", l, c};
    case '}':
      return {TokenType::ACCOLADE_FERM, "}", l, c};
    case '[':
      return {TokenType::CROCHET_OUV, "[", l, c};
    case ']':
      return {TokenType::CROCHET_FERM, "]", l, c};
    case ',':
      return {TokenType::VIRGULE, ",", l, c};
    case ';':
      return {TokenType::POINT_VIRGULE, ";", l, c};
    case '.':
      return {TokenType::POINT, ".", l, c};

    // Opérateurs à 1 ou 2 caractères
    case '=':
      return courant() == '=' ? (avancer(), Token{TokenType::EGAL, "==", l, c})
                              : Token{TokenType::AFFECTATION, "=", l, c};
    case '!':
      return courant() == '='
                 ? (avancer(), Token{TokenType::DIFFERENT, "!=", l, c})
                 : Token{TokenType::NON_LOGIQUE, "!", l, c};
    case '<':
      return courant() == '='
                 ? (avancer(), Token{TokenType::INF_EGAL, "<=", l, c})
                 : Token{TokenType::INF, "<", l, c};
    case '>':
      return courant() == '='
                 ? (avancer(), Token{TokenType::SUP_EGAL, ">=", l, c})
                 : Token{TokenType::SUP, ">", l, c};

    default:
      return {TokenType::INCONNU, std::string(1, ch), l, c};
    }
  }
};
