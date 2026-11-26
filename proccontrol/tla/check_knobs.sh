#!/bin/bash
# Regression sweep for a ProcControl TLA model: run TLC once per BUG knob
# with that knob flipped TRUE (others FALSE).  Every knob must be CAUGHT
# (TLC reports a deadlock or invariant/temporal violation); a knob that
# comes back clean means the model lost the failure it was built to
# remember.  Usage: check_knobs.sh [module]   (default: ProcControlLocks;
# also: ProcControlLifetime).  Run the default config (all knobs FALSE,
# must be clean) separately:
#   $JAVA -XX:+UseParallelGC -cp tla2tools.jar tlc2.TLC -workers 4 <module>.tla
set -u
MODULE=${1:-ProcControlLocks}
JAVA=${JAVA:-java}
command -v "$JAVA" >/dev/null || JAVA=~/tools/jdk-17.0.19+10-jre/bin/java
TLA_DIR=$(cd "$(dirname "$0")" && pwd)
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# Knob list comes from the cfg so the two never drift.
KNOBS=$(sed -n 's/^  \(BUG[0-9A-Za-z_]*\) = FALSE$/\1/p' "$TLA_DIR/$MODULE.cfg")
[ -n "$KNOBS" ] || { echo "no knobs found in cfg"; exit 1; }

fail=0
for knob in $KNOBS; do
  d="$WORK/$knob"; mkdir -p "$d"
  cp "$TLA_DIR/$MODULE.tla" "$d/"
  sed "s/^  $knob = FALSE/  $knob = TRUE/" "$TLA_DIR/$MODULE.cfg" > "$d/$MODULE.cfg"
  ( cd "$d" && "$JAVA" -Xmx3g -XX:+UseParallelGC -cp "$TLA_DIR/tla2tools.jar" \
      tlc2.TLC -workers 2 "$MODULE.tla" ) > "$d/tlc.log" 2>&1
  if grep -qE "^Error:" "$d/tlc.log"; then
    echo "$knob: CAUGHT -- $(grep -m1 -E '^Error:' "$d/tlc.log")"
  else
    echo "$knob: *** NOT CAUGHT *** model regression (log: $d/tlc.log)"
    fail=1
  fi
done
exit $fail
