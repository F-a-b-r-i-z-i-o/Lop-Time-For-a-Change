#!/bin/bash

rm CDRVNS MAEDM exhaustive_evaluation

cd algorithms/MS_CDRVNS
make clean

cd ../MS_MAEDM
make clean

cd ../exhaustive_evaluation
make clean

cd ../..
pwd

