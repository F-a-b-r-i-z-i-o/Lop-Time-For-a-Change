#!/bin/bash

ubiquity_neighborhood='F';
	for problem in lop # lop fsp qap # tsp
	do
		for file in `ls /home/jceberio001/Instances/LOP/xLOLIB*/*`
		  do
			echo "qsub ILS.sh $problem $file $ubiquity_neighborhood";
			qsub ILS.sh $problem $file $ubiquity_neighborhood
		done
	done
