#!/bin/bash
cd "$(dirname "$0")"
bash report/runall.sh 2>&1 | tee report/out-ALL.txt
echo
echo "finished - you can close this window"
