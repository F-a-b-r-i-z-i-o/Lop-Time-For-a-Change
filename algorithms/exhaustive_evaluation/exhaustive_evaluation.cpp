//Recursive implementation of this algorithm https://en.wikipedia.org/wiki/Steinhaus%E2%80%93Johnson%E2%80%93Trotter_algorithm
//Additionally, it calculates LOP fitness for all solutions of a given instance
//Translation of the Java program at https://introcs.cs.princeton.edu/java/23recursion/JohnsonTrotter.java.html

#include "exhaustive_evaluation.h"
#include "lop.h"
#include <cstdio>
#include <cstring>



//global variables
int global_fit = -1;
bool global_verbose = false;

//utility functions
void my_vector_empty(vector<int*>& optima) {
	for (int i=0; i<(int)optima.size(); i++)
		delete[] optima[i];
	optima.clear();
}

void my_vector_add(vector<int*>& optima, int* p, int nn) {
	int *pp = new int[nn];
	memcpy(pp,p,sizeof(int)*nn);
	optima.push_back(pp);
}


//QUESTA FUNZIONE E' PRIVATE
void rec_trotter(int n, int* p, int* pi, int* dir, int* H, int nn, vector<int*>& optima, int& fopt, int swapped_index1=-1) {
	// base case - print out permutation
	if (n>=nn) {
		if (global_verbose) { //debug
			printf("(%d %d)  ", swapped_index1, swapped_index1+1);
			for (int i = 0; i < nn; i++)
				printf("%d",p[i]);
		}
		int fp;
		if (swapped_index1<0) //e' la prima valutazione
			fp = lop_eval(p, nn, H);
		else
			fp = global_fit + H[p[swapped_index1]*nn+p[swapped_index1+1]] - H[p[swapped_index1+1]*nn+p[swapped_index1]];
		if (fp>fopt) {
			my_vector_empty(optima);
			my_vector_add(optima,p,nn);
			fopt = fp;
		} else if (fp==fopt) {
			my_vector_add(optima,p,nn);
		}
		global_fit = fp;
		if (global_verbose) { //debug
			printf("  %d", fp);
			//if (fp!=lop_eval(p,nn,H)) printf("  PROBLEMA!!! calcolata=%d vera=%d  ",fp,lop_eval(p,nn,H));//DEBUG!
			printf("\n");
		}
		return;
	}
	//recursive call
	rec_trotter(n+1, p, pi, dir, H, nn, optima, fopt, swapped_index1);
	//loop
	for (int i=0; i<=n-1; i++) {
		// swap
		//printf("   (%d %d)\n", pi[n], pi[n] + dir[n]);
		swapped_index1 = dir[n]>0 ? pi[n] : pi[n] + dir[n];
		int z = p[pi[n] + dir[n]];
		p[pi[n]] = z;
		p[pi[n] + dir[n]] = n;
		pi[z] = pi[n];
		pi[n] = pi[n] + dir[n];
		//recursive call
		rec_trotter(n+1, p, pi, dir, H, nn, optima, fopt, swapped_index1);
	}
	//change direction
	dir[n] = -dir[n];
}


//QUESTA E' LA FUNZIONE DA CHIAMARE ALL'ESTERNO!!!
//MEGLIO PASSARE optima vuoto quando si chiama
int exhaustive_evaluation(int* H, int nn, vector<int*>& optima, bool verbose) {
	my_vector_empty(optima); //giusto per sicurezza
	global_verbose = verbose;
	int p[nn]; //permutation
	int pi[nn]; // inverse permutation
	int dir[nn]; // direction = +1 or -1
	for (int i=0; i<nn; i++) {
		dir[i] = -1;
		p[i]  = i;
		pi[i] = i;
	}
	int fopt = -1;
	rec_trotter(0, p, pi, dir, H, nn, optima, fopt);
	return fopt;
}
