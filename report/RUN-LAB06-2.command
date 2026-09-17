#!/bin/bash
cd "$(dirname "$0")"
ROOT="$(pwd)"; OUT="$ROOT/report"
run () {
  printf '=== %-9s (up to %ss) ... ' "$1" "$2"
  script -q "$OUT/out-$1.txt" "$ROOT/build/$1" >/dev/null 2>&1 &
  spid=$!; sleep "$2"
  kill "$spid" 2>/dev/null; pkill -f "$ROOT/build/$1" 2>/dev/null; wait 2>/dev/null
  echo "captured $(wc -l < "$OUT/out-$1.txt" | tr -d ' ') lines"
}
mkdir -p reference
run triangle 120
run index    120
echo "=== retry results ==="
cat "$OUT/out-triangle.txt"; echo; cat "$OUT/out-index.txt"
echo "ALLDONE2"
