/*******************************************************************************************
Authors: Carlos Segura (Programmer and Designer), Lazaro Lugo (Programmer and Designer),
Gara Miranda (Designer)
Description: main function to execute MA-EDM to an instance.
Parameters:
- Number of individuals in the population
- Crossover: it can be cx or ob
- Stopping criterion in time (optional, if not provided uses 1000*N*N evaluations)
- Seed
- Instance file
- Output file
- m (archive size)
********************************************************************************************/
#include "MA.h"
#include <unistd.h>
#include <bits/stdc++.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include "Archive.h"

Problem *Individual::problem;

int main(int argc, char **argv){
	if(argc != 7 && argc != 8){
		cout << "Error. Usage: " << argv[0] << " N crossover [stopping_criterion] seed instance_file output_file m" << endl;
		cout << "  N: Population size" << endl;
		cout << "  crossover: cx or ob" << endl;
		cout << "  stopping_criterion: time in seconds (optional, if omitted uses 1000*N*N evaluations)" << endl;
		cout << "  seed: random seed" << endl;
		cout << "  instance_file: path to instance" << endl;
		cout << "  output_file: path to output" << endl;
		cout << "  m: archive size" << endl;
		exit(0);
	}
	
	int N = atoi(argv[1]);
	double pc = 1.0;
	string crossType = argv[2];
	double finalTime;
	int seed;
	string instanceFile;
	string outputFile;
	int m;
	
	// Check if finalTime is provided or not
	if (argc == 8) {
		// finalTime is provided
		finalTime = atof(argv[3]);
		seed = atoi(argv[4]);
		instanceFile = argv[5];
		outputFile = argv[6];
		m = atoi(argv[7]);
	} else {
		finalTime = -1.0; 
		seed = atoi(argv[3]);
		instanceFile = argv[4];
		outputFile = argv[5];
		m = atoi(argv[6]);
	}
	
	srand(seed);
	MA ma(N, pc, crossType, finalTime, outputFile, instanceFile, seed, m);
	Problem p(instanceFile);
	Individual::problem = &p;
	ma.run();
	
	/*
		Examples for run:
		With time criterion:
			./main 200 cx 1 1 ../../Dataset/pxp/pxp_n_170/pxp_US_2022_n170 output.csv 5
		With evaluation criterion (1000*N*N):
			./main 200 cx 1 ../../Dataset/pxp/pxp_n_170/pxp_US_2022_n170 output.csv 5
	*/
}