#!/bin/bash

cd algorithms/MS_CDRVNS
pwd
make clean; make

cd ../MS_MAEDM
pwd
make clean; make

cd ../exhaustive_evaluation
pwd
make clean; make

cd ../..
pwd
cp algorithms/MS_CDRVNS/CDRVNS .
cp algorithms/MS_MAEDM/MAEDM .
cp algorithms/exhaustive_evaluation/exhaustive_evaluation .

cd algorithms/MS_CDRVNS
pwd
make clean
cd ../MS_MAEDM
pwd
make clean
cd ../exhaustive_evaluation
pwd
make clean
cd ../..
pwd

