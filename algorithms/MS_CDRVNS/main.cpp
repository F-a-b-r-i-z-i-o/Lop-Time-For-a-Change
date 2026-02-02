//
//  main.cpp
//  CDRVNS
//
//  Created by Josu Ceberio Uribe on 09/01/2019 and modified by Santucci & Fagiolo on 22/01/2026.
//

#include "ParameterSetting.h"
#include "LOP.h"
#include <iostream>
#include <sys/time.h>
#include "PSolution.h"
#include "random.h"
#include <math.h>
#include "Tools.hpp"
#include <stdio.h>
#include <stdlib.h>
#include "Archive.h"


void Compile_Precedences (long** memory, int n, int* solution) {
	int i,j;
	for (i=0; i<n-1; i++)
		for (j=i+1; j<n; j++)
			memory[solution[i]][solution[j]]++;
}



bool term(int n_local_optima, double time_elapsed, int nevals, int MAX_LOCAL_OPTIMA, long long int max_evals) {
	//if neither time nor evaluations factor is passed, terminate after MAX_LOCAL_OPTIMA
	//if time_execution passed, terminates after time_execution seconds are passed
	//if nevals_factor passed, terminates after nevals_factor*(n^2) evaluations have been performed
	if (time_execution==0 && nevals_factor==0 && n_local_optima < MAX_LOCAL_OPTIMA)
		return false;
	else if (time_execution>0 && time_elapsed < time_execution)
		return false;
	else if (nevals_factor>0 && nevals < max_evals)
		return false;
	return true;
}



int main(int argc, char* argv[]) {
	//Get parameters from command line
	if ( !GetParameters(argc, argv) )
		return -1;
	//Read LOP instance
	LOP* lop = new LOP();
	int n = lop->Read(INSTANCE_FILENAME);
	//Variables, modification and initialization for MultiSolution version
	if (nevals_factor==0)
		lop->m_max_evaluations = LLONG_MAX; //to deactivate previous stopping criterion
	else
		lop->m_max_evaluations = nevals_factor*n*n; //nevals_factor*(n^2)
	int MAX_LOCAL_OPTIMA = 100*n; //new stopping criterion based on number of visited local optima (not necessarily different)
	double time_elapsed; //required by the time-based stopping criterion
	Archive archive(archive_m, n, SEED);
	//Init random number generator
	initRand(SEED);
	//Create precedences memory (for construction/destruction)
	int i;
	int compiled_solutions = 0;
	long** memory = new long*[n];
	for (i=0; i<n; i++) {
		memory[i] = new long[n];
		fill_n(memory[i], lop->m_problem_size, 0);
	}
	int* arrayLOP = new int[n*n];
	for (int i=0; i<n*n; i++)
		arrayLOP[i] = -1;
	lop->GetMatrixAsArray(arrayLOP);
	PSolution::setLOPMatrix(arrayLOP, n);
	PSolution psol(n);
	psol.makeEmpty();
	//Calculate restricted matrix
	lop->CalculateSparsityMatrix();
	//Declare variables for optimization
	bool improvement = false;
	int* solution = new int[n];
	int* best_solution = new int[n];
	long int fitness = 0;
	long int best_fitness = 0;
	long int fit = 0;
	float factor = 0;
	//Construct initial solution with construction heuristic
	int* prev_solution = new int[n];
	Q = urand()*0.1 + 0.9; //according to paper it should be set in the range [0.9,1].
	psol.construct(Q);
	psol.toPermutation(solution);
	fitness = psol.fit;
	lop->m_evaluations++;
	memcpy(best_solution, solution, sizeof(int)*n);
	best_fitness = fitness;
	Compile_Precedences(memory, n, solution);
	compiled_solutions++;
	//Main loop
	do {
		//Run VNS
		improvement = false;
		do {
			//Efficient local search on the insert neighbourhood
			fitness = GreedyLocalSearch_Insert(solution, fitness, lop);
			//One Greedy step in the interchange neighbourhood
			fit = Best_IntechangeStep_Efficient(solution, fitness, lop);
			//Update fitness if the case
			if (fit>fitness) {
				fitness = fit;
				improvement = true;
			}
		} while (improvement && lop->m_evaluations < lop->m_max_evaluations);
		//We have a local optimum, so update the archive for multisolutions version of LOP
		archive.update(solution, fitness);
		//Compile the precedences for VNS<->Constr/Destr
		Compile_Precedences(memory, n, solution);
		compiled_solutions++;
		//Update best solution if it is the case.
		if (fitness>best_fitness) {
			memcpy(best_solution, solution, sizeof(int)*n);
			best_fitness = fitness;
		} else {
			memcpy(solution, best_solution, sizeof(int)*n);
		}
		//Run destruction-construction if evaluations not exhausted (but deactivated for multisolutions version)
		if (lop->m_evaluations < lop->m_max_evaluations) {
			//Destruction-Construction procedure on 'solution' vector
			memcpy(prev_solution, solution, sizeof(int)*n);
			psol.fromPermutation(solution);
			do {
				Q = urand()*0.1 + 0.9; //according to paper it should be set in the range [0.9,1].
				//factor = ((float)lop->m_evaluations)/((float)lop->m_max_evaluations); //original code
				//percentage of execution calculated based on the three different stopping criterion for new experiments
				if (time_execution==0 && nevals_factor==0) //max number of local optima as stopping criterion
					factor = archive.n_local_optima / (double)MAX_LOCAL_OPTIMA;
				else if (time_execution>0) { //max execution time as stopping criterion
					time_elapsed = double(clock() - archive.start_time) / CLOCKS_PER_SEC;
					factor = time_elapsed / (double)time_execution;
				} else if (nevals_factor>0) //max number of evaluations as stopping criterion
					factor = lop->m_evaluations / (double)lop->m_max_evaluations;
				if (factor>1.)
					factor = 1.;
				R = 1 - factor*0.9;
				//destruct-construct and then convert back to permutation representation
				psol.destruct_sorted(memory,R);
				psol.construct(Q);
				psol.toPermutation(solution);
				//stop to loop current solution is equal to previous one
			} while( memcmp(prev_solution, solution, sizeof(int)*n)==0 );
			//update number of evaluations and best solution if the case
			fitness = psol.eval();
			lop->m_evaluations++;
			if (fitness > best_fitness) {
				memcpy(best_solution, solution, sizeof(int)*n);
				best_fitness = fitness;
			}
		}
		time_elapsed = double(clock() - archive.start_time) / CLOCKS_PER_SEC;
		//cout << lop->m_evaluations << " " << archive.n_local_optima << " " << factor << "\n"; //debug
	//} while (lop->m_evaluations < lop->m_max_evaluations); //original code
	} while ( !term(archive.n_local_optima, time_elapsed, lop->m_evaluations, MAX_LOCAL_OPTIMA, lop->m_max_evaluations) );
	//Print results to file
	archive.print(
		RESULTS_FILENAME,
		"MS-CDRVNS",
		INSTANCE_FILENAME,
		lop->m_evaluations
	);
	//Free memory (archive destructor is called automatically)
	delete lop;
	delete[] solution;
	delete[] best_solution;
	for (int i=0; i<n; i++)
		delete[] memory[i];
	delete[] memory;
	delete[] prev_solution;
	delete[] arrayLOP;
	//Done
	return 0;
}
