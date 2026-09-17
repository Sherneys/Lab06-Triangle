#!/bin/bash
# Lab 06 - build everything, run each part, capture its panel.
# Each binary opens a window AFTER it has finished measuring and printed,
# so we let it print, then kill it. No clicking needed.
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/report"
cd "$ROOT"

echo "=== configuring + building ==="
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release > "$OUT/out-build.txt" 2>&1
cmake --build build >> "$OUT/out-build.txt" 2>&1
BUILD_RC=$?
echo "build exit code $BUILD_RC (log: report/out-build.txt)"
if [ $BUILD_RC -ne 0 ]; then
  echo "BUILD FAILED - stopping"; tail -30 "$OUT/out-build.txt"; echo "ALLDONE"; exit 1
fi

run () {
  name="$1"; secs="$2"
  printf '=== %-9s (up to %ss) ... ' "$name" "$secs"
  # script(1) gives a pty so the C++ streams stay line buffered
  script -q "$OUT/out-$name.txt" "$ROOT/build/$name" >/dev/null 2>&1 &
  spid=$!
  sleep "$secs"
  kill "$spid" 2>/dev/null
  pkill -f "$ROOT/build/$name" 2>/dev/null
  wait 2>/dev/null
  echo "captured $(wc -l < "$OUT/out-$name.txt" | tr -d ' ') lines"
}

run info      15
run triangle  25
run index     25
run earlyz    70
run stripes  100
run cost     150
run probe    150

echo
echo "=== all panels ==="
for f in info triangle index earlyz stripes cost probe; do
  echo "---------- $f ----------"
  cat "$OUT/out-$f.txt"
done
echo "ALLDONE"
