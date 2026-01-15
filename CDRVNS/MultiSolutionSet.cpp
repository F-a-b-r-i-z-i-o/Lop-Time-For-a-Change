#include "MultiSolutionSet.h"
#include <iostream>
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
