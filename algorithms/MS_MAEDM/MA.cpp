/*******************************************************************************************
Authors: Carlos Segura (Programmer and Designer), Lazaro Lugo (Programmer and Designer),
Gara Miranda (Designer)

Description: implementation of MA-EDM. The novelty appears in the replacement phase, where
the BNP strategy is implemented. See "A Diversity-Aware Memetic Algorithm for the Linear
Order Problem" for a complete description. There are some parts that are specific of the
LOP, such as the 
********************************************************************************************/

#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <string>

#include "MA.h"
#include "utils.h"
#include "Archive.h"

using namespace std;



MA::MA(int N_, double pc_, string &crossType_, double finalTime_, int nevals_factor_, string &outputFile_, string &instanceFile_, int seed_, int m_) {
	//modified to take into account max time and max number of evaluations
	N = N_;
	pc = pc_;
	crossType= crossType_;
	finalTime = finalTime_;
	nevals_factor = nevals_factor_;
	outputFile = outputFile_;
	instanceFile = instanceFile_;
	seed = seed_;
	m = m_;
	struct timeval currentTime; 
	gettimeofday(&currentTime, NULL);
	initialTime = (double) (currentTime.tv_sec) + (double) (currentTime.tv_usec)/1.0e6;
}


//Initialize and apply intensification to each individual
//void MA::initPopulation(){
unsigned long MA::initPopulation() {
	//modified to return number of evaluations performed
	unsigned long _nevals = 0;
	for (int i = 0; i < N; i++){
		Individual *ei = new Individual();
		ei->initialize_heuristic();
		//ei->intensify();
		_nevals += ei->intensify();
		population.push_back(ei);
	}
	return _nevals;
}


//Select two parents with binary selection
void MA::selectParents() {
	parents.clear();
	for (int i = 0; i < population.size(); i++){
		int first = getRandomInteger0_N(N - 1);
		int second = getRandomInteger0_N(N - 1);
		if (population[first]->getCost() <= population[second]->getCost()){
			parents.push_back(population[first]);
		} else {
			parents.push_back(population[second]);
		}
	}
}


//Apply crossover: two crossover operators were implemented cx and bx
void MA::crossover() {
	for (int i = 0; i < parents.size(); i++){
		Individual *ni = new Individual();
		*ni = *parents[i];
		offspring.push_back(ni);
	}

	if(crossType=="cx"){
		for (int i = 0; i < offspring.size() - N%2; i+=2) {
			if (generateRandomDouble0_Max(1) <= pc) {
				offspring[i]->crossoverCX(*offspring[i + 1]);
			}
		}
	} else if (crossType=="ob"){
		for (int i = 0; i < offspring.size() -N%2; i+=2){
			if (generateRandomDouble0_Max(1) <= pc){
				offspring[i]->crossoverOB(*offspring[i+1]);
			}
		}
	} else{
		cout << "No se reconoce el tipo de operador de cruce" << endl;
		exit(-1);
	}
}


//void MA::intensify(){
unsigned long MA::intensify() {
	//modified to return number of evaluations performed
	unsigned long _nevals = 0;
	for (int i = 0; i < offspring.size(); i++){
		//offspring[i]->intensify();
		_nevals += offspring[i]->intensify();
	}
	return _nevals;
}


void MA::replacement() {
	vector < Individual* > all;
	
	//Join population and offspring
	for (int i = 0; i < population.size(); i++){
		all.push_back(population[i]);
		all.back()->distance = MAX_DISTANCE;
	}
	population.clear();

	for (int i = 0; i < offspring.size(); i++){
		all.push_back(offspring[i]);
		all.back()->distance = MAX_DISTANCE;
	}
	offspring.clear();
	
	//Select best solution
	int indexBest = 0;
	for (int i = 1; i < all.size(); i++){
		if (all[i]->getCost() < all[indexBest]->getCost()){
			indexBest = i;
		}
	}
	population.push_back(all[indexBest]);
	all[indexBest] = all.back();
	all.pop_back();

	struct timeval currentTime; 
	gettimeofday(&currentTime, NULL);
	double elapsedTime = (double) (currentTime.tv_sec) + (double) (currentTime.tv_usec)/1.0e6;
	elapsedTime -= initialTime;

	//Select next N - 1 solution
	double D = DI - DI * elapsedTime / finalTime;
	while(population.size() != N){
		//Update distances
		for (int i = 0; i < all.size(); i++){
			all[i]->distance = min(all[i]->distance, all[i]->getDistance(*population.back()));
		}
		//Select best option
		indexBest = 0;
		for (int i = 1; i < all.size(); i++){
			bool betterInDist =	(all[i]->distance > all[indexBest]->distance);
			bool eqInDist = (all[i]->distance == all[indexBest]->distance);
			bool betterInFit = (all[i]->getCost() < all[indexBest]->getCost());
			bool eqInFit = (all[i]->getCost() == all[indexBest]->getCost());

			if (all[indexBest]->distance < D){//Do not fulfill distance requirement
				if ((betterInDist) || (eqInDist && betterInFit)){
					indexBest = i;
				}
			} else {
				if (all[i]->distance >= D){
					if ((betterInFit) || (eqInFit && betterInDist)){
						indexBest = i;
					}
				}
			}
		}
		//Insert best option
		population.push_back(all[indexBest]);
		all[indexBest] = all.back();
		all.pop_back();
	}
	//Release memory
	for (int i = 0; i < all.size(); i++){
		delete(all[i]);
	}
}


void MA::initDI() {
	double meanDistance = 0;
	for (int i = 0; i < population.size(); i++){
		for (int j = i + 1; j < population.size(); j++){
			meanDistance += population[i]->getDistance(*population[j]);
		}
	}
	meanDistance /= (population.size() * (population.size() - 1)) / 2;
	DI = meanDistance;
}


/* BEGIN FUNCTION FOR TERMINATION CRITERIA */
bool term(int n_local_optima, double time_elapsed, unsigned long nevals, int MAX_LOCAL_OPTIMA, unsigned long max_evals, double time_execution, int nevals_factor) {
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
/* END FUNCTION FOR TERMINATION CRITERIA */


void MA::run() {
	//modified to consider the three termination criteria
	int MAX_LOCAL_OPTIMA = 20000;
	int _n = Individual::problem->problemDimension;
	unsigned long _nevals = 0; //added
	//initPopulation();
	_nevals += initPopulation(); //added
	initDI();
	Archive archive(m, population[0]->S.size(),seed);
	generation = 0;
	double cTime;
	double bestCost;
	do {
		//Iteration of the MA: selection, crossover, intensification, replacement
		selectParents();
		crossover();
		//intensify();
		_nevals += intensify(); //added
		/* BEGIN ARCHIVE UPDATE */
		for (int i = 0; i < offspring.size(); i++)
			archive.update(offspring[i]->S.data(), -offspring[i]->getCost()); //minus required because Archive business logic is for maximization
		/* END ARCHIVE UPDATE */
		replacement();
		struct timeval currentTime;
		gettimeofday(&currentTime, NULL);
		cTime = (double) (currentTime.tv_sec) + (double) (currentTime.tv_usec) / 1.0e6;
		elapsedTime = cTime - initialTime;
		if (generation == 0) {
			bestCost = population[0]->getCost();
		} else {
			if (population[0]->getCost() < bestCost) {
				bestCost = population[0]->getCost();
			}
		}
		generation++;
		//cout << _nevals << " " << archive.n_local_optima << " " << finalTime << " " << elapsedTime << endl; //debug
	} while ( !term(archive.n_local_optima, elapsedTime, _nevals, MAX_LOCAL_OPTIMA, _n*_n*(unsigned long)nevals_factor, finalTime, nevals_factor) );
	//} while (cTime - initialTime < finalTime); //original
	//print best solution
	//population[0]->print("orginal-result.txt");
	/* BEGIN ARCHIVE PRINT */
	archive.print(
		outputFile,
		"MS-MA-EDM", 
		instanceFile,
		_nevals
	);
	/* END ARCHIVE PRINT */
}
