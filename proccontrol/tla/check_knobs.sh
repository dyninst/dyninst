#!/bin/bash
# Regression sweep for ProcControlLocks.tla: run TLC once per BUG knob with
# that knob flipped TRUE (others FALSE).  Every knob must be CAUGHT (TLC
# reports a deadlock or invariant/temporal violation); a knob that comes back
# clean means the model lost the failure it was built to remember.
# Run the default config (all knobs FALSE, must be clean) separately:
#   $JAVA -XX:+UseParallelGC -cp tla2tools.jar tlc2.TLC -workers 4 ProcControlLocks.tla
set -u
JAVA=${JAVA:-java}
command -v "$JAVA" >/dev/null || JAVA=~/tools/jdk-17.0.19+10-jre/bin/java
TLA_DIR=$(cd "$(dirname "$0")" && pwd)
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# Knob list comes from the cfg so the two never drift.
KNOBS=$(sed -n 's/^  \(BUG[0-9A-Za-z_]*\) = FALSE$/\1/p' "$TLA_DIR/ProcControlLocks.cfg")
[ -n "$KNOBS" ] || { echo "no knobs found in cfg"; exit 1; }

fail=0
for knob in $KNOBS; do
  d="$WORK/$knob"; mkdir -p "$d"
  cp "$TLA_DIR/ProcControlLocks.tla" "$d/"
  sed "s/^  $knob = FALSE/  $knob = TRUE/" "$TLA_DIR/ProcControlLocks.cfg" > "$d/ProcControlLocks.cfg"
  ( cd "$d" && "$JAVA" -Xmx3g -XX:+UseParallelGC -cp "$TLA_DIR/tla2tools.jar" \
      tlc2.TLC -workers 2 ProcControlLocks.tla ) > "$d/tlc.log" 2>&1
  if grep -qE "^Error:" "$d/tlc.log"; then
    echo "$knob: CAUGHT -- $(grep -m1 -E '^Error:' "$d/tlc.log")"
  else
    echo "$knob: *** NOT CAUGHT *** model regression (log: $d/tlc.log)"
    fail=1
  fi
done
exit $fail
