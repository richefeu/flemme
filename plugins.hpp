// plugins.hpp
// Liste des plugins actifs dans l'interpréteur flemme.
//
// Pour ajouter un plugin :
//   1. Créer plugins/monoutil/ avec MonOutil.hpp, MonOutil.cpp, Makefile
//   2. Écrire plugins/monoutil/monoutil_plugin.hpp  (voir timer ou stats comme modèle)
//   3. Ajouter le bloc #ifdef / #include / register ci-dessous
//   4. Ajouter DISABLE_MONOUTIL dans le Makefile principal (PLUGIN_LIBS/INCS/DEFS)
#pragma once
#include "interpreter.hpp"
#include <string>
#include <unordered_set>

// ── Plugins actifs (inclus seulement si compilés avec -DFLEMME_PLUGIN_*) ──

#ifdef FLEMME_PLUGIN_TIMER
#  include "plugins/timer/timer_plugin.hpp"
#endif

#ifdef FLEMME_PLUGIN_STATS
#  include "plugins/stats/stats_plugin.hpp"
#endif

#ifdef FLEMME_PLUGIN_MOCKROCKABLE
#  include "plugins/mockrockable/mockrockable_plugin.hpp"
#endif

#ifdef FLEMME_PLUGIN_PLOTSVG
#  include "plugins/plotsvg/plotsvg_plugin.hpp"
#endif

#ifdef FLEMME_PLUGIN_SCHEMA
#  include "plugins/schema/schema_plugin.hpp"
#endif

// Plugin Rockable réel (décommenter + fournir la lib)
// #ifdef FLEMME_PLUGIN_ROCKABLE
// #  include "rockable_plugin.hpp"
// #endif

// ─────────────────────────────────────────────────────────────────────
// disabled : noms à exclure à l'exécution (ex: {"timer", "stats"})
// ─────────────────────────────────────────────────────────────────────

inline void registerPlugins(Interpreter& interp,
                            const std::unordered_set<std::string>& disabled = {}) {
#ifdef FLEMME_PLUGIN_TIMER
    if (!disabled.count("timer"))        registerTimerPlugin(interp);
#endif
#ifdef FLEMME_PLUGIN_STATS
    if (!disabled.count("stats"))        registerStatsPlugin(interp);
#endif
#ifdef FLEMME_PLUGIN_MOCKROCKABLE
    if (!disabled.count("mockrockable")) registerMockRockablePlugin(interp);
#endif
#ifdef FLEMME_PLUGIN_PLOTSVG
    if (!disabled.count("plotsvg"))      registerPlotSVGPlugin(interp);
#endif
#ifdef FLEMME_PLUGIN_SCHEMA
    if (!disabled.count("schema"))       registerSchemaPlugin(interp);
#endif
    // if (!disabled.count("rockable"))  registerRockablePlugin(interp);
}
