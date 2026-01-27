/*******************************************************************************************
Authors: Carlos Segura (Programmer and Designer), Lazaro Lugo (Programmer and Designer),
Gara Miranda (Designer)

Description: class for the Memetic Algorithm with Explicit Diversity Management (MA-EDM)
published in the paper "A Diversity-aware Memetic Algorithm for the Linear Ordering Problem"
********************************************************************************************/

#ifndef __MA_H__
#define __MA_H__

#include "Individual.h"


class MA {

	public:
		MA(int N_, double pc_,string &crossType_, double finalTime_, int nevals_factor_, string &outputFile, string &instanceFile, int seed, int m); //modified to take into account max time and max number of evaluations
		void run();

	private:
		//Parameters of MA
		int N;				//Population Size
		double pc;			//crossover probability
		string crossType;	//crossover type
		double finalTime;	//Seconds, but if 0 time is not the termination criterion
		int nevals_factor;	//(n^2)*nevals_factor is the maximum number of evaluations, but if 0 evaluation number is not the termination criterion
		string outputFile;
		string instanceFile; 
		int seed;
		int m;

		//Basic procedures of MA
		//void initPopulation();
		unsigned long initPopulation(); //modified to return the number of evaluations
		void initDI();
		void selectParents();
		void crossover();
		//void intensify();
		unsigned long intensify(); //modified to return the number of evaluations
		void replacement();

		//Internal attributes of MA
		vector< Individual * > population; 
		vector< Individual * > parents;
		vector< Individual * > offspring;
		double initialTime;
		double DI;
		int generation;
		double elapsedTime;

};

#endif
