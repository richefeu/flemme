// main.cpp
// Point d'entrée de l'interpréteur.
// Usage : ./flemme [--no-plugin <nom>]... <script.flm>
#include "plugins.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>

int main(int argc, char *argv[]) {
  std::unordered_set<std::string> disabledPlugins;
  std::string scriptPath;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--no-plugin") {
      if (i + 1 >= argc) {
        std::cerr << "--no-plugin : nom de plugin manquant\n";
        return 1;
      }
      disabledPlugins.insert(argv[++i]);
    } else if (arg.rfind("--no-plugin=", 0) == 0) {
      disabledPlugins.insert(arg.substr(12));
    } else if (arg == "-v" || arg == "--version") {
      std::cout << "Ni Python, ni C++, juste flemme v0.1 — Un langage qui se la coule douce.\n";
      return 0;
    } else if (arg == "-h" || arg == "--help") {
      std::cout << "Usage : flemme [--no-plugin <nom>]... <script.flm>\n"
                   "  --no-plugin timer          desactive le plugin Timer au demarrage\n"
                   "  --no-plugin stats          desactive le plugin Stats\n"
                   "  --no-plugin mockrockable   desactive le plugin MockRockable\n"
                   "  -v, --version              affiche la version\n";
      return 0;
    } else if (!scriptPath.empty()) {
      std::cerr << "Argument inattendu : " << arg << "\n";
      return 1;
    } else {
      scriptPath = arg;
    }
  }

  if (scriptPath.empty()) {
    std::cerr << "Usage : " << argv[0] << " [--no-plugin <nom>]... <script.flm>\n";
    return 1;
  }

  std::ifstream file(scriptPath);
  if (!file) {
    std::cerr << "Impossible d'ouvrir : " << scriptPath << "\n";
    return 1;
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  std::string source = ss.str();

  try {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Parser parser(std::move(tokens));
    auto ast = parser.parse();

    Interpreter interp;
    registerPlugins(interp, disabledPlugins);

    interp.run(ast.get());

  } catch (const std::exception &e) {
    std::cerr << "Erreur : " << e.what() << "\n";
    return 1;
  }

  return 0;
}
