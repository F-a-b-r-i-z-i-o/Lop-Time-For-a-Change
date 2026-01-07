#!/usr/bin/env bash
set -euo pipefail

for s in construct_dataset_aggregate_category.py construct_dataset_aggregate_code.py construct_dataset_aggregate_regions construct_dataset_regions; do
  echo "== Create $s =="
  python "$s"
done
