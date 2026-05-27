// webide.cpp — Flemme Web IDE
// Compile via Makefile : make web
// Nécessite vendor/httplib.h (téléchargé automatiquement par le Makefile)

#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "plugins.hpp"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>

#include "httplib.h"

// ─── Utilitaire JSON ─────────────────────────────────────────────────────────

static std::string jsonEscape(const std::string &s) {
  std::string out;
  out.reserve(s.size() + 16);
  for (unsigned char c : s) {
    switch (c) {
    case '"':
      out += "\\\"";
      break;
    case '\\':
      out += "\\\\";
      break;
    case '\n':
      out += "\\n";
      break;
    case '\r':
      out += "\\r";
      break;
    case '\t':
      out += "\\t";
      break;
    default:
      if (c < 0x20) {
        char buf[8];
        std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned)c);
        out += buf;
      } else {
        out += (char)c;
      }
    }
  }
  return out;
}

// ─── Page HTML embarquée ─────────────────────────────────────────────────────

static const char *HTML_PAGE = R"WEBIDE(<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Flemme IDE</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
    :root {
      --bg: #1e1e2e; --bg2: #181825; --border: #313244;
      --fg: #cdd6f4;  --fg2: #6c7086; --fg3: #45475a;
      --green: #a6e3a1; --red: #f38ba8; --purple: #cba6f7;
      --teal: #94e2d5; --yellow: #f9e2af;
      --font-mono: 'Courier New', Courier, monospace;
      --lh: 1.55; --fs: 0.9rem;
    }
    body {
      font-family: system-ui, -apple-system, sans-serif;
      background: var(--bg); color: var(--fg);
      height: 100vh; display: flex; flex-direction: column; overflow: hidden;
    }
    header {
      display: flex; align-items: center; gap: 10px;
      padding: 7px 14px; background: var(--bg2);
      border-bottom: 1px solid var(--border); flex-shrink: 0; flex-wrap: wrap;
    }
    h1 { font-size: 0.95rem; font-weight: 700; color: var(--purple); letter-spacing: .06em; }
    select, button {
      padding: 4px 10px; border: 1px solid var(--border); border-radius: 4px;
      font-size: 0.82rem; cursor: pointer; background: var(--bg); color: var(--fg);
    }
    #runBtn { background: var(--green); color: var(--bg); border-color: var(--green); font-weight: 700; }
    #runBtn:hover { background: var(--teal); border-color: var(--teal); }
    #runBtn:disabled { opacity: .45; cursor: not-allowed; }
    button:hover:not(:disabled) { border-color: var(--purple); }
    #status { font-size: 0.78rem; color: var(--fg2); margin-left: auto; white-space: nowrap; }
    main { display: flex; flex: 1; overflow: hidden; }
    .panel { display: flex; flex-direction: column; flex: 1; min-width: 0; }
    .panel-title {
      padding: 3px 10px; font-size: 0.68rem; font-weight: 600;
      text-transform: uppercase; letter-spacing: .1em; color: var(--fg2);
      background: var(--bg2); border-bottom: 1px solid var(--border); flex-shrink: 0;
    }
    /* Editeur avec numéros de ligne */
    .editor-wrap {
      display: flex; flex: 1; overflow: hidden;
      border-right: 1px solid var(--border);
    }
    #lineNums {
      padding: 10px 8px 10px 10px; min-width: 3rem; text-align: right;
      font-family: var(--font-mono); font-size: var(--fs); line-height: var(--lh);
      color: var(--fg2); background: var(--bg2); border-right: 1px solid var(--border);
      overflow-y: scroll; overflow-x: hidden; scrollbar-width: none;
      user-select: none; flex-shrink: 0;
    }
    #lineNums::-webkit-scrollbar { display: none; }
    #editor {
      flex: 1; padding: 10px 12px;
      background: var(--bg); color: var(--fg);
      font-family: var(--font-mono); font-size: var(--fs); line-height: var(--lh);
      border: none; outline: none; resize: none; overflow-y: scroll;
    }
    #output-wrap { flex: 1; overflow-y: auto; }
    #output {
      padding: 10px 12px; font-family: var(--font-mono); font-size: 0.87rem;
      line-height: var(--lh); white-space: pre-wrap; word-break: break-all;
    }
    .out-normal { color: var(--fg); }
    .out-error  { color: var(--red); }
    .err-line   { background: rgba(243,139,168,.15); display: block; }
  </style>
</head>
<body>
<header>
  <h1>Flemme IDE</h1>
  <select id="examplesSelect" title="Charger un exemple">
    <option value="">— Exemples —</option>
    <option value="hello">Bonjour monde</option>
    <option value="facto">Factorielle</option>
    <option value="fib">Fibonacci</option>
    <option value="tri">Tri a bulles</option>
    <option value="chaines">Fonctions chaines</option>
  </select>
  <button id="runBtn">&#9654; Executer (Ctrl+Entree)</button>
  <input  type="file" id="fileInput" accept=".flm,.txt" style="display:none">
  <button id="openBtn"  title="Charger un fichier .flm">Ouvrir</button>
<button id="dlBtn"    title="Telecharger en .flm">Telecharger</button>
  <span id="status"></span>
</header>
<main>
  <div class="panel">
    <div class="panel-title">Code</div>
    <div class="editor-wrap">
      <pre id="lineNums">1</pre>
      <textarea id="editor" spellcheck="false" autocomplete="off"
        autocorrect="off" autocapitalize="off"
      >print "Bonjour, monde !";</textarea>
    </div>
  </div>
  <div class="panel">
    <div class="panel-title">Sortie</div>
    <div id="output-wrap"><pre id="output" class="out-normal"></pre></div>
  </div>
</main>
<script>
const editor  = document.getElementById('editor');
const lineNums= document.getElementById('lineNums');
const output  = document.getElementById('output');
const runBtn  = document.getElementById('runBtn');
const status  = document.getElementById('status');

// ── Exemples ──────────────────────────────────────────────────────────────────
const EXAMPLES = {
  hello:
`print "Bonjour, monde !";
let n = 7;
print "Le carre de {n} est {n * n}";`,

  facto:
`fn factorielle(n) {
  if (n <= 1) { return 1; }
  return n * factorielle(n - 1);
}
for (let i = 0; i <= 10; i++) {
  print "{i}! = {factorielle(i)}";
}`,

  fib:
`fn fib(n) {
  if (n <= 1) { return n; }
  return fib(n - 1) + fib(n - 2);
}
let i = 0;
while (fib(i) <= 100) {
  print "fib({i}) = {fib(i)}";
  i++;
}`,

  tri:
`fn tri_bulles(t) {
  let n = len(t);
  for (let i = 0; i < n - 1; i++) {
    for (let j = 0; j < n - i - 1; j++) {
      if (t[j] > t[j + 1]) {
        let tmp = t[j];
        t[j] = t[j + 1];
        t[j + 1] = tmp;
      }
    }
  }
}
let arr = [64, 34, 25, 12, 22, 11, 90];
print "Avant : {arr}";
tri_bulles(arr);
print "Apres  : {arr}";`,

  chaines:
`let s    = "  Bonjour le Monde  ";
let t    = trim(s);
let mots = split(t, " ");
let r    = replace(t, "Monde", "Flemme");
let pos  = find(t, "Monde");
print "original : '{s}'";
print "trim     : '{t}'";
print "upper    : '{upper(t)}'";
print "lower    : '{lower(t)}'";
print "mots     : {mots}  ({len(mots)} elements)";
print "replace  : '{r}'";
print "find     : position {pos}";`
};

document.getElementById('examplesSelect').addEventListener('change', function() {
  if (this.value && EXAMPLES[this.value]) {
    editor.value = EXAMPLES[this.value];
    updateLineNums();
    localStorage.setItem('flemme_code', editor.value);
  }
  this.value = '';
});

// ── Numéros de ligne ──────────────────────────────────────────────────────────
function updateLineNums() {
  const n = editor.value.split('\n').length;
  lineNums.textContent = Array.from({length: n}, (_, i) => i + 1).join('\n');
}
editor.addEventListener('input', () => {
  updateLineNums();
  localStorage.setItem('flemme_code', editor.value);
});
editor.addEventListener('scroll', () => { lineNums.scrollTop = editor.scrollTop; });

// ── Chargement d'un fichier local ─────────────────────────────────────────────
const fileInput = document.getElementById('fileInput');
document.getElementById('openBtn').addEventListener('click', () => fileInput.click());
fileInput.addEventListener('change', function() {
  const file = this.files[0];
  if (!file) return;
  const reader = new FileReader();
  reader.onload = e => {
    editor.value = e.target.result;
    updateLineNums();
    localStorage.setItem('flemme_code', editor.value);
    status.textContent = file.name + ' charge';
    setTimeout(() => { status.textContent = ''; }, 3000);
  };
  reader.readAsText(file);
  this.value = ''; // permet de recharger le meme fichier
});

// ── Telechargement ────────────────────────────────────────────────────────────
document.getElementById('dlBtn').addEventListener('click', () => {
  const blob = new Blob([editor.value], {type: 'text/plain'});
  const a = document.createElement('a');
  a.href = URL.createObjectURL(blob);
  a.download = 'programme.flm';
  a.click();
  URL.revokeObjectURL(a.href);
});

// ── Mise en evidence de la ligne en erreur ────────────────────────────────────
function gotoErrorLine(lineNum) {
  const lines = editor.value.split('\n');
  let pos = 0;
  for (let i = 0; i < lineNum - 1 && i < lines.length; i++) pos += lines[i].length + 1;
  editor.focus();
  editor.setSelectionRange(pos, pos + (lines[lineNum - 1] || '').length);
  const lh = parseFloat(getComputedStyle(editor).lineHeight) || 20;
  editor.scrollTop = Math.max(0, (lineNum - 1) * lh - editor.clientHeight / 2);
  lineNums.scrollTop = editor.scrollTop;
}

// ── Execution ─────────────────────────────────────────────────────────────────
async function run() {
  runBtn.disabled = true;
  status.textContent = 'En cours...';
  output.textContent = '';
  output.className = 'out-normal';
  try {
    const resp = await fetch('/run', {
      method: 'POST',
      headers: {'Content-Type': 'text/plain; charset=utf-8'},
      body: editor.value
    });
    const data = await resp.json();
    let text = data.output || '';
    if (data.error) {
      if (text) text += '\n';
      text += 'Erreur : ' + data.error;
      output.className = 'out-error';
      if (data.line > 0) gotoErrorLine(data.line);
    }
    output.textContent = text || '(aucune sortie)';
    status.textContent = data.ms + ' ms';
  } catch (err) {
    output.textContent = 'Erreur reseau : ' + err.message;
    output.className = 'out-error';
    status.textContent = '';
  }
  runBtn.disabled = false;
}

runBtn.addEventListener('click', run);
editor.addEventListener('keydown', function(e) {
  if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') { e.preventDefault(); run(); }
  if (e.key === 'Tab') {
    e.preventDefault();
    const s = editor.selectionStart, end = editor.selectionEnd;
    editor.value = editor.value.slice(0, s) + '  ' + editor.value.slice(end);
    editor.selectionStart = editor.selectionEnd = s + 2;
    updateLineNums();
  }
});

// ── Initialisation ────────────────────────────────────────────────────────────
(function init() {
  const saved = localStorage.getItem('flemme_code');
  if (saved) editor.value = saved;
  updateLineNums();
})();
</script>
</body>
</html>
)WEBIDE";

// ─── Plugins désactivés pour le web IDE ──────────────────────────────────────
// Contrôlés à la compilation via WEBIDE_NO_PLUGIN_* (cf. Makefile).

static const std::unordered_set<std::string> WEB_DISABLED = [] {
    std::unordered_set<std::string> s;
#ifdef WEBIDE_NO_PLUGIN_TIMER
    s.insert("timer");
#endif
#ifdef WEBIDE_NO_PLUGIN_STATS
    s.insert("stats");
#endif
#ifdef WEBIDE_NO_PLUGIN_MOCKROCKABLE
    s.insert("mockrockable");
#endif
    return s;
}();

// ─── Serveur ─────────────────────────────────────────────────────────────────

static std::mutex g_runMutex;
static const int PORT = 8765;

int main() {
  httplib::Server svr;

  svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
    res.set_content(HTML_PAGE, "text/html; charset=utf-8");
  });

  svr.Post("/run", [](const httplib::Request &req, httplib::Response &res) {
    // Sérialiser les exécutions (une à la fois)
    std::lock_guard<std::mutex> lock(g_runMutex);

    const std::string &code = req.body;

    // Capturer stdout dans un flux en mémoire
    std::ostringstream captured;
    std::streambuf *oldBuf = std::cout.rdbuf(captured.rdbuf());

    std::string errorMsg;
    long long ms = 0;

    {
      Interpreter interp;

      // Remplacer input() par une erreur explicite
      interp.registerFunction("input", [](std::vector<Value>) -> Value {
        throw std::runtime_error("input() n'est pas disponible en mode web");
      });

      registerPlugins(interp, WEB_DISABLED);

      // Chien de garde : interrompt l'exécution après 5 s
      std::atomic<bool> done{false};
      std::thread watchdog([&interp, &done]() {
        auto deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!done.load(std::memory_order_relaxed) &&
               std::chrono::steady_clock::now() < deadline)
          std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (!done.load(std::memory_order_relaxed))
          interp.shouldStop = true;
      });

      auto t0 = std::chrono::steady_clock::now();
      try {
        Lexer lexer(code);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));

        auto ast = parser.parse();
        interp.run(ast.get());
      } catch (const std::exception &e) {
        errorMsg = e.what();
      } catch (...) {
        errorMsg = "Erreur interne inconnue";
      }
      auto t1 = std::chrono::steady_clock::now();
      ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0)
               .count();

      done = true;
      if (watchdog.joinable())
        watchdog.join();
    }

    std::cout.rdbuf(oldBuf);

    std::string output = captured.str();
    // Supprimer le dernier \n (print ajoute endl)
    if (!output.empty() && output.back() == '\n')
      output.pop_back();

    // Extraire le numéro de ligne depuis "Ligne N : ..."
    int errLine = 0;
    if (!errorMsg.empty() && errorMsg.rfind("Ligne ", 0) == 0) {
      size_t colon = errorMsg.find(" : ");
      if (colon != std::string::npos) {
        try {
          errLine = std::stoi(errorMsg.substr(6, colon - 6));
        } catch (...) {
        }
      }
    }

    std::string json = "{\"output\":\"" + jsonEscape(output) +
                       "\",\"error\":\"" + jsonEscape(errorMsg) +
                       "\",\"line\":" + std::to_string(errLine) +
                       ",\"ms\":" + std::to_string(ms) + "}";

    res.set_content(json, "application/json");
  });

  std::cout << "Flemme Web IDE — http://localhost:" << PORT << "\n"
            << "Ctrl+C pour arreter." << std::endl;

  // Ouvrir le navigateur après un court délai
  std::thread([] {
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::string url = "http://localhost:" + std::to_string(PORT);
#if defined(_WIN32)
    std::system(("start " + url).c_str());
#elif defined(__APPLE__)
    std::system(("open " + url).c_str());
#else
    std::system(("xdg-open " + url + " 2>/dev/null &").c_str());
#endif
  }).detach();

  svr.listen("0.0.0.0", PORT);
  return 0;
}
