#include "MultiSolutionSet.h"
#include <iostream>
#include <utility>

using namespace std;

#ifdef DEBUG
  #define DBG(stmt) do { stmt; } while(0)
#else
  #define DBG(stmt) do {} while(0)
#endif

MultiSolutionSet::MultiSolutionSet(int m_, int n_) : m(m_), n(n_) {
    DBG(cout << "Sono il costruttore! m=" << m << " n=" << n << "\n");
}

MultiSolutionSet::~MultiSolutionSet() {
    DBG(cout << "Sono il distruttore!\n");
}

void MultiSolutionSet::update_set(const int* x, unsigned long fx) {
    for (const auto& sol : set_possible_solution) {
        bool equal = true;

        for (int i = 0; i < n; ++i) {
            if (sol[i] != x[i]) {
                equal = false;
                break;
            }
        }

        if (equal) {
            DBG(cout << "EQUAL" << endl);
            return; 
        }
    }

    DBG(cout << "NOT PRESENT" << endl);

    if (set_possible_solution.size() < m) {
        vector<int> cand(x, x + n);
        DBG(cout << "INSERT" << endl);
    } else {
        unsigned long fworst = set_fx[0];
        int idx_worst = 0; 
        for (size_t i = 1; i < set_fx.size(); i++)
        {
            if (set_fx[i] < fworst){
                fworst = set_fx[i];
                idx_worst = i;
            }
        }
        if (fx<fworst)
            return;
        else{
            //cout<<"TODO";
        }
    }
            
    DBG(cout << "SIZE = " << set_possible_solution.size() << endl);

	for (const auto& perm : set_possible_solution) {
		for (int v : perm) {
			DBG(cout << v << " ");
		}
		DBG(cout << endl);
	}
	
}