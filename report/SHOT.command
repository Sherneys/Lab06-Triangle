#!/bin/bash
# Save PNGs of the two Xcode GPU-capture windows into report/.
cd "$(dirname "$0")"
open -a Xcode
sleep 4
screencapture -x -o -l 6684 report/capture-summary-full.png  2>&1
screencapture -x -o -l 6856 report/capture-summary-small.png 2>&1
screencapture -x -o             report/capture-screen.png    2>&1
ls -l report/capture-*.png 2>&1
echo SHOTDONE
