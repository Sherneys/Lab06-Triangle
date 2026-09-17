#!/bin/bash
# Re-display the recorded panels (report/out-*.txt, written by script(1) during the
# real runs) in a Terminal window and photograph each screenful.
cd "$(dirname "$0")"
OUT=report
printf '\e[8;40;104t'   # 40 rows x 104 cols
sleep 1

show () {   # show <outfile.png> <header> <file>...
  png="$1"; shift
  hdr="$1"; shift
  printf '\e[3J\e[H\e[2J'
  echo "  $hdr"
  echo "  Apple M1 Pro  ·  recorded by script(1) during the run  ·  $(date '+%d %b %Y')"
  echo
  for f in "$@"; do
    echo "  \$ make ${f}"
    # strip the ^D that script(1) writes at EOF
    sed $'s/\x04//g' "$OUT/out-$f.txt"
    echo
  done
  sleep 4
  screencapture -x -o "$OUT/$png"
  sleep 1
}

show panels-1.png "Lab 06 — Part 0 and Part I"          info triangle
show panels-2.png "Lab 06 — Part II and Part III"       cost stripes
show panels-3.png "Lab 06 — Part IV and Part V"         earlyz index
show panels-4.png "Lab 06 — Part VI"                    probe

printf '\e[3J\e[H\e[2J'
ls -l "$OUT"/panels-*.png
echo PANELSDONE
