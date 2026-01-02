#!/usr/bin/env bash
set -euo pipefail

for s in control_diag.py control_sum.py; do
  echo "== Running $s =="
  python "$s"
done
