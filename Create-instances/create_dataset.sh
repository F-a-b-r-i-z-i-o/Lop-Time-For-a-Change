#!/usr/bin/env bash
set -euo pipefail

for s in agg_cat.py agg_code.py agg_regions.py sub_regions.py; do
  echo "== Create $s =="
  python "$s"
done
