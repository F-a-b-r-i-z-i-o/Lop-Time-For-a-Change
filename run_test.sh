#!/usr/bin/env bash
cd "MS_MAEDM"
pwd
PATH_INPUT="../Dataset/pxp/pxp_n_170/pxp_US_2022_n170"
PATH_OUT="test.csv"
ARCHIVE_VARIABLE="5" 
TIME_VARIABLE="10"
MAEDM_BIN="./MAEDM"
echo "Running MAEDM..."
"$MAEDM_BIN" -i "$PATH_INPUT" -o "$PATH_OUT" -m "$ARCHIVE_VARIABLE"

cd ..
pwd

cd "MS_CDRVNS"

CDRVNS_BIN="./CDRVNS"
echo "Running MSCDRVNS..."
"$CDRVNS_BIN" -i "$PATH_INPUT" -o "$PATH_OUT" -m "$ARCHIVE_VARIABLE"
