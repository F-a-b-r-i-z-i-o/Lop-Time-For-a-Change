#include "Archive.h"
#include <cstring> //memcpy
#include <algorithm> //sort, lexicographical_compare, shuffle
#include <fstream>
#include <sstream>
#include <ctime>
#include <iostream>
using namespace std;



//BEGIN HELP FUNCTION
bool are_these_permutations_equal(int* x, int* y, int n) {
	for (int i=0; i<n; i++)
		if (x[i]!=y[i])
			return false;
	return true;
}

int kendall_tau(int* x, int* y, int n) {
	//number of disagreements between x and y
	int pos_in_y[n];
	for (int i=0; i<n; i++)
		pos_in_y[y[i]] = i;
	int tau = 0;
	for (int i=0; i<n-1; i++)
		for (int j=i+1; j<n; j++)
			//siccome i<j, allora x[i] precede x[j] in x, quindi
			//c'è un disagreement se e solo se pos_in_y[x[i]] > pos_in_y[x[j]]
			if (pos_in_y[x[i]] > pos_in_y[x[j]])
				tau++;
	return tau;
}
//END HELP FUNCTION



Archive::Archive(int m_, int n_, int seed_): m(m_), n(n_), seed(seed_), rng(seed_) {
	//initialize full memory
	this->sol = new int*[m+1]; //m+1 soluzioni al massimo
	for (int i=0; i<m+1; i++)
		this->sol[i] = new int[n]; //permutazioni lunghe n
	this->fit = new unsigned long[m+1]; //m+1 fitness al massimo
	this->dmat = new int*[m+1]; //unordered distance matrix per tutte le coppie, quindi (m+1)by(m+1) al massimo
	for (int i=0; i<m+1; i++)
		this->dmat[i] = new int[m+1];
	this->fdp = new int*[m+1]; //distance profiles (ordered distance matrix) ... stessa massima dimensione della distance matrix, perche' fitness la metto al posto della distanza con se stessa
	for (int i=0; i<m+1; i++)
		this->fdp[i] = new int[m+1];
	this->rnd_perm = new int[m+1]; //random permutation used for tie-breaks
	//set empty at beginning
	this->size = 0;
	//set start time
	this->start_time = clock();
	//initialize random permutation to identity
	for (int i=0; i<m+1; i++)
		rnd_perm[i] = i;
	//init number of local optima
	n_local_optima = 0;
	//done
}



Archive::~Archive() {
	//destroy memory
	for (int i=0; i<m+1; i++) {
		delete[] sol[i];
		delete[] dmat[i];
		delete[] fdp[i];
	}
	delete[] sol;
	delete[] fit;
	delete[] dmat;
	delete[] fdp;
	delete[] rnd_perm;
	//done
}



void Archive::update(int* x, unsigned long fx) {
	//x,fx is the new solution,fitness candidate to enter the archive
	//(0) count number of local optima considered by update
	n_local_optima++;
	//(1) if x already exists in the archive, take no action
	for (int i=0; i<size; i++)
		if (are_these_permutations_equal(x, sol[i], n))
			return;
	//(2) add x,fx in the last position of the archive (size)
	memcpy(sol[size], x, n*sizeof(int));
	fit[size] = fx;
	//(3) calculate distances of existing solutions wrt x
	if (size==0) {
		//this is first solution arrived, so simply add one 0-entry to the 1by1 distance matrix
		dmat[0][0] = 0;
	} else {
		//some solutions already in the archive, so add one row and one column
		for (int i=0; i<size; i++)
			dmat[size][i] = dmat[i][size] = kendall_tau(x, sol[i], n);
		dmat[size][size] = 0;
	}
	//(4) increase archive size by 1
	size++;
	//(5) if archive size does not exceed m, calculate fworst and take no further action
	if (size<=m)
		return;
	//(6) calculate fitness-distance profiles
	//    (6.1) fdp rows are the ordered version (ascending) of dmat rows
	for (int i=0; i<size; i++) { //size is m+1 here for sure!!!
		memcpy(fdp[i], dmat[i], size*sizeof(int));
		sort(fdp[i], fdp[i]+size);
	}
	//    (6.2) since fdp[*][0] is always 0 (distance to itself), replace it with fitness
	for (int i=0; i<size; i++) //size is m+1 here for sure!!!
		fdp[i][0] = fit[i];
	//(7) find lexicographical minimum of fdp rows (using random scan for random tie-breaking), its index is the solution to remove
	shuffle(rnd_perm, rnd_perm+size, rng); //size is m+1 for sure here!
	int worst_idx = rnd_perm[0];
	for (int i=1; i<size; i++) {
		int ii = rnd_perm[i];
		if (lexicographical_compare(fdp[ii],fdp[ii]+size, fdp[worst_idx],fdp[worst_idx]+size)) //true if 1st (i) is less than 2nd (worst_idx)
			worst_idx = ii;
	}
	//(8) remove solution worst_idx by replacing it with the last added one (in position size==m+1)
	if (worst_idx!=m) { //to speedup a bit, replacing worst_idx with last solution data, does not make sense if worst_idx is already the last solution
		//copy solution and fitness
		//memcpy(sol[worst_idx], sol[m], n*sizeof(int));
		int* tmp = sol[worst_idx]; 
		sol[worst_idx] = sol[m]; 
		sol[m] = tmp;
		fit[worst_idx] = fit[m];
		//update distance matrix: last row goes in worst row, last column goes in worst column
		//memcpy(dmat[worst_idx], dmat[m], (m+1)*sizeof(int));
		tmp = dmat[worst_idx];
		dmat[worst_idx] = dmat[m]; 
		dmat[m] = tmp;
		for (int i=0; i<m+1; i++)
			dmat[i][worst_idx] = dmat[i][m];
	}
	//(9) decrease size by 1
	size--; //now is m for sure!
	//done
}



void Archive::print(string filename, string algname, string instance, unsigned long nevals) {

	clock_t end_time = clock();
	unsigned long millis = (unsigned long)(1000. * double(end_time - start_time) / CLOCKS_PER_SEC);


	//create index array
	int indices[m];
	for (int i=0; i<m; i++)
		indices[i] = i;

	//selection sort: find max and put in position i
	for (int i=0; i<m-1; i++) {
		int max_idx = i;
		for (int j=i+1; j<m; j++) {
			if (fit[indices[j]] > fit[indices[max_idx]]) {
				max_idx = j;
			}
		}
		//swap
		int temp = indices[i];
		indices[i] = indices[max_idx];
		indices[max_idx] = temp;
	}

	//build string for the solution set (ordered by fitness)
	ostringstream oss;
	for (int i=0; i<m; i++) {
		int idx = indices[i];
		for (int j=0; j<n; j++) {
			oss << sol[idx][j];
			if (j < n-1)
				oss << " ";
		}
		if (i<m-1)
			oss << ",";
	}
	string sol_set = oss.str();

	//build string for the fitness set (ordered) ;
	oss.str("");
	oss.clear();
	for (int i=0; i<m; i++) {
		oss << fit[indices[i]];
		if (i < m-1)
			oss << ",";
	}
	string fit_set = oss.str();

	ofstream f(filename, ios::app);

	f << "seed;algname;m;instance;n;nevals;millis;sol_set;fit_set;local_optima\n";

	f << seed << ";"
	  << algname << ";"
	  << m << ";"
	  << instance << ";"
	  << n << ";"
	  << nevals << ";"
	  << millis << ";"
	  << sol_set << ";"
	  << fit_set << ";"
	  << n_local_optima << "\n";
	  
}

