#include "exhaustive_evaluation.h"
#include "lop.h"
#include "timer.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
using namespace std;

#define STR_SIZE 1024



int main(int argc, char** argv) {
	//filename of the instance
	char instance[STR_SIZE];
	bool verbose = false;
	//read command line arguments
	if (argc<2) {
		cerr << "USAGE: ./exhaustive_evaluation INSTANCE_FILE [-v]" << endl;
		cerr << "       The output will be appended to results/isic.csv" << endl;
		return EXIT_FAILURE;
	}
	strcpy(instance,argv[1]);
	if (argc>2 && strcmp(argv[2],"-v")==0)
		verbose = true;
	//load instance H and normalize it
	int *H=0, n, opt;
	lop_readInstance(H, n, opt, instance);
	opt = lop_normalize(H, n, opt);
	//lop_printInstance(H,n);//debug
	//some printing
	cerr << "* Instance " << instance << " loaded. Exhaustive search running ..." << endl;
	//start timer
	setTimer();
	//run the exhaustive search
	vector<int*> optima;
	int fopt = exhaustive_evaluation(H,n,optima,verbose);
	//stop timer
	unsigned long ptime = getTimer();
	//print true output, i.e. fitness optimum, than all local optima
	/*
	cout << fopt << endl;
	for (int i=0; i<(int)optima.size(); i++) {
		for (int j=0; j<n; j++)
			cout << optima[i][j] << ( j==n-1 ? "\n" : "," );
	}*/
	string filename = "results/isic_exp1.csv";
	ofstream file;
	if (filesystem::exists(filename))
		file.open(filename, ios::app);
	else {
		file.open(filename, ios::out);
		file << "instance;fit;opts" << endl;
	}
	file << instance << ";" << fopt << ";";
	for (int i=0; i<(int)optima.size(); i++) {
		for (int j=0; j<n; j++) {
			file << optima[i][j];
			if (j<n-1)
				file << " ";
			else if (j==n-1 && i<(int)optima.size()-1)
				file << ",";
		}
	}
	file << endl;
	file.close();
	//some printing
	cerr << "* Execution terminated in " << (ptime/1000.) << " seconds." << endl;
	//cerr << "* The true output is in COUT, while these info messages with asterisks are in CERR." << endl;
	//cerr << "* First row of true output is the optimum fitness, then there is one row for each optima." << endl;
	//exit
	return EXIT_SUCCESS;
}
