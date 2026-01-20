#include "MultiSolutionSet.h"
#include <iostream>
#include <utility>
#include <algorithm>
#include <string.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>
using namespace std;

#ifdef DEBUG
  #define DBG(stmt) do { stmt; } while(0)
#else
  #define DBG(stmt) do {} while(0)
#endif

MultiSolutionSet::MultiSolutionSet(int m_, int n_){
    m = m_; 
    n = n_;
    //DBG(cout << "Sono il costruttore! m=" << m << " n=" << n << "\n");
}

// MultiSolutionSet::~MultiSolutionSet() {
//     //DBG(cout << "Sono il distruttore!\n");
// }

// int kendallTau(int *a, int *b, int n){
//     int i,j,v = 0; 
//     for (j=1; j < n; j++)
//         for (i=0; i < j; i++)
//             v += (a[i] < a[j]) != (b[i] < b[j]);
//     return v;
// }

// efficient Kendall Tau distance implementation

int MultiSolutionSet::mergesortCount(int* a, int* temp, int begin, int end) {
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

int MultiSolutionSet::countInversions(int* a, int size) {
	//returns the number of inversions in a and sort a
	int* workspace = new int[size];
	int ninv = mergesortCount(a,workspace,0,size-1);
	delete[] workspace;
	return ninv;
}

void MultiSolutionSet::pinverse(int *pinv,int *p,int n) {
    for(int i=0; i<n; i++)
        pinv[p[i]]=i;
}

void MultiSolutionSet::pcompose(int *p3,int *p1,int *p2,int n) {
	for(int i=0;i<n;i++)
		p3[i]=p1[p2[i]];
}

void MultiSolutionSet::pdifference(int *diff,int *p2,int *p3,int n) {
    int *p2inv=new int[n];
	pinverse(p2inv,p2,n);
	pcompose(diff,p2inv,p3,n);  
	delete[] p2inv;
}

int MultiSolutionSet::kendallTauDistance(int *p1,int *p2,int n) {
    int* p3=new int[n];
    pdifference(p3,p1,p2,n);
	int res=countInversions(p3,n);
	delete[] p3;
	return res;
}


void MultiSolutionSet::rebuild_fitness_distances() {
     size_t S = set_possible_solution.size();

    // [0]=fitness, [1..S]=dist 
    fitness_distances.assign(S, vector<unsigned long>(S + 1, 0ULL));

    // fitness in columns 0
    for (size_t i = 0; i < S; ++i) {
        fitness_distances[i][0] = set_fx[i];
    }

    // kendall
    for (size_t i = 0; i < S; ++i) {
        for (size_t j = i + 1; j < S; ++j) {
             int d = kendallTauDistance(
                set_possible_solution[i].data(),
                set_possible_solution[j].data(),
                n
            );
            fitness_distances[i][j + 1] = d;
            fitness_distances[j][i + 1] = d;
        }
    }

    for (size_t i = 0; i < S; ++i) {
        sort(fitness_distances[i].begin() + 1, fitness_distances[i].end());
    }
}


void MultiSolutionSet::update_set(const int* x, uint64_t fx) {
    /*
    Maintain the set ordered by:
    - fitness descending (maximize fitness)
    - if tie, distance vector lexicographically ascending (lexicographically smallest is best)
    */

    // Duplicate check
    for (const auto& sol : set_possible_solution) {
        bool equal = true;
        for (int i = 0; i < n; ++i) {
            if (sol[i] != x[i]) { equal = false; break; }
        }
        if (equal) {
            DBG(cout << "DUPLICATE -> skip\n");
            return;
        }
    }

    vector<int> cand(x, x + n);

    // Find insertion position to keep ordering:
    // - higher fitness first
    // - for equal fitness, lexicographically smaller first
    size_t pos = 0;

    while (pos < set_fx.size() && set_fx[pos] > fx) {
        ++pos;
    }


    while (pos < set_fx.size() && set_fx[pos] == fx && ///togliere questo confronto
           lexicographical_compare(set_possible_solution[pos].begin(),
                                   set_possible_solution[pos].end(),
                                   cand.begin(), cand.end()))
    {
        ++pos;
    } //togliere fino a qua

    // If full, accept only if cand is better than the current worst (last element).
    if (set_possible_solution.size() == m) {
        unsigned long fworst = set_fx.back();           // smallest fitness (worst)
        auto& worst_perm = set_possible_solution.back();

        // Lower fitness than worst -> reject
        if (fx < fworst) {
            DBG(cout << "FULL: worse fitness -> reject\n");
            return;
        }

        // Same fitness: accept only if lexicographically smaller than the current worst
        if (fx == fworst) {
            bool cand_better_lex = lexicographical_compare(
                cand.begin(), cand.end(),
                worst_perm.begin(), worst_perm.end()
            );
            if (!cand_better_lex) {
                DBG(cout << "FULL: same fitness, not lex-better -> reject\n");
                return;
            }
        }
    }

    // Insert candidate
    set_possible_solution.insert(set_possible_solution.begin() + pos, move(cand));
    set_fx.insert(set_fx.begin() + pos, fx);

    // If size exceeds m, drop the worst (last), which is correct for the requested ranking.
    if (set_possible_solution.size() > m) {
        set_possible_solution.pop_back();
        set_fx.pop_back();
    }

    rebuild_fitness_distances();

    DBG(
        cout << "\n[STATE] set_fx + permutations (ordered by fitness DESC, lex ASC):\n";
        for (size_t i = 0; i < set_possible_solution.size(); ++i) {
            cout << "i=" << i << " fx=" << set_fx[i] << " perm=";
            int limit = (n < 200 ? n : 200);
            for (int k = 0; k < limit; ++k) cout << set_possible_solution[i][k] << " ";
            cout << "\n";
        }
        cout << "-------------------------------------\n";

        cout << "[STATE] fitness_distances (fitness + sorted Kendall distances):\n";
        for (size_t i = 0; i < fitness_distances.size(); ++i) {
            cout << "i=" << i
                 << " fx=" << fitness_distances[i][0]
                 << " dists=";
            for (size_t k = 1; k < fitness_distances[i].size(); ++k) {
                cout << fitness_distances[i][k] << " ";
            }
            cout << "\n";
        }
        cout << "-------------------------------------\n";
    );
}
string perm_to_semicolon(const vector<int>& perm) {
    ostringstream oss;
    for (size_t i = 0; i < perm.size(); ++i) {
        if (i) oss << ';';
        oss << perm[i];
    }
    return oss.str();
}


string join_to_semicolon(const vector<unsigned long>& v, size_t from = 0) {
    ostringstream oss;
    for (size_t i = from; i < v.size(); ++i) {
        if (i > from) oss << ';';
        oss << v[i];
    }
    return oss.str();
}

void MultiSolutionSet::print_final_results(const string& path,
                                           int seed,
                                           const string& algorithm,
                                           int nevals,
                                           double time_sec,
                                           string outputfile, 
                                           int m)
                                                 
{
    ofstream out(outputfile, ios::app);
    if (!out) return;

    // header 
    out.seekp(0, ios::end);
    if (out.tellp() == 0) {
        out << "seed\talgorithm\tm\tinstance\tn\tnevals\ttime\tpermutation\tfitness\tkendall_dists\n";
    }

    // One row per solution in the set
    size_t S = set_possible_solution.size();
    for (size_t i = 0; i < S; ++i) {
        string perm_str = perm_to_semicolon(set_possible_solution[i]);

        // fitness_distances[i] = [fitness, d1, d2, ...] with distances sorted
        unsigned long fit_i = set_fx[i];
        string dists_str;
        if (i < fitness_distances.size()) {
            dists_str = join_to_semicolon(fitness_distances[i], 1); // skip [0]=fitness
        }

        out << seed << '\t'
            << algorithm << '\t'
            << m << '\t'
            << path << '\t'
            << n << '\t'
            << nevals << '\t'
            << time_sec << '\t'
            << perm_str << '\t'
            << fit_i << '\t'
            << dists_str
            << endl;
    }
}
