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

MA::MA(int N_, double pc_, string &crossType_, double finalTime_, string &outputFile_, string &instanceFile_, int seed_, int m_){
	N = N_;
	pc = pc_;
	crossType= crossType_;
	finalTime = finalTime_;
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
unsigned long MA::initPopulation(){//VALENTINO
	unsigned long __nevals = 0;//VALENTINO
	for (int i = 0; i < N; i++){
		Individual *ei = new Individual();
		ei->initialize_heuristic();
		//ei->intensify();
		__nevals += ei->intensify();//VALENTINO
		population.push_back(ei);
	}
	return __nevals;//VALENTINO
}

//Select two parents with binary selection
void MA::selectParents(){
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
void MA::crossover(){
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
unsigned long MA::intensify(){//VALENTINO
	unsigned long ___nevals = 0;//VALENTINO
	for (int i = 0; i < offspring.size(); i++){
		//offspring[i]->intensify();
		___nevals += offspring[i]->intensify();//VALENTINO
	}
	return ___nevals;//VALENTINO
}

void MA::replacement(){
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

void MA::initDI(){
	double meanDistance = 0;
	for (int i = 0; i < population.size(); i++){
		for (int j = i + 1; j < population.size(); j++){
			meanDistance += population[i]->getDistance(*population[j]);
		}
	}
	meanDistance /= (population.size() * (population.size() - 1)) / 2;
	DI = meanDistance;
}

void MA::run() {
	int MAX_LOCAL_OPTIMA = 20000;
	int _n = Individual::problem->problemDimension;
	//unsigned long MAX_COMPUTATIONAL_WORKING_UNITS = 1000*_n*_n;//VALENTINO
	unsigned long ___nevals = 0;//VALENTINO
	//initPopulation();
	___nevals += initPopulation();//VALENTINO
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
		___nevals += intensify();//VALENTINO
		for (int i = 0; i < offspring.size(); i++){
			//CHIAMARE QUI UPDATE SET ... offspring[i]->S offspring[i]->cost //VALENTINO
			archive.update(offspring[i]->S.data(),-offspring[i]->getCost());
		}
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
		//cout << ___nevals << " " << archive.n_local_optima << " " << finalTime << " " << elapsedTime << endl; //debug
	} while ((finalTime == 0 && archive.n_local_optima < MAX_LOCAL_OPTIMA) || (finalTime>0 && elapsedTime < finalTime));
	//} while (cTime - initialTime < finalTime);
	//print best solution
	//population[0]->print("orginal-result.txt");
	archive.print(
		outputFile,
		"MS-MA-EDM", 
		instanceFile,
		___nevals
	);
}
