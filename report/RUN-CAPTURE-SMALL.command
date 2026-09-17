#!/bin/bash
# A SECOND, deliberately small Metal capture of probe, so Xcode's replay
# profiler can actually finish. Same scenes and same structure as the full
# capture; only the instance count and the frame counts are reduced, and both
# are put back by the EXIT trap.
set -u
cd "$(dirname "$0")"
ROOT="$(pwd)"
UTILS="$ROOT/src/utils/utils.h"
STUD="$ROOT/src/utils/Student.h"
TRACE="$ROOT/report/probe-small.gputrace"

restore () {
  [ -f "$UTILS.orig" ] && mv -f "$UTILS.orig" "$UTILS"
  [ -f "$STUD.orig"  ] && mv -f "$STUD.orig"  "$STUD"
  echo "restored utils.h (5/21) and Student.h (INSTANCES = 5000)"
  cmake --build build --target probe >/dev/null 2>&1
  echo "rebuilt probe at full size"
}
trap restore EXIT

cp "$UTILS" "$UTILS.orig"
cp "$STUD"  "$STUD.orig"
sed -i '' 's/WARMUP_FRAMES   = 5/WARMUP_FRAMES   = 0/; s/MEASURED_FRAMES = 21/MEASURED_FRAMES = 1/' "$UTILS"
sed -i '' 's/INSTANCES = 5000/INSTANCES = 200/' "$STUD"
grep -h -n "FRAMES\|INSTANCES" "$UTILS" "$STUD"

echo "=== rebuilding probe: 1 frame per config, 200 instances (~900 draws) ==="
cmake --build build --target probe || exit 1

rm -rf "$TRACE"
export MTL_CAPTURE_ENABLED=1
export METAL_CAPTURE_ENABLED=1
export MVK_CONFIG_AUTO_GPU_CAPTURE_SCOPE=1
export MVK_CONFIG_AUTO_GPU_CAPTURE_OUTPUT_FILE="$TRACE"

echo
echo "=== running probe under Metal capture ==="
echo ">>> CLOSE THE WINDOW when it appears (red button or Cmd-W)."
echo ">>> Do NOT Force Quit - the trace is only finalised by vkDestroyDevice."
echo
"$ROOT/build/probe"

echo
if [ -d "$TRACE" ] && [ -e "$TRACE/index" ]; then
  echo "trace OK: report/probe-small.gputrace  ($(du -sh "$TRACE" | cut -f1), index present)"
  open -a Xcode "$TRACE"
else
  echo "TRACE INCOMPLETE - was the window force-quit?"
fi
echo "SMALLDONE"
