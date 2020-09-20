#!/bin/bash
tmpfile="$(mktemp)"
"$(dirname "$0")/read_pcb.py" "$1" > "$tmpfile"
gist -p -f "$1.md" -d "Generated automatically from $1" <("$(dirname "$0")/to_gist.awk" < "$tmpfile") -f "$1.csv" <("$(dirname "$0")/to_csv.awk" < "$tmpfile")
rm "$tmpfile"
