#!/bin/bash
# Lab 06 - Part VI, write-up 6.3: produce an Xcode .gputrace of one probe frame.
#
# probe.cpp's main() takes no argv, so `make probe ARGS=--capture` is inert;
# the capture has to be armed from outside with MoltenVK's environment variables.
# A full run would capture 13 configs x 26 frames (~585,000 draw calls), which
# Xcode cannot open, so this temporarily drops the frame counts to 1 and puts
# them back afterwards.
set -u
cd "$(dirname "$0")"
ROOT="$(pwd)"
UTILS="$ROOT/src/utils/utils.h"
TRACE="$ROOT/report/probe.gputrace"

restore () {
  [ -f "$UTILS.orig" ] && mv -f "$UTILS.orig" "$UTILS"
  echo "restored src/utils/utils.h (5 warmup / 21 measured)"
  cmake --build build --target probe >/dev/null 2>&1
}
trap restore EXIT

cp "$UTILS" "$UTILS.orig"
sed -i '' 's/WARMUP_FRAMES   = 5/WARMUP_FRAMES   = 0/; s/MEASURED_FRAMES = 21/MEASURED_FRAMES = 1/' "$UTILS"
grep -n FRAMES "$UTILS"

echo "=== rebuilding probe with 1 frame per config ==="
cmake --build build --target probe || exit 1

rm -rf "$TRACE"
export MTL_CAPTURE_ENABLED=1
export METAL_CAPTURE_ENABLED=1
export MVK_CONFIG_AUTO_GPU_CAPTURE_SCOPE=1          # whole VkDevice lifetime
export MVK_CONFIG_AUTO_GPU_CAPTURE_OUTPUT_FILE="$TRACE"

echo
echo "=== running probe under Metal capture ==="
echo ">>> A window will open at the end. CLOSE IT (red button or Cmd-W)."
echo ">>> The trace is only written when the window closes and the device is destroyed."
echo
"$ROOT/build/probe"

echo
if [ -d "$TRACE" ]; then
  echo "trace written: report/probe.gputrace  ($(du -sh "$TRACE" | cut -f1))"
  echo "opening it in Xcode..."
  open -a Xcode "$TRACE"
else
  echo "NO TRACE WRITTEN - check that Xcode is installed and the SDK's MoltenVK is recent"
fi
echo "CAPTUREDONE"
