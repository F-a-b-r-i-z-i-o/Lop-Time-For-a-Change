#include "MultiSolutionSet.h"
#include <iostream>
#include <utility>
#include <algorithm>

using namespace std;

#ifdef DEBUG
  #define DBG(stmt) do { stmt; } while(0)
#else
  #define DBG(stmt) do {} while(0)
#endif

MultiSolutionSet::MultiSolutionSet(int m_, int n_){
    m = m_; 
    n = n_;
    DBG(cout << "Sono il costruttore! m=" << m << " n=" << n << "\n");
}

MultiSolutionSet::~MultiSolutionSet() {
    DBG(cout << "Sono il distruttore!\n");
}

double kendellTau(int *a, int *b, int n){
    int i,j,v = 0; 
    for (j=1; j < n; j++)
        for (i=0; i < j; i++)
            v += (a[i] < a[j]) != (b[i] < b[j]);
    double denom = 0.5 * n * (n - 1);
    return v / denom;
}

// efficient Kendall Tau distance implementation

int mergesortCount(int* a, int* temp, int begin, int end) {
	//merge sort that count the inversions and sort a (the merge procedure is hardcoded here)
	//http://rupakcs.blogspot.it/2011/05/counting-inversions-in-array-using.html
	//http://www.geeksforgeeks.org/counting-inversions/
	if (begin==end)  
		return 0;
	int middle = begin + (end-begin)/2;
	int linv = mergesortCount(a,temp,begin,middle); //inversions on the left part
	int rinv = mergesortCount(a,temp,middle+1,end); //inversions on the right part
	int i = begin;
	int j = middle+1;
    int k = begin;
	int minv = 0; //inversions for the merging step
	while (i<=middle && j<=end) {
		if (a[i]<a[j]) { //I know that or a[i]<a[j] or a[i]>a[j] ... they can't be equals
			temp[k++] = a[i++];
		} else {
			minv += middle-i+1;
			temp[k++] = a[j++];
		}
	}
	int q = middle-i+1;
	memcpy(temp+k,a+i,sizeof(int)*q);
	memcpy(temp+(k+q),a+j,sizeof(int)*(end-j+1));
	memcpy(a+begin,temp+begin,sizeof(int)*(end-begin+1));
	return linv+rinv+minv;
}



int countInversions(int* a, int size) {
	//returns the number of inversions in a and sort a
	int* workspace = new int[size];
	int ninv = mergesortCount(a,workspace,0,size-1);
	delete[] workspace;
	return ninv;
}

void pinverse(int *pinv,int *p,int n) {
    for(int i=0; i<n; i++)
        pinv[p[i]]=i;
}

void pcompose(int *p3,int *p1,int *p2,int n) {
	for(int i=0;i<n;i++)
		p3[i]=p1[p2[i]];
}

void pdifference(int *diff,int *p2,int *p3,int n) {
    int *p2inv=new int[n];
	pinverse(p2inv,p2,n);
	pcompose(diff,p2inv,p3,n);  
	delete[] p2inv;
}

int kendall_tau_distance(int *p1,int *p2,int n) {
    int* p3=new int[n];
    pdifference(p3,p1,p2,n);
	int res=countInversions(p3,n);
	delete[] p3;
	return res;
}


void MultiSolutionSet::rebuild_fitness_distances() {
    size_t S = set_possible_solution.size();

    // fitness_distances[i] length S+1:
    //   fitness_distances[i][0]   = fitness of solution i
    //   fitness_distances[i][j+1] = KendallTau(i, j) for j != i
    fitness_distances.assign(S, vector<double>(S + 1, 0.0));

    // Store fitness in the first position
    for (size_t i = 0; i < S; ++i) {
        fitness_distances[i][0] = static_cast<double>(set_fx[i]);
    }

    // Fill Kendall tau distances
    for (size_t i = 0; i < S; ++i) {
        for (size_t j = i + 1; j < S; ++j) {
            double d = kendellTau(set_possible_solution[i].data(),
                                  set_possible_solution[j].data(),
                                  n);
            fitness_distances[i][j + 1] = d;
            fitness_distances[j][i + 1] = d;
        }
    }

    // Sort distance
    for (size_t i = 0; i < S; ++i) {
        sort(fitness_distances[i].begin() + 1, fitness_distances[i].end());
    }
}


void MultiSolutionSet::update_set(const int* x, unsigned long fx) {

    // Check duplicates: if x is already in the set
    for (const auto& sol : set_possible_solution) {
        bool equal = true;
        for (int i = 0; i < n; ++i) {
            if (sol[i] != x[i]) { equal = false; break; }
        }
        if (equal) {
            DBG(cout << "EQUAL\n");
            return;
        }
    }

    DBG(cout << "NOT PRESENT\n");

    vector<int> cand(x, x + n);

    // Find insertion position:
    //  - fitness ascending (minimization)
    //  - lexicographic order on permutations (ascending)
    size_t pos = 0;

    while (pos < set_fx.size() && set_fx[pos] < fx) {
        ++pos;
        DBG(cout << "ORDER(fitness)\n");
    }

    while (pos < set_fx.size() && set_fx[pos] == fx &&
           lexicographical_compare(set_possible_solution[pos].begin(),
                                   set_possible_solution[pos].end(),
                                   cand.begin(), cand.end()))
    {
        ++pos;
        DBG(cout << "ORDER(lex)\n");
    }

    // If the set is full, accept only if cand is not worse than the current worst
    //  Worst is the last element because the set is kept ordered (fitness ascending).
    if (set_possible_solution.size() >= m) {
        unsigned long fworst = set_fx.back();              // largest fitness (worst)
        auto& worst_perm = set_possible_solution.back();

        // If cand has higher fitness, it's worse -> reject
        if (fx > fworst) {
            DBG(cout << "FULL: fx > fworst -> return\n");
            return;
        }

        // If same fitness as worst, accept only if cand is lexicographically smaller
        if (fx == fworst) {
            bool cand_better_lex = lexicographical_compare(
                cand.begin(), cand.end(),
                worst_perm.begin(), worst_perm.end()
            );
            if (!cand_better_lex) {
                DBG(cout << "FULL: fx==fworst and not lex-better -> return\n");
                return;
            }
        }
    }

    // Insert cand and its fitness at the computed position
    set_possible_solution.insert(set_possible_solution.begin() + pos, move(cand));
    set_fx.insert(set_fx.begin() + pos, fx);
    DBG(cout << "INSERT at pos=" << pos << "\n");

    rebuild_fitness_distances();

    // If size exceeds m, remove the solution.
    if (set_possible_solution.size() > m) {
        size_t S = fitness_distances.size();
        size_t idx_min = 0;

        for (size_t i = 1; i < S; ++i) {
            // take min lexicographical
            if (lexicographical_compare(fitness_distances[i].begin(), fitness_distances[i].end(),
                            fitness_distances[idx_min].begin(), fitness_distances[idx_min].end()))
            {
                idx_min = i;
            }
        }

        DBG(cout << "REMOVE idx_min=" << idx_min
                << " fd0(fx)=" << (ulong)fitness_distances[idx_min][0] << endl);
        
        // remove min lexicographical
        set_possible_solution.erase(set_possible_solution.begin() + idx_min);
        set_fx.erase(set_fx.begin() + idx_min);
        
        rebuild_fitness_distances();
    }
    
    DBG(
        cout << "\n[STATE] set_fx + permutations (ordered):\n";
        for (size_t i = 0; i < set_possible_solution.size(); ++i) {
            cout << "i=" << i << " fx=" << set_fx[i] << " perm=";
            int limit = (n < 200 ? n : 200);
            for (int k = 0; k < limit; ++k) cout << set_possible_solution[i][k] << " ";
            cout << "\n";
        }
        cout << "-------------------------------------\n";

        cout << "[STATE] fitness_distances (fitness + sorted Kendall distances):" << endl;
        for (size_t i = 0; i < fitness_distances.size(); ++i) {
            cout << "i=" << i
                << " fx=" << set_fx[i]  
                << " dists=";

            for (size_t k = 1; k < fitness_distances[i].size(); ++k) { // <-- solo distanze
                cout << fitness_distances[i][k] << " ";
            }
            cout << "\n";
        }
        cout << "-------------------------------------\n";
    );
}
