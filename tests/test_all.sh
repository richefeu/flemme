#!/bin/bash
# test_all.sh — lance tous les tests .flm et vérifie qu'ils s'exécutent sans erreur.
# Usage :  bash test_all.sh          (depuis le répertoire flemme/)
#          bash test_all.sh -v       (affiche la sortie de chaque test)

VERBOSE=0
[ "$1" = "-v" ] && VERBOSE=1

BIN="../flemme"
PASS=0; FAIL=0; SKIP=0

# Tests à ignorer (nécessitent une saisie interactive)
SKIP_LIST="input.flm"

run_test() {
  local f="$1"
  # Vérifier si le fichier doit être ignoré
  for skip in $SKIP_LIST; do
    [ "$(basename "$f")" = "$skip" ] && return 2
  done
  output=$("$BIN" "$f" 2>&1)
  return $?
}

for f in *.flm; do
  [ -f "$f" ] || continue

  # Vérification de la liste d'exclusion
  skip=0
  for s in $SKIP_LIST; do [ "$(basename "$f")" = "$s" ] && skip=1 && break; done
  if [ $skip -eq 1 ]; then
    echo "[SKIP] $f  (saisie interactive)"
    SKIP=$((SKIP+1))
    continue
  fi

  output=$("$BIN" "$f" 2>&1)
  rc=$?
  if [ $rc -eq 0 ]; then
    echo "[OK]   $f"
    [ $VERBOSE -eq 1 ] && echo "$output" | sed 's/^/       /'
    PASS=$((PASS+1))
  else
    echo "[FAIL] $f"
    echo "$output" | head -5 | sed 's/^/       → /'
    FAIL=$((FAIL+1))
  fi
done

echo ""
echo "─────────────────────────────────────"
echo "Résultat : ${PASS} OK  ${FAIL} FAIL  ${SKIP} SKIP"
echo "─────────────────────────────────────"
[ $FAIL -eq 0 ]
