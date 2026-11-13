#!/bin/bash
# generate_noreturn_list.sh <noreturn_scan> <target library> <out dir> [<seed demangled-names file>]
#
# Builds a list of non-returning functions for <target library> from the code of
# the libraries it links against. See README.md.
#
# Output in <out dir>:
#   mangled.txt    the generated names: the mangled-names file of noreturn_check
#   demangled.txt  the seed names (empty without a seed): its demangled-names file
#   review.tsv     every EXPORT: mangled, demangled, library, SUSPECT reason
#                  (names with a reason are not in mangled.txt)
#   missing.txt    the target's MISSING lines: callees still to look at
#   <lib>.scan     raw noreturn_scan output per library
#
# SKIP_RE: extended regex of library file names to leave out (default: glibc,
# whose non-returning functions Dyninst already knows).
set -u
if [ $# -lt 3 ]; then
  sed -n '2,3p' "$0" >&2
  exit 1
fi
SCAN=$1
TARGET=$(readlink -f "$2")
OUT=$3
SEED=${4:-/dev/null}
SKIP_RE=${SKIP_RE:-'^(libc|libm|libdl|librt|libpthread|ld-linux[^.]*|libutil|libresolv)\.so'}
MAX_PASSES=6

mkdir -p "$OUT"
grep -vE '^[[:space:]]*(#|$)' "$SEED" > "$OUT/demangled.txt"
: > "$OUT/mangled.txt"

# File name -> path of every library the target loads.
declare -A path_of
while read -r name path; do
  [ -f "$path" ] && path_of[$name]=$(readlink -f "$path")
done < <(ldd "$TARGET" 2>/dev/null | awk '$2 == "=>" {print $1, $3}')

# Dependencies before dependents: depth-first post-order over DT_NEEDED.
declare -A visited
order=()
visit() {
  local lib=$1 needed
  [ -n "${visited[$lib]:-}" ] && return
  visited[$lib]=1
  for needed in $(readelf -d "$lib" 2>/dev/null | sed -n 's/.*(NEEDED).*\[\(.*\)\]/\1/p'); do
    [[ $needed =~ $SKIP_RE ]] && continue
    [ -n "${path_of[$needed]:-}" ] && visit "${path_of[$needed]}"
  done
  order+=("$lib")
}
visit "$TARGET"

# Adds the EXPORT names of a scan that have no SUSPECT line to mangled.txt.
add_trusted_exports() {
  awk -F'\t' '$1 ~ /^SUSPECT / {doubtful[substr($1, 9)] = 1; next}
              $1 ~ /^EXPORT /  {exported[substr($1, 8)] = 1}
              END {for (n in exported) if (!(n in doubtful)) print n}' "$1" |
    sort -u - "$OUT/mangled.txt" > "$OUT/mangled.tmp" && mv "$OUT/mangled.tmp" "$OUT/mangled.txt"
}

# A library calls its own exported functions through its own PLT stubs, which
# are not connected to the definitions while it is parsed, so each library is
# scanned again with its own names registered until no new name appears.
for lib in "${order[@]}"; do
  base=$(basename "$lib")
  for ((pass = 1; pass <= MAX_PASSES; pass++)); do
    "$SCAN" "$lib" "$OUT/mangled.txt" "$OUT/demangled.txt" > "$OUT/$base.scan" ||
      { echo "noreturn_scan failed on $lib" >&2; exit 1; }
    before=$(wc -l < "$OUT/mangled.txt")
    add_trusted_exports "$OUT/$base.scan"
    added=$(($(wc -l < "$OUT/mangled.txt") - before))
    echo "$base pass $pass: +$added names"
    [ "$added" -eq 0 ] && break
  done
done

target_scan="$OUT/$(basename "$TARGET").scan"
awk '$1 == "MISSING"' "$target_scan" | sort -k2,2nr > "$OUT/missing.txt"

printf 'mangled\tdemangled\tlibrary\treason\n' > "$OUT/review.tsv"
for lib in "${order[@]}"; do
  base=$(basename "$lib")
  awk -F'\t' -v lib="$base" '
    $1 ~ /^SUSPECT / {reason[substr($1, 9)] = $2; next}
    $1 ~ /^EXPORT /  {names[substr($1, 8)] = 1}
    END {for (n in names) print n "\t" lib "\t" reason[n]}' "$OUT/$base.scan" | sort
done | while IFS=$'\t' read -r mangled lib reason; do
  printf '%s\t%s\t%s\t%s\n' "$mangled" "$(c++filt <<< "$mangled")" "$lib" "$reason"
done >> "$OUT/review.tsv"

echo "names: $(wc -l < "$OUT/mangled.txt")," \
  "held back: $(awk -F'\t' 'NR > 1 && $4 != ""' "$OUT/review.tsv" | wc -l)," \
  "still missing: $(awk '{s += $2} END {print s + 0}' "$OUT/missing.txt") calls to $(wc -l < "$OUT/missing.txt") callees"
