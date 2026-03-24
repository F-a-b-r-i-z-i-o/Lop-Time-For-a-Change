#!/bin/bash

cd algorithms/MS_CDRVNS
make clean
cd ../MS_MAEDM
make clean
cd ../exhaustive_evaluation
make clean
cd ../..

zip -r ms_lop_all.zip *.py *.sh requirements.txt algorithms/MS_CDRVNS algorithms/MS_MAEDM algorithms/exhaustive_evaluation exiobase real_data classic_benchmarks results graphs tables
