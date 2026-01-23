/*******************************************************************************************
Authors: Carlos Segura (Programmer and Designer), Lazaro Lugo (Programmer and Designer),
Gara Miranda (Designer)

Description: main function to execute MA-EDM to an instance.
Parameters:
- Number of individuals in the population
- Crossover: it can be cx or ob
- Stopping criterion in time
- Seed
- Instance file
- Output file
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
	// if(argc!=8){
	// 	cout << "Error. Usage: " << argv[0] << " N crossover stopping_criterion seed instance_file output_file" << endl; 
	// 	exit(0);
	// }
	// int N = atoi(argv[1]);
	// double pc = 1.0;
	// string crossType = argv[2];
	// double finalTime = atof(argv[3]);
	// int seed = atoi(argv[4]);
	// string instanceFile = argv[5];
	// string outputFile = argv[6];
	// int m = atoi(argv[7]);
	
	// defaults paper parameters 
    int N = 200;
    double pc = 1.0;
    string crossType = "cx";

    // required
    string instanceFile, outputFile;
    int m = -1;
    int finalTime = 0;
    int seed = 0; 


	int opt;
    while ((opt = getopt(argc, argv, "i:o:m:t:s:N:c:p:h")) != -1) {
        switch (opt) {
            case 'i': instanceFile = optarg; break;
            case 'o': outputFile   = optarg; break;
            case 'm': m           = atoi(optarg); break;
            case 't': finalTime   = atoi(optarg); break;
            case 's': seed        = atoi(optarg); break;
            case 'n': N           = atoi(optarg); break;
            case 'c': crossType   = optarg; break;
            case 'p': pc          = atof(optarg); break;
        }
    }

	// //PARAMETRI POPOLAZIONE E CROSSOVER COME NEL PAPER
	// N = 200;
	// crossType = "cx";
	
	

	//srand(seed);
	MA ma(N, pc, crossType, finalTime, outputFile, instanceFile, seed, m);
	Problem p(instanceFile);
	Individual::problem = &p;
	ma.run();
	/*
		example for run:
			./main 200 cx 1 1 ../../Dataset/pxp/pxp_n_170/pxp_US_2022_n170 output.csv 5 

	*/
}
