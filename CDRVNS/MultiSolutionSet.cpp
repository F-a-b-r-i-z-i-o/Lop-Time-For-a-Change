#include "MultiSolutionSet.h"
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

MultiSolutionSet::MultiSolutionSet(int m) {
	cout << "Sono il costruttore!\n";
}

MultiSolutionSet::~MultiSolutionSet() {
	cout << "Sono il distruttore!\n";
}

void MultiSolutionSet::update_set(int* x, unsigned long fx) {
	cout << "Sono update_set!\n";
}

void MultiSolutionSet::print_result(int seed, 
									string algorithm, 
									int m, 
									string name_instance, 
									int nevals, 
									double time, 
									vector<int> solution_set, 
									vector<double> fitness)
	{
    string filename = "results_MA-EDM.csv";
    ofstream out(filename, ios::app);
    string algorithm = "MS-MA-EDM";
	
	out << "seed\talgorithm\tm\tinstance\tnevals\ttime\tsolution_set\tfitness_set\n";
    
    out << seed << '\t'
        << algorithm << '\t'
        << m << '\t'
        << nevals << '\t'
        << time <<'\t';

    out << endl;

}
