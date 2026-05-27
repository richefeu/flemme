// parser.hpp
#pragma once
#include "lexer.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// ------------------------------------------------
// ENUM DE NŒUDS — évite dynamic_cast dans l'interpréteur
// ------------------------------------------------

enum class NodeKind {
    NumberLiteral, StringLiteral, ArrayLiteral, IndexExpr,
    VarExpr, MemberExpr, BinOpExpr, CallExpr, AssignExpr,
    VarDecl, Block, WhileStmt, ForStmt, ForInStmt, IfStmt,
    PrintStmt, FnDecl, ReturnStmt, BreakStmt, ContinueStmt, ExprStmt
};

// ------------------------------------------------
// ENUM D'OPÉRATEURS — évite les comparaisons de chaînes
// ------------------------------------------------

enum class OpKind {
    Add, Sub, Mul, Div, Mod, IntDiv, Pow,
    Eq, Ne, Lt, Gt, Le, Ge,
    And, Or, Not
};

inline OpKind strToOpKind(const std::string &op) {
    if (op == "+")  return OpKind::Add;
    if (op == "-")  return OpKind::Sub;
    if (op == "*")  return OpKind::Mul;
    if (op == "/")  return OpKind::Div;
    if (op == "%")  return OpKind::Mod;
    if (op == "//") return OpKind::IntDiv;
    if (op == "^")  return OpKind::Pow;
    if (op == "==") return OpKind::Eq;
    if (op == "!=") return OpKind::Ne;
    if (op == "<")  return OpKind::Lt;
    if (op == ">")  return OpKind::Gt;
    if (op == "<=") return OpKind::Le;
    if (op == ">=") return OpKind::Ge;
    if (op == "&&") return OpKind::And;
    if (op == "||") return OpKind::Or;
    if (op == "!")  return OpKind::Not;
    throw std::runtime_error("Opérateur inconnu : " + op);
}

// ------------------------------------------------
// NOEUDS DE L'AST
// ------------------------------------------------

struct ASTNode {
    virtual ~ASTNode() = default;
    int line = 0;
    NodeKind kind;
protected:
    explicit ASTNode(NodeKind k) : kind(k) {}
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

// --- Expressions ---

struct NumberLiteral : ASTNode {
    double value;
    explicit NumberLiteral(double v) : ASTNode(NodeKind::NumberLiteral), value(v) {}
};

struct StringLiteral : ASTNode {
    std::string value;
    explicit StringLiteral(const std::string &v) : ASTNode(NodeKind::StringLiteral), value(v) {}
};

struct ArrayLiteral : ASTNode {
    std::vector<ASTNodePtr> elements;
    explicit ArrayLiteral(std::vector<ASTNodePtr> elems)
        : ASTNode(NodeKind::ArrayLiteral), elements(std::move(elems)) {}
};

struct IndexExpr : ASTNode {
    ASTNodePtr array;
    ASTNodePtr index;
    IndexExpr(ASTNodePtr arr, ASTNodePtr idx)
        : ASTNode(NodeKind::IndexExpr), array(std::move(arr)), index(std::move(idx)) {}
};

struct VarExpr : ASTNode {
    std::string name;
    explicit VarExpr(const std::string &n) : ASTNode(NodeKind::VarExpr), name(n) {}
};

struct MemberExpr : ASTNode {
    std::string object;
    std::string member;
    MemberExpr(const std::string &o, const std::string &m)
        : ASTNode(NodeKind::MemberExpr), object(o), member(m) {}
};

struct BinOpExpr : ASTNode {
    OpKind op;
    ASTNodePtr left;
    ASTNodePtr right;
    BinOpExpr(OpKind op, ASTNodePtr l, ASTNodePtr r)
        : ASTNode(NodeKind::BinOpExpr), op(op), left(std::move(l)), right(std::move(r)) {}
};

struct CallExpr : ASTNode {
    std::string object;
    std::string method;
    std::vector<ASTNodePtr> args;
    CallExpr(const std::string &obj, const std::string &meth, std::vector<ASTNodePtr> a)
        : ASTNode(NodeKind::CallExpr), object(obj), method(meth), args(std::move(a)) {}
};

struct AssignExpr : ASTNode {
    std::string object;
    std::string name;
    ASTNodePtr indexExpr;
    ASTNodePtr value;

    AssignExpr(const std::string &obj, const std::string &n, ASTNodePtr v)
        : ASTNode(NodeKind::AssignExpr), object(obj), name(n), indexExpr(nullptr), value(std::move(v)) {}

    AssignExpr(ASTNodePtr idx, ASTNodePtr v)
        : ASTNode(NodeKind::AssignExpr), object(""), name(""), indexExpr(std::move(idx)), value(std::move(v)) {}
};

// --- Instructions ---

struct VarDecl : ASTNode {
    std::string name;
    ASTNodePtr initializer;
    VarDecl(const std::string &n, ASTNodePtr init)
        : ASTNode(NodeKind::VarDecl), name(n), initializer(std::move(init)) {}
};

struct Block : ASTNode {
    Block() : ASTNode(NodeKind::Block) {}
    std::vector<ASTNodePtr> statements;
};

struct WhileStmt : ASTNode {
    ASTNodePtr condition;
    ASTNodePtr body;
    WhileStmt(ASTNodePtr cond, ASTNodePtr b)
        : ASTNode(NodeKind::WhileStmt), condition(std::move(cond)), body(std::move(b)) {}
};

struct ForStmt : ASTNode {
    ASTNodePtr initializer;
    ASTNodePtr condition;
    ASTNodePtr increment;
    ASTNodePtr body;
    ForStmt(ASTNodePtr init, ASTNodePtr cond, ASTNodePtr incr, ASTNodePtr b)
        : ASTNode(NodeKind::ForStmt), initializer(std::move(init)), condition(std::move(cond)),
          increment(std::move(incr)), body(std::move(b)) {}
};

struct ForInStmt : ASTNode {
    std::string varName;
    ASTNodePtr iterable;
    ASTNodePtr body;
    ForInStmt(const std::string &var, ASTNodePtr iter, ASTNodePtr b)
        : ASTNode(NodeKind::ForInStmt), varName(var), iterable(std::move(iter)), body(std::move(b)) {}
};

struct IfStmt : ASTNode {
    ASTNodePtr condition;
    ASTNodePtr thenBranch;
    ASTNodePtr elseBranch;
    IfStmt(ASTNodePtr cond, ASTNodePtr thenB, ASTNodePtr elseB = nullptr)
        : ASTNode(NodeKind::IfStmt), condition(std::move(cond)), thenBranch(std::move(thenB)),
          elseBranch(std::move(elseB)) {}
};

struct PrintStmt : ASTNode {
    std::string text;
    explicit PrintStmt(const std::string &t) : ASTNode(NodeKind::PrintStmt), text(t) {}
};

struct FnDecl : ASTNode {
    std::string name;
    std::vector<std::string> params;
    ASTNodePtr body;
    FnDecl(const std::string &n, std::vector<std::string> p, ASTNodePtr b)
        : ASTNode(NodeKind::FnDecl), name(n), params(std::move(p)), body(std::move(b)) {}
};

struct ReturnStmt : ASTNode {
    ASTNodePtr value;
    explicit ReturnStmt(ASTNodePtr v) : ASTNode(NodeKind::ReturnStmt), value(std::move(v)) {}
};

struct BreakStmt : ASTNode {
    BreakStmt() : ASTNode(NodeKind::BreakStmt) {}
};

struct ContinueStmt : ASTNode {
    ContinueStmt() : ASTNode(NodeKind::ContinueStmt) {}
};

struct ExprStmt : ASTNode {
    ASTNodePtr expr;
    explicit ExprStmt(ASTNodePtr e) : ASTNode(NodeKind::ExprStmt), expr(std::move(e)) {}
};

// ------------------------------------------------
// PARSER
// ------------------------------------------------

class Parser {
public:
    explicit Parser(std::vector<Token> tokens)
        : tokens(std::move(tokens)), pos(0) {}

    std::unique_ptr<Block> parse() {
        auto program = std::make_unique<Block>();
        while (!atEnd())
            program->statements.push_back(parseStatement());
        return program;
    }

    ASTNodePtr parseExpression() {
        auto left = parseLogicalAnd();
        while (check(TokenType::OU_LOGIQUE)) {
            std::string op = advance().valeur;
            auto right = parseLogicalAnd();
            left = std::make_unique<BinOpExpr>(strToOpKind(op), std::move(left), std::move(right));
        }
        if (check(TokenType::PLUS_EGAL) || check(TokenType::MOINS_EGAL) ||
            check(TokenType::ETOILE_EGAL) || check(TokenType::SLASH_EGAL)) {
            std::string tok = advance().valeur;
            OpKind opKind = strToOpKind(std::string(1, tok[0]));
            auto rhs = parseExpression();
            if (auto *v = dynamic_cast<VarExpr *>(left.get()))
                return std::make_unique<AssignExpr>("", v->name,
                    std::make_unique<BinOpExpr>(opKind, std::make_unique<VarExpr>(v->name), std::move(rhs)));
            if (auto *m = dynamic_cast<MemberExpr *>(left.get()))
                return std::make_unique<AssignExpr>(m->object, m->member,
                    std::make_unique<BinOpExpr>(opKind, std::make_unique<MemberExpr>(m->object, m->member), std::move(rhs)));
            throw std::runtime_error("Cible d'affectation composée invalide");
        }
        if (check(TokenType::AFFECTATION)) {
            advance();
            auto val = parseExpression();
            if (auto *m = dynamic_cast<MemberExpr *>(left.get()))
                return std::make_unique<AssignExpr>(m->object, m->member, std::move(val));
            if (auto *v = dynamic_cast<VarExpr *>(left.get()))
                return std::make_unique<AssignExpr>("", v->name, std::move(val));
            if (dynamic_cast<IndexExpr *>(left.get()))
                return std::make_unique<AssignExpr>(std::move(left), std::move(val));
            throw std::runtime_error("Cible d'affectation invalide");
        }
        return left;
    }

private:
    std::vector<Token> tokens;
    size_t pos;

    Token peek() const { return tokens[pos]; }
    Token advance() { return tokens[pos++]; }
    bool atEnd() const { return tokens[pos].type == TokenType::FIN; }
    bool check(TokenType t) const { return tokens[pos].type == t; }
    bool checkVal(const std::string &v) const { return tokens[pos].valeur == v; }

    Token expect(TokenType t, const std::string &msg) {
        if (!check(t))
            throw std::runtime_error(msg + " (ligne " + std::to_string(peek().ligne) +
                                     ", reçu '" + peek().valeur + "')");
        return advance();
    }

    bool match(TokenType t) {
        if (check(t)) { advance(); return true; }
        return false;
    }

    bool matchVal(const std::string &v) {
        if (checkVal(v)) { advance(); return true; }
        return false;
    }

    ASTNodePtr parseStatement() {
        int line = peek().ligne;
        ASTNodePtr node;

        if      (check(TokenType::LET))          node = parseVarDecl();
        else if (check(TokenType::FN))           node = parseFnDecl();
        else if (check(TokenType::IF))           node = parseIf();
        else if (check(TokenType::PRINT))        node = parsePrint();
        else if (check(TokenType::WHILE))        node = parseWhile();
        else if (check(TokenType::FOR))          node = parseForStatement();
        else if (check(TokenType::BREAK))        node = parseBreak();
        else if (check(TokenType::CONTINUE))     node = parseContinue();
        else if (check(TokenType::RETURN))       node = parseReturn();
        else if (check(TokenType::ACCOLADE_OUV)) node = parseBlock();
        else                                     node = parseExprStmt();

        node->line = line;
        return node;
    }

    ASTNodePtr parseVarDecl() {
        advance();
        std::string name = expect(TokenType::IDENTIFIANT, "Nom de variable attendu").valeur;
        expect(TokenType::AFFECTATION, "'=' attendu après le nom de variable");
        auto init = parseExpression();
        expect(TokenType::POINT_VIRGULE, "';' attendu après la déclaration");
        return std::make_unique<VarDecl>(name, std::move(init));
    }

    ASTNodePtr parseFnDecl() {
        advance();
        std::string name = expect(TokenType::IDENTIFIANT, "Nom de fonction attendu").valeur;
        expect(TokenType::PARENTHESE_OUV, "'(' attendu");
        std::vector<std::string> params;
        while (!check(TokenType::PARENTHESE_FERM) && !atEnd()) {
            params.push_back(expect(TokenType::IDENTIFIANT, "Paramètre attendu").valeur);
            if (!check(TokenType::PARENTHESE_FERM))
                expect(TokenType::VIRGULE, "',' attendu entre les paramètres");
        }
        expect(TokenType::PARENTHESE_FERM, "')' attendu");
        auto body = parseBlock();
        return std::make_unique<FnDecl>(name, std::move(params), std::move(body));
    }

    ASTNodePtr parseWhile() {
        advance();
        expect(TokenType::PARENTHESE_OUV, "'(' attendu après while");
        auto cond = parseExpression();
        expect(TokenType::PARENTHESE_FERM, "')' attendu après la condition");
        auto body = parseBlock();
        return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
    }

    ASTNodePtr parseForStatement() {
        advance();
        expect(TokenType::PARENTHESE_OUV, "'(' attendu après for");
        if (check(TokenType::LET) && pos + 2 < tokens.size() &&
            tokens[pos + 1].type == TokenType::IDENTIFIANT &&
            tokens[pos + 2].type == TokenType::IN) {
            return parseForIn();
        }
        return parseForTraditional();
    }

    ASTNodePtr parseForIn() {
        advance();
        std::string varName = expect(TokenType::IDENTIFIANT, "Nom de variable attendu").valeur;
        expect(TokenType::IN, "'in' attendu après le nom de variable");
        auto iterable = parseExpression();
        expect(TokenType::PARENTHESE_FERM, "')' attendu après l'itérable");
        auto body = parseBlock();
        return std::make_unique<ForInStmt>(varName, std::move(iterable), std::move(body));
    }

    ASTNodePtr parseForTraditional() {
        ASTNodePtr init = nullptr;
        if (!check(TokenType::POINT_VIRGULE)) {
            if (check(TokenType::LET)) {
                advance();
                std::string name = expect(TokenType::IDENTIFIANT, "Nom de variable attendu").valeur;
                expect(TokenType::AFFECTATION, "'=' attendu après le nom de variable");
                auto initExpr = parseExpression();
                init = std::make_unique<VarDecl>(name, std::move(initExpr));
            } else {
                init = parseExpression();
            }
        }
        expect(TokenType::POINT_VIRGULE, "';' attendu après l'initialisation");

        ASTNodePtr cond = nullptr;
        if (!check(TokenType::POINT_VIRGULE))
            cond = parseExpression();
        expect(TokenType::POINT_VIRGULE, "';' attendu après la condition");

        ASTNodePtr incr = nullptr;
        if (!check(TokenType::PARENTHESE_FERM))
            incr = parseExpression();
        expect(TokenType::PARENTHESE_FERM, "')' attendu après l'incrément");

        auto body = parseBlock();
        if (!cond) cond = std::make_unique<NumberLiteral>(1.0);
        return std::make_unique<ForStmt>(std::move(init), std::move(cond), std::move(incr), std::move(body));
    }

    ASTNodePtr parseIf() {
        advance();
        expect(TokenType::PARENTHESE_OUV, "'(' attendu après if");
        auto cond = parseExpression();
        expect(TokenType::PARENTHESE_FERM, "')' attendu après la condition");
        auto thenBranch = parseBlock();
        ASTNodePtr elseBranch = nullptr;
        if (match(TokenType::ELSE)) {
            if (check(TokenType::IF))
                elseBranch = parseIf();
            else
                elseBranch = parseBlock();
        }
        return std::make_unique<IfStmt>(std::move(cond), std::move(thenBranch), std::move(elseBranch));
    }

    ASTNodePtr parsePrint() {
        advance();
        Token strTok = expect(TokenType::CHAINE, "Chaîne attendue après print");
        expect(TokenType::POINT_VIRGULE, "';' attendu après print");
        return std::make_unique<PrintStmt>(strTok.valeur);
    }

    ASTNodePtr parseBreak() {
        advance();
        expect(TokenType::POINT_VIRGULE, "';' attendu après break");
        return std::make_unique<BreakStmt>();
    }

    ASTNodePtr parseContinue() {
        advance();
        expect(TokenType::POINT_VIRGULE, "';' attendu après continue");
        return std::make_unique<ContinueStmt>();
    }

    ASTNodePtr parseReturn() {
        advance();
        ASTNodePtr val = nullptr;
        if (!check(TokenType::POINT_VIRGULE))
            val = parseExpression();
        expect(TokenType::POINT_VIRGULE, "';' attendu après return");
        return std::make_unique<ReturnStmt>(std::move(val));
    }

    ASTNodePtr parseBlock() {
        expect(TokenType::ACCOLADE_OUV, "'{' attendu");
        auto block = std::make_unique<Block>();
        while (!check(TokenType::ACCOLADE_FERM) && !atEnd())
            block->statements.push_back(parseStatement());
        expect(TokenType::ACCOLADE_FERM, "'}' attendu");
        return block;
    }

    ASTNodePtr parseExprStmt() {
        auto expr = parseExpression();
        expect(TokenType::POINT_VIRGULE, "';' attendu après l'expression");
        return std::make_unique<ExprStmt>(std::move(expr));
    }

    ASTNodePtr parseLogicalAnd() {
        auto left = parseComparison();
        while (check(TokenType::ET_LOGIQUE)) {
            std::string op = advance().valeur;
            auto right = parseComparison();
            left = std::make_unique<BinOpExpr>(strToOpKind(op), std::move(left), std::move(right));
        }
        return left;
    }

    ASTNodePtr parseComparison() {
        auto left = parseAddSub();
        std::string op = peek().valeur;
        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
            advance();
            auto right = parseAddSub();
            return std::make_unique<BinOpExpr>(strToOpKind(op), std::move(left), std::move(right));
        }
        return left;
    }

    ASTNodePtr parseAddSub() {
        auto left = parseMulDiv();
        while (check(TokenType::PLUS) || check(TokenType::MOINS)) {
            std::string op = advance().valeur;
            auto right = parseMulDiv();
            left = std::make_unique<BinOpExpr>(strToOpKind(op), std::move(left), std::move(right));
        }
        return left;
    }

    ASTNodePtr parseMulDiv() {
        auto left = parsePower();
        while (check(TokenType::ETOILE) || check(TokenType::SLASH) ||
               check(TokenType::MODULO) || check(TokenType::SLASH_SLASH)) {
            std::string op = advance().valeur;
            auto right = parsePower();
            left = std::make_unique<BinOpExpr>(strToOpKind(op), std::move(left), std::move(right));
        }
        return left;
    }

    ASTNodePtr parsePower() {
        auto left = parsePrimary();
        if (check(TokenType::PUISSANCE)) {
            std::string op = advance().valeur;
            auto right = parsePower();
            return std::make_unique<BinOpExpr>(strToOpKind(op), std::move(left), std::move(right));
        }
        return left;
    }

    ASTNodePtr parsePrimary() {
        if (check(TokenType::NON_LOGIQUE)) {
            advance();
            auto right = parsePrimary();
            return std::make_unique<BinOpExpr>(OpKind::Not,
                std::make_unique<NumberLiteral>(0.0), std::move(right));
        }

        if (check(TokenType::MOINS)) {
            advance();
            auto right = parsePrimary();
            return std::make_unique<BinOpExpr>(OpKind::Sub,
                std::make_unique<NumberLiteral>(0.0), std::move(right));
        }

        if (check(TokenType::CROCHET_OUV))
            return parseArrayLiteral();

        if (check(TokenType::NOMBRE_ENTIER) || check(TokenType::NOMBRE_DECIMAL)) {
            double v = std::stod(advance().valeur);
            return std::make_unique<NumberLiteral>(v);
        }

        if (check(TokenType::CHAINE)) {
            std::string s = advance().valeur;
            return std::make_unique<StringLiteral>(s);
        }

        if (check(TokenType::IDENTIFIANT)) {
            std::string name = advance().valeur;
            ASTNodePtr node = std::make_unique<VarExpr>(name);

            if (match(TokenType::CROCHET_OUV)) {
                auto index = parseExpression();
                expect(TokenType::CROCHET_FERM, "]' attendu");
                node = std::make_unique<IndexExpr>(std::move(node), std::move(index));
                while (check(TokenType::CROCHET_OUV)) {
                    advance();
                    auto idx2 = parseExpression();
                    expect(TokenType::CROCHET_FERM, "]' attendu");
                    node = std::make_unique<IndexExpr>(std::move(node), std::move(idx2));
                }
                return node;
            }

            if (matchVal(".")) {
                std::string member = expect(TokenType::IDENTIFIANT, "Membre attendu après '.'").valeur;
                if (match(TokenType::CROCHET_OUV)) {
                    auto idxNode = std::make_unique<MemberExpr>(name, member);
                    auto index = parseExpression();
                    expect(TokenType::CROCHET_FERM, "]' attendu");
                    return std::make_unique<IndexExpr>(std::move(idxNode), std::move(index));
                }
                if (check(TokenType::PARENTHESE_OUV))
                    return parseCallArgs(name, member);
                return std::make_unique<MemberExpr>(name, member);
            }

            if (check(TokenType::PARENTHESE_OUV))
                return parseCallArgs("", name);

            if (check(TokenType::PLUS_PLUS)) {
                advance();
                return std::make_unique<AssignExpr>("", name,
                    std::make_unique<BinOpExpr>(OpKind::Add,
                        std::make_unique<VarExpr>(name), std::make_unique<NumberLiteral>(1.0)));
            }
            if (check(TokenType::MOINS_MOINS)) {
                advance();
                return std::make_unique<AssignExpr>("", name,
                    std::make_unique<BinOpExpr>(OpKind::Sub,
                        std::make_unique<VarExpr>(name), std::make_unique<NumberLiteral>(1.0)));
            }

            return node;
        }

        if (check(TokenType::PARENTHESE_OUV)) {
            advance();
            auto expr = parseExpression();
            expect(TokenType::PARENTHESE_FERM, "')' attendu");
            return expr;
        }

        throw std::runtime_error("Expression attendue ligne " +
                                 std::to_string(peek().ligne) + ", reçu '" +
                                 peek().valeur + "'");
    }

    ASTNodePtr parseArrayLiteral() {
        advance();
        std::vector<ASTNodePtr> elements;
        while (!check(TokenType::CROCHET_FERM) && !atEnd()) {
            elements.push_back(parseExpression());
            if (!check(TokenType::CROCHET_FERM))
                expect(TokenType::VIRGULE, "',' attendu entre les éléments");
        }
        expect(TokenType::CROCHET_FERM, "]' attendu");
        return std::make_unique<ArrayLiteral>(std::move(elements));
    }

    ASTNodePtr parseCallArgs(const std::string &obj, const std::string &method) {
        advance();
        std::vector<ASTNodePtr> args;
        while (!check(TokenType::PARENTHESE_FERM) && !atEnd()) {
            args.push_back(parseExpression());
            if (!check(TokenType::PARENTHESE_FERM))
                expect(TokenType::VIRGULE, "',' attendu entre les arguments");
        }
        expect(TokenType::PARENTHESE_FERM, "')' attendu");
        return std::make_unique<CallExpr>(obj, method, std::move(args));
    }
};
