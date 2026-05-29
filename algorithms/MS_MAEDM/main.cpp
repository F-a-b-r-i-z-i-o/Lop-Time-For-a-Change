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


void usage(char *progname) {
	printf("Usage: MAEDM -i <instance_name>  -o <results_filename> -s <seed> -m <archive_size> [-t <max_time>] [-e <nevals_factor>]\n");
	printf("   -i File name of the instance.\n");
	printf("   -o Name of the file to store the results.\n");
	printf("   -s Seed used for pseudo-random numbers generator.\n");
	printf("   -m Archive size.\n");
	printf("   -t Seconds of execution.\n");
	printf("   -e Factor to multiply n^2 for number of evaluations.\n");
	printf("Termination criterion:\n");
	printf("   * If neither -t nor -e are passed, the termination criterion is a hard-coded number of visited local optima 100*sizeInstance (not necessarily distinct).\n");
	printf("   * If only -t is passed, the execution terminates after the given number of seconds.\n");
	printf("   * If only -e is passed, the execution terminates after given_factor*(n^2) evaluations have been performed.\n");
	printf("   * Unpredictable behaviour if both -t and -e are passed.\n");
}


Problem *Individual::problem;


int main(int argc, char **argv) {
	//print usage
	if (argc == 1) {
		usage(argv[0]);
		return 1;
	}
	//defaults paper parameters 
	int N = 200;
	double pc = 1.0;
	string crossType = "cx";
	//command line arguments
	string instanceFile, outputFile;
	int m = -1;
	int finalTime = 0;
	int nevals_factor = 0;
	int seed = 0; 
	int opt;
	while ((opt = getopt(argc, argv, "i:o:m:t:e:s:N:c:p:h")) != -1) {
		switch (opt) {
			case 'i': instanceFile  = optarg; break;
			case 'o': outputFile    = optarg; break;
			case 'm': m             = atoi(optarg); break;
			case 't': finalTime     = atoi(optarg); break;
			case 'e': nevals_factor = atoi(optarg); break;
			case 's': seed          = atoi(optarg); break;
			case 'n': N             = atoi(optarg); break;
			case 'c': crossType     = optarg; break;
			case 'p': pc            = atof(optarg); break;
			default:  printf("INVALID ARGUMENTS"); return 1;
		}
	}
	//initialize random number generator
	srand(seed);
	//run the algorithm
	MA ma(N, pc, crossType, finalTime, nevals_factor, outputFile, instanceFile, seed, m);
	Problem p(instanceFile);
	Individual::problem = &p;
	ma.run();
	//done
}
