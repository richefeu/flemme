// interpreter.hpp
#pragma once
#include "parser.hpp"
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <atomic>
#include <random>
#include <variant>
#include <vector>

// ------------------------------------------------
// VALUE — type dynamique du langage
// ------------------------------------------------

struct NativeObject;
struct ArrayObject;

using Value = std::variant<std::monostate,
                           double,
                           std::string,
                           std::shared_ptr<NativeObject>,
                           std::shared_ptr<ArrayObject>
                           >;

inline double asDouble(const Value &v) { return std::get<double>(v); }
inline std::string asString(const Value &v) { return std::get<std::string>(v); }

inline std::string valueToString(const Value &v);

// ------------------------------------------------
// NATIVE OBJECT — conteneur générique pour tout objet C++ bindé
// ------------------------------------------------

struct NativeObject {
    std::string typeName;
    std::unordered_map<std::string, std::function<Value(std::vector<Value>)>> methods;
    std::unordered_map<std::string, std::function<Value()>> getters;
    std::unordered_map<std::string, std::function<void(Value)>> setters;

    explicit NativeObject(std::string type = "native") : typeName(std::move(type)) {}
};

struct ArrayObject {
    std::vector<Value> elements;

    explicit ArrayObject(const std::vector<Value> &elems) : elements(elems) {}
    explicit ArrayObject(std::vector<Value> &&elems) : elements(std::move(elems)) {}

    Value get(size_t index) const {
        if (index >= elements.size())
            throw std::runtime_error("Index hors limites : " + std::to_string(index));
        return elements[index];
    }

    void set(size_t index, Value val) {
        if (index >= elements.size())
            throw std::runtime_error("Index hors limites : " + std::to_string(index));
        elements[index] = std::move(val);
    }

    size_t length() const { return elements.size(); }
};

inline ArrayObject *getArray(const Value &v) {
    if (std::holds_alternative<std::shared_ptr<ArrayObject>>(v))
        return std::get<std::shared_ptr<ArrayObject>>(v).get();
    return nullptr;
}

inline std::string valueToString(const Value &v) {
    if (std::holds_alternative<std::monostate>(v)) return "void";
    if (std::holds_alternative<double>(v)) {
        double d = std::get<double>(v);
        if (d == std::floor(d) && !std::isinf(d) && std::abs(d) < 1e15)
            return std::to_string(static_cast<long long>(d));
        std::ostringstream oss;
        oss << d;
        return oss.str();
    }
    if (std::holds_alternative<std::string>(v)) return std::get<std::string>(v);
    if (std::holds_alternative<std::shared_ptr<NativeObject>>(v))
        return "<" + std::get<std::shared_ptr<NativeObject>>(v)->typeName + ">";
    if (std::holds_alternative<std::shared_ptr<ArrayObject>>(v)) {
        auto &arr = *std::get<std::shared_ptr<ArrayObject>>(v);
        std::string s = "[";
        for (size_t i = 0; i < arr.elements.size(); ++i) {
            if (i > 0) s += ", ";
            s += valueToString(arr.elements[i]);
        }
        return s + "]";
    }
    return "?";
}

// ------------------------------------------------
// EXCEPTIONS DE CONTRÔLE
// ------------------------------------------------

struct ReturnException  { Value value; };
struct BreakException   {};
struct ContinueException {};

// ------------------------------------------------
// ENVIRONNEMENT
// ------------------------------------------------

class Environment {
public:
    explicit Environment(std::shared_ptr<Environment> parent = nullptr)
        : parent(std::move(parent)) {}

    void set(const std::string &name, Value val) { vars[name] = std::move(val); }

    void assign(const std::string &name, Value val) {
        auto it = vars.find(name);
        if (it != vars.end()) { it->second = std::move(val); return; }
        if (parent) { parent->assign(name, std::move(val)); return; }
        throw std::runtime_error("Variable non déclarée : '" + name + "'");
    }

    Value get(const std::string &name) const {
        auto it = vars.find(name);
        if (it != vars.end()) return it->second;
        if (parent) return parent->get(name);
        throw std::runtime_error("Variable inconnue : '" + name + "'");
    }

    bool has(const std::string &name) const {
        if (vars.count(name)) return true;
        if (parent) return parent->has(name);
        return false;
    }

private:
    std::unordered_map<std::string, Value> vars;
    std::shared_ptr<Environment> parent;
};

// ------------------------------------------------
// FONCTION UTILISATEUR
// ------------------------------------------------

struct UserFunction {
    std::vector<std::string> params;
    Block *body;
    std::shared_ptr<Environment> closure;
};

// ------------------------------------------------
// INTERPRETER
// ------------------------------------------------

class Interpreter {
public:
    explicit Interpreter() : globalEnv(std::make_shared<Environment>()) {
        registerMathFunctions();
    }

    void registerFunction(const std::string &name,
                          std::function<Value(std::vector<Value>)> fn) {
        nativeFunctions[name] = std::move(fn);
    }

    void registerConstructor(const std::string &name,
                             std::function<Value()> ctor) {
        constructors[name] = std::move(ctor);
    }

    void run(Block *program) { execBlock(program, globalEnv); }

    std::atomic<bool> shouldStop{false};

private:
    std::shared_ptr<Environment> globalEnv;
    std::unordered_map<std::string, std::function<Value(std::vector<Value>)>> nativeFunctions;
    std::unordered_map<std::string, std::function<Value()>> constructors;
    std::unordered_map<std::string, UserFunction> userFunctions;

    void checkInterrupt() {
        if (shouldStop.load(std::memory_order_relaxed))
            throw std::runtime_error("Exécution interrompue : délai dépassé (> 5 s)");
    }

    // ------------------------------------------------
    // Fonctions natives
    // ------------------------------------------------

    void registerMathFunctions() {
        auto math1 = [&](const std::string &name, double(*fn)(double)) {
            nativeFunctions[name] = [fn, name](std::vector<Value> args) -> Value {
                if (args.size() != 1 || !std::holds_alternative<double>(args[0]))
                    throw std::runtime_error(name + " : attend un nombre");
                return Value{fn(std::get<double>(args[0]))};
            };
        };
        math1("sin",   std::sin);
        math1("cos",   std::cos);
        math1("tan",   std::tan);
        math1("sqrt",  std::sqrt);
        math1("exp",   std::exp);
        math1("floor", std::floor);
        math1("ceil",  std::ceil);

        nativeFunctions["ln"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<double>(args[0]))
                throw std::runtime_error("ln : attend un nombre");
            double x = std::get<double>(args[0]);
            if (x <= 0.0) throw std::runtime_error("ln : argument doit être > 0");
            return Value{std::log(x)};
        };
        nativeFunctions["log10"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<double>(args[0]))
                throw std::runtime_error("log10 : attend un nombre");
            double x = std::get<double>(args[0]);
            if (x <= 0.0) throw std::runtime_error("log10 : argument doit être > 0");
            return Value{std::log10(x)};
        };
        nativeFunctions["round"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<double>(args[0]))
                throw std::runtime_error("round : attend un nombre");
            return Value{std::round(std::get<double>(args[0]))};
        };
        nativeFunctions["abs"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<double>(args[0]))
                throw std::runtime_error("abs : attend un nombre");
            return Value{std::abs(std::get<double>(args[0]))};
        };

        // rand() → double dans [0, 1)  ;  srand(graine) pour reproductibilité
        {
            auto rng  = std::make_shared<std::mt19937>(std::random_device{}());
            auto dist = std::make_shared<std::uniform_real_distribution<double>>(0.0, 1.0);
            nativeFunctions["rand"] = [rng, dist](std::vector<Value> args) -> Value {
                if (!args.empty())
                    throw std::runtime_error("rand : attend 0 argument");
                return Value{(*dist)(*rng)};
            };
            nativeFunctions["srand"] = [rng](std::vector<Value> args) -> Value {
                if (args.size() != 1 || !std::holds_alternative<double>(args[0]))
                    throw std::runtime_error("srand : attend un entier (graine)");
                rng->seed(static_cast<uint32_t>(std::get<double>(args[0])));
                return {};
            };
        }

        nativeFunctions["len"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1) throw std::runtime_error("len : attend un argument");
            if (auto *arr = getArray(args[0]))
                return Value{static_cast<double>(arr->length())};
            if (std::holds_alternative<std::string>(args[0]))
                return Value{static_cast<double>(std::get<std::string>(args[0]).size())};
            throw std::runtime_error("len : attend un tableau ou une chaîne");
        };

        nativeFunctions["range"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<double>(args[0]))
                throw std::runtime_error("range : attend un entier");
            int n = static_cast<int>(std::get<double>(args[0]));
            if (n < 0) throw std::runtime_error("range : argument doit être >= 0");
            std::vector<Value> elems;
            elems.reserve(static_cast<size_t>(n));
            for (int i = 0; i < n; ++i)
                elems.push_back(Value{static_cast<double>(i)});
            return Value{std::make_shared<ArrayObject>(std::move(elems))};
        };

        nativeFunctions["push"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 2)
                throw std::runtime_error("push : attend 2 arguments (tableau, valeur)");
            if (auto *arr = getArray(args[0])) { arr->elements.push_back(args[1]); return {}; }
            throw std::runtime_error("push : premier argument doit être un tableau");
        };

        nativeFunctions["pop"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1) throw std::runtime_error("pop : attend 1 argument");
            if (auto *arr = getArray(args[0])) {
                if (arr->elements.empty()) throw std::runtime_error("pop : tableau vide");
                Value last = arr->elements.back();
                arr->elements.pop_back();
                return last;
            }
            throw std::runtime_error("pop : argument doit être un tableau");
        };

        nativeFunctions["input"] = [](std::vector<Value> args) -> Value {
            if (!args.empty() && std::holds_alternative<std::string>(args[0]))
                std::cout << std::get<std::string>(args[0]) << std::flush;
            std::string line;
            std::getline(std::cin, line);
            return Value{line};
        };

        nativeFunctions["num"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1) throw std::runtime_error("num : attend 1 argument");
            if (std::holds_alternative<double>(args[0])) return args[0];
            if (std::holds_alternative<std::string>(args[0])) {
                try { return Value{std::stod(std::get<std::string>(args[0]))}; }
                catch (...) {
                    throw std::runtime_error("num : impossible de convertir '"
                        + std::get<std::string>(args[0]) + "' en nombre");
                }
            }
            throw std::runtime_error("num : attend une chaîne ou un nombre");
        };

        nativeFunctions["str"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1) throw std::runtime_error("str : attend 1 argument");
            return Value{valueToString(args[0])};
        };

        nativeFunctions["upper"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("upper : attend une chaîne");
            std::string s = std::get<std::string>(args[0]);
            for (char &c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            return Value{s};
        };

        nativeFunctions["lower"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("lower : attend une chaîne");
            std::string s = std::get<std::string>(args[0]);
            for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return Value{s};
        };

        nativeFunctions["trim"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("trim : attend une chaîne");
            const std::string &s  = std::get<std::string>(args[0]);
            const std::string  ws = " \t\n\r\f\v";
            size_t start = s.find_first_not_of(ws);
            if (start == std::string::npos) return Value{std::string("")};
            size_t end = s.find_last_not_of(ws);
            return Value{s.substr(start, end - start + 1)};
        };

        nativeFunctions["split"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 2
                || !std::holds_alternative<std::string>(args[0])
                || !std::holds_alternative<std::string>(args[1]))
                throw std::runtime_error("split : attend (chaîne, séparateur)");
            const std::string &s   = std::get<std::string>(args[0]);
            const std::string &sep = std::get<std::string>(args[1]);
            std::vector<Value> parts;
            if (sep.empty()) {
                for (char c : s) parts.push_back(Value{std::string(1, c)});
            } else {
                size_t pos = 0, found;
                while ((found = s.find(sep, pos)) != std::string::npos) {
                    parts.push_back(Value{s.substr(pos, found - pos)});
                    pos = found + sep.size();
                }
                parts.push_back(Value{s.substr(pos)});
            }
            return Value{std::make_shared<ArrayObject>(std::move(parts))};
        };

        nativeFunctions["find"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 2
                || !std::holds_alternative<std::string>(args[0])
                || !std::holds_alternative<std::string>(args[1]))
                throw std::runtime_error("find : attend (chaîne, sous-chaîne)");
            const std::string &s   = std::get<std::string>(args[0]);
            const std::string &sub = std::get<std::string>(args[1]);
            size_t pos = s.find(sub);
            return Value{pos == std::string::npos ? -1.0 : static_cast<double>(pos)};
        };

        nativeFunctions["replace"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 3
                || !std::holds_alternative<std::string>(args[0])
                || !std::holds_alternative<std::string>(args[1])
                || !std::holds_alternative<std::string>(args[2]))
                throw std::runtime_error("replace : attend (chaîne, ancien, nouveau)");
            std::string s           = std::get<std::string>(args[0]);
            const std::string &from = std::get<std::string>(args[1]);
            const std::string &to   = std::get<std::string>(args[2]);
            if (!from.empty()) {
                size_t pos = 0;
                while ((pos = s.find(from, pos)) != std::string::npos) {
                    s.replace(pos, from.size(), to);
                    pos += to.size();
                }
            }
            return Value{s};
        };

        nativeFunctions["type"] = [](std::vector<Value> args) -> Value {
            if (args.size() != 1) throw std::runtime_error("type : attend 1 argument");
            if (std::holds_alternative<std::monostate>(args[0]))                   return Value{std::string("void")};
            if (std::holds_alternative<double>(args[0]))                           return Value{std::string("nombre")};
            if (std::holds_alternative<std::string>(args[0]))                      return Value{std::string("chaine")};
            if (std::holds_alternative<std::shared_ptr<ArrayObject>>(args[0]))     return Value{std::string("tableau")};
            if (std::holds_alternative<std::shared_ptr<NativeObject>>(args[0]))  return Value{std::string("objet")};
            return Value{std::string("inconnu")};
        };

        nativeFunctions["print_raw"] = [](std::vector<Value> args) -> Value {
            for (size_t i = 0; i < args.size(); ++i) {
                if (i > 0) std::cout << " ";
                std::cout << valueToString(args[i]);
            }
            std::cout << std::flush;
            return {};
        };

        nativeFunctions["assert"] = [](std::vector<Value> args) -> Value {
            if (args.empty() || args.size() > 2)
                throw std::runtime_error("assert : attend 1 ou 2 arguments");
            bool ok = false;
            if (std::holds_alternative<double>(args[0]))
                ok = std::get<double>(args[0]) != 0.0;
            else if (std::holds_alternative<std::string>(args[0]))
                ok = !std::get<std::string>(args[0]).empty();
            if (!ok) {
                std::string msg = "Assertion échouée";
                if (args.size() == 2 && std::holds_alternative<std::string>(args[1]))
                    msg += " : " + std::get<std::string>(args[1]);
                throw std::runtime_error(msg);
            }
            return {};
        };
    }

    // ------------------------------------------------
    // Exécution des instructions — switch sur NodeKind
    // ------------------------------------------------

    void execBlock(Block *block, std::shared_ptr<Environment> env) {
        for (auto &stmt : block->statements) {
            try {
                execNode(stmt.get(), env);
            } catch (ReturnException &)   { throw; }
              catch (BreakException &)    { throw; }
              catch (ContinueException &) { throw; }
              catch (std::runtime_error &e) {
                if (stmt->line > 0) {
                    std::string msg = e.what();
                    if (msg.rfind("Ligne ", 0) != 0)
                        throw std::runtime_error(
                            "Ligne " + std::to_string(stmt->line) + " : " + msg);
                }
                throw;
            }
        }
    }

    void execNode(ASTNode *node, std::shared_ptr<Environment> env) {
        switch (node->kind) {

        case NodeKind::VarDecl: {
            auto *n = static_cast<VarDecl*>(node);
            env->set(n->name, evalExpr(n->initializer.get(), env));
            return;
        }

        case NodeKind::FnDecl: {
            auto *n = static_cast<FnDecl*>(node);
            userFunctions[n->name] = UserFunction{n->params, static_cast<Block*>(n->body.get()), env};
            return;
        }

        case NodeKind::WhileStmt: {
            auto *n = static_cast<WhileStmt*>(node);
            auto loopEnv = std::make_shared<Environment>(env);
            while (isTruthy(evalExpr(n->condition.get(), env))) {
                checkInterrupt();
                try {
                    execBlock(static_cast<Block*>(n->body.get()), loopEnv);
                } catch (ReturnException &) { throw; }
                  catch (BreakException &)  { break; }
                  catch (ContinueException &) {}
            }
            return;
        }

        case NodeKind::ForStmt: {
            auto *n = static_cast<ForStmt*>(node);
            auto loopEnv = std::make_shared<Environment>(env);
            if (n->initializer) {
                if (n->initializer->kind == NodeKind::VarDecl) {
                    auto *vd = static_cast<VarDecl*>(n->initializer.get());
                    loopEnv->set(vd->name, evalExpr(vd->initializer.get(), loopEnv));
                } else {
                    execNode(n->initializer.get(), loopEnv);
                }
            }
            while (isTruthy(evalExpr(n->condition.get(), loopEnv))) {
                checkInterrupt();
                try {
                    execBlock(static_cast<Block*>(n->body.get()), loopEnv);
                } catch (ReturnException &) { throw; }
                  catch (BreakException &)  { break; }
                  catch (ContinueException &) {}
                if (n->increment) execNode(n->increment.get(), loopEnv);
            }
            return;
        }

        case NodeKind::ForInStmt: {
            auto *n = static_cast<ForInStmt*>(node);
            auto loopEnv = std::make_shared<Environment>(env);
            Value iterableVal = evalExpr(n->iterable.get(), loopEnv);
            auto *arr = getArray(iterableVal);
            if (!arr) throw std::runtime_error("for...in nécessite un tableau");
            for (size_t i = 0; i < arr->length(); ++i) {
                checkInterrupt();
                loopEnv->set(n->varName, arr->get(i));
                try {
                    execBlock(static_cast<Block*>(n->body.get()), loopEnv);
                } catch (ReturnException &) { throw; }
                  catch (BreakException &)  { break; }
                  catch (ContinueException &) {}
            }
            return;
        }

        case NodeKind::IfStmt: {
            auto *n = static_cast<IfStmt*>(node);
            if (isTruthy(evalExpr(n->condition.get(), env))) {
                auto thenEnv = std::make_shared<Environment>(env);
                execBlock(static_cast<Block*>(n->thenBranch.get()), thenEnv);
            } else if (n->elseBranch) {
                if (n->elseBranch->kind == NodeKind::Block) {
                    auto elseEnv = std::make_shared<Environment>(env);
                    execBlock(static_cast<Block*>(n->elseBranch.get()), elseEnv);
                } else {
                    execNode(n->elseBranch.get(), env);
                }
            }
            return;
        }

        case NodeKind::PrintStmt:
            std::cout << formatString(static_cast<PrintStmt*>(node)->text, env) << std::endl;
            return;

        case NodeKind::BreakStmt:
            throw BreakException{};

        case NodeKind::ContinueStmt:
            throw ContinueException{};

        case NodeKind::ReturnStmt: {
            auto *n = static_cast<ReturnStmt*>(node);
            Value val = n->value ? evalExpr(n->value.get(), env) : Value{std::monostate{}};
            throw ReturnException{std::move(val)};
        }

        case NodeKind::Block: {
            auto inner = std::make_shared<Environment>(env);
            execBlock(static_cast<Block*>(node), inner);
            return;
        }

        case NodeKind::AssignExpr:
            evalExpr(node, env);
            return;

        case NodeKind::ExprStmt:
            evalExpr(static_cast<ExprStmt*>(node)->expr.get(), env);
            return;

        default:
            throw std::runtime_error("Nœud inconnu dans execNode");
        }
    }

    // ------------------------------------------------
    // Évaluation des expressions — switch sur NodeKind
    // ------------------------------------------------

    Value evalExpr(ASTNode *node, std::shared_ptr<Environment> env) {
        switch (node->kind) {

        case NodeKind::NumberLiteral:
            return Value{static_cast<NumberLiteral*>(node)->value};

        case NodeKind::StringLiteral:
            return Value{static_cast<StringLiteral*>(node)->value};

        case NodeKind::ArrayLiteral: {
            auto *n = static_cast<ArrayLiteral*>(node);
            std::vector<Value> elements;
            elements.reserve(n->elements.size());
            for (auto &elem : n->elements)
                elements.push_back(evalExpr(elem.get(), env));
            return Value{std::make_shared<ArrayObject>(std::move(elements))};
        }

        case NodeKind::IndexExpr: {
            auto *n = static_cast<IndexExpr*>(node);
            Value arrVal = evalExpr(n->array.get(), env);
            Value idxVal = evalExpr(n->index.get(), env);
            auto *arr = getArray(arrVal);
            if (!arr) throw std::runtime_error("Accès par index sur un non-tableau");
            if (!std::holds_alternative<double>(idxVal))
                throw std::runtime_error("Index doit être un nombre");
            int index = static_cast<int>(std::get<double>(idxVal));
            if (index < 0 || index >= static_cast<int>(arr->length()))
                throw std::runtime_error("Index hors limites : " + std::to_string(index));
            return arr->get(static_cast<size_t>(index));
        }

        case NodeKind::VarExpr:
            return env->get(static_cast<VarExpr*>(node)->name);

        case NodeKind::MemberExpr: {
            auto *n = static_cast<MemberExpr*>(node);
            Value obj = env->get(n->object);
            if (n->member == "length") {
                if (auto *arr = getArray(obj))
                    return Value{static_cast<double>(arr->length())};
            }
            if (auto *ro = getNative(obj))
                return ro->getters.at(n->member)();
            throw std::runtime_error("Accès membre sur un non-objet : " + n->object);
        }

        case NodeKind::AssignExpr: {
            auto *n = static_cast<AssignExpr*>(node);
            Value val = evalExpr(n->value.get(), env);
            if (n->indexExpr) {
                auto *idx = static_cast<IndexExpr*>(n->indexExpr.get());
                Value arrVal = evalExpr(idx->array.get(), env);
                Value idxVal = evalExpr(idx->index.get(), env);
                auto *arr = getArray(arrVal);
                if (!arr) throw std::runtime_error("Affectation par index sur un non-tableau");
                if (!std::holds_alternative<double>(idxVal))
                    throw std::runtime_error("Index doit être un nombre");
                int index = static_cast<int>(std::get<double>(idxVal));
                if (index < 0 || index >= static_cast<int>(arr->length()))
                    throw std::runtime_error("Index hors limites : " + std::to_string(index));
                arr->set(static_cast<size_t>(index), val);
                return val;
            }
            if (n->object.empty()) {
                env->assign(n->name, val);
            } else {
                Value obj = env->get(n->object);
                if (auto *ro = getNative(obj))
                    ro->setters.at(n->name)(val);
                else
                    throw std::runtime_error("Setter sur un non-objet : " + n->object);
            }
            return val;
        }

        case NodeKind::BinOpExpr:
            return evalBinOp(static_cast<BinOpExpr*>(node), env);

        case NodeKind::CallExpr:
            return evalCall(static_cast<CallExpr*>(node), env);

        default:
            throw std::runtime_error("Expression inconnue dans evalExpr");
        }
    }

    // ------------------------------------------------
    // Opérations binaires — switch sur OpKind + court-circuit
    // ------------------------------------------------

    Value evalBinOp(BinOpExpr *n, std::shared_ptr<Environment> env) {
        // Court-circuit pour && et ||
        if (n->op == OpKind::And) {
            if (!isTruthy(evalExpr(n->left.get(), env))) return Value{0.0};
            return Value{isTruthy(evalExpr(n->right.get(), env)) ? 1.0 : 0.0};
        }
        if (n->op == OpKind::Or) {
            if (isTruthy(evalExpr(n->left.get(), env))) return Value{1.0};
            return Value{isTruthy(evalExpr(n->right.get(), env)) ? 1.0 : 0.0};
        }

        Value l = evalExpr(n->left.get(), env);
        Value r = evalExpr(n->right.get(), env);

        if (std::holds_alternative<double>(l) && std::holds_alternative<double>(r)) {
            double a = std::get<double>(l), b = std::get<double>(r);
            switch (n->op) {
            case OpKind::Add:    return Value{a + b};
            case OpKind::Sub:    return Value{a - b};
            case OpKind::Mul:    return Value{a * b};
            case OpKind::Div:
                if (b == 0.0) throw std::runtime_error("Division par zéro");
                return Value{a / b};
            case OpKind::Mod:
                if (b == 0.0) throw std::runtime_error("Modulo par zéro");
                return Value{std::fmod(a, b)};
            case OpKind::Pow:    return Value{std::pow(a, b)};
            case OpKind::IntDiv:
                if (b == 0.0) throw std::runtime_error("Division entière par zéro");
                return Value{std::floor(a / b)};
            case OpKind::Eq:     return Value{a == b ? 1.0 : 0.0};
            case OpKind::Ne:     return Value{a != b ? 1.0 : 0.0};
            case OpKind::Lt:     return Value{a <  b ? 1.0 : 0.0};
            case OpKind::Gt:     return Value{a >  b ? 1.0 : 0.0};
            case OpKind::Le:     return Value{a <= b ? 1.0 : 0.0};
            case OpKind::Ge:     return Value{a >= b ? 1.0 : 0.0};
            case OpKind::Not:    return Value{b == 0.0 ? 1.0 : 0.0};
            default: break;
            }
        }

        if (std::holds_alternative<std::string>(l) && std::holds_alternative<std::string>(r)) {
            const std::string &a = std::get<std::string>(l), &b = std::get<std::string>(r);
            switch (n->op) {
            case OpKind::Add: return Value{a + b};
            case OpKind::Eq:  return Value{a == b ? 1.0 : 0.0};
            case OpKind::Ne:  return Value{a != b ? 1.0 : 0.0};
            case OpKind::Lt:  return Value{a <  b ? 1.0 : 0.0};
            case OpKind::Gt:  return Value{a >  b ? 1.0 : 0.0};
            case OpKind::Le:  return Value{a <= b ? 1.0 : 0.0};
            case OpKind::Ge:  return Value{a >= b ? 1.0 : 0.0};
            default: break;
            }
        }

        throw std::runtime_error("Opération non supportée entre ces types");
    }

    // ------------------------------------------------
    // Appels de fonctions / méthodes
    // ------------------------------------------------

    Value evalCall(CallExpr *n, std::shared_ptr<Environment> env) {
        std::vector<Value> args;
        args.reserve(n->args.size());
        for (auto &a : n->args)
            args.push_back(evalExpr(a.get(), env));

        if (!n->object.empty()) {
            Value obj = env->get(n->object);
            if (auto *ro = getNative(obj)) {
                auto it = ro->methods.find(n->method);
                if (it == ro->methods.end())
                    throw std::runtime_error("Méthode inconnue : " + n->method);
                return it->second(args);
            }
            throw std::runtime_error("Appel de méthode sur un non-objet : " + n->object);
        }

        // Fonctions utilisateur en premier (plus fréquentes dans les scripts)
        {
            auto it = userFunctions.find(n->method);
            if (it != userFunctions.end()) {
                auto &fn = it->second;
                if (args.size() != fn.params.size())
                    throw std::runtime_error("'" + n->method + "' attend " +
                                             std::to_string(fn.params.size()) + " argument(s)");
                auto fnEnv = std::make_shared<Environment>(fn.closure);
                for (size_t i = 0; i < fn.params.size(); ++i)
                    fnEnv->set(fn.params[i], args[i]);
                try {
                    execBlock(fn.body, fnEnv);
                } catch (ReturnException &r) {
                    return r.value;
                }
                return Value{std::monostate{}};
            }
        }

        {
            auto it = nativeFunctions.find(n->method);
            if (it != nativeFunctions.end()) return it->second(args);
        }

        {
            auto it = constructors.find(n->method);
            if (it != constructors.end()) return it->second();
        }

        throw std::runtime_error("Fonction inconnue : '" + n->method + "'");
    }

    // ------------------------------------------------
    // Utilitaires
    // ------------------------------------------------

    bool isTruthy(const Value &v) {
        if (std::holds_alternative<double>(v))
            return std::get<double>(v) != 0.0;
        if (std::holds_alternative<std::string>(v))
            return !std::get<std::string>(v).empty();
        return false;
    }

    std::string formatString(const std::string &fmt, std::shared_ptr<Environment> env) {
        std::ostringstream result;
        size_t pos = 0;

        while (pos < fmt.size()) {
            size_t openBrace = fmt.find('{', pos);
            if (openBrace == std::string::npos) { result << fmt.substr(pos); break; }

            result << fmt.substr(pos, openBrace - pos);

            size_t closeBrace = fmt.find('}', openBrace);
            if (closeBrace == std::string::npos)
                throw std::runtime_error("Accolade fermante manquante dans la chaîne de format");

            std::string placeholder = fmt.substr(openBrace + 1, closeBrace - openBrace - 1);

            size_t colonPos = placeholder.find(':');
            std::string exprStr    = colonPos != std::string::npos
                                     ? placeholder.substr(0, colonPos) : placeholder;
            std::string formatSpec = colonPos != std::string::npos
                                     ? placeholder.substr(colonPos + 1) : "";

            Value val;
            try {
                Lexer  lexer(exprStr);
                auto   tokens = lexer.tokenize();
                Parser parser(std::move(tokens));
                auto   exprNode = parser.parseExpression();
                val = evalExpr(exprNode.get(), env);
            } catch (const std::exception &e) {
                throw std::runtime_error(
                    std::string("Interpolation '{") + exprStr + "}' : " + e.what());
            }

            if (std::holds_alternative<double>(val) && !formatSpec.empty()) {
                double num_val = std::get<double>(val);
                if (formatSpec.find('.') != std::string::npos && formatSpec.back() == 'f') {
                    size_t dotPos = formatSpec.find('.');
                    int precision = std::stoi(formatSpec.substr(dotPos + 1,
                                               formatSpec.size() - dotPos - 1));
                    result << std::fixed << std::setprecision(precision) << num_val;
                    result << std::defaultfloat;
                } else {
                    result << num_val;
                }
            } else {
                result << valueToString(val);
            }

            pos = closeBrace + 1;
        }

        return result.str();
    }

    NativeObject *getNative(const Value &v) {
        if (std::holds_alternative<std::shared_ptr<NativeObject>>(v))
            return std::get<std::shared_ptr<NativeObject>>(v).get();
        return nullptr;
    }
};
