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

MultiSolutionSet::MultiSolutionSet(int m_, int n_) : m(m_), n(n_) {
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
    
    return v / (0.5*n*(n-1));
}

void MultiSolutionSet::update_set(const int* x, unsigned long fx) {
    //TODO: rincontrollare insieme S ed S' 
    //      ricontrollare kendell perchè vettore ordina distanze ma 
    //      forse non permutazioni e fiteness 

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
        size_t pos = 0;

        while (pos < set_fx.size() && set_fx[pos] > fx) {
            ++pos;
            DBG(cout << "ORDER" << endl);
        }
        // find position to insert (decreasing order for fx)
        set_possible_solution.insert(set_possible_solution.begin() + pos, move(cand));
        set_fx.insert(set_fx.begin() + pos, fx);
        DBG(cout << "INSERT" << endl);
    } 
    else {    
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
            vector<int> cand(x, x + n);
            size_t pos = 0;
            
            while (pos < set_fx.size() && set_fx[pos] > fx) {
                ++pos;
                DBG(cout << "ORDER" << endl);
            }
            
            set_possible_solution.insert(set_possible_solution.begin() + pos, move(cand));
            set_fx.insert(set_fx.begin() + pos, fx);
            DBG(cout << "INSERT (FULL -> REPLACE WORST)" << endl);
            
            // Vector of kendall distances 
            vector<double> ktDistances;
            size_t S = set_possible_solution.size();

            for (size_t i = 0; i < S; i++)
            {
                for (size_t j = i + 1; j < S; j++)
                {
                    double ktValue = kendellTau(set_possible_solution[i].data(),set_possible_solution[j].data(),n);
                    //DBG(cout << "KendallTau(" << i << "," << j << ") = " << ktValue << endl);
                    ktDistances.push_back(ktValue);
                }
            }

            // Sort small -> big
            sort(ktDistances.begin(), ktDistances.end());
            DBG(cout << "KendallTau distances sorted (" << ktDistances.size() << "):" << endl);
            // for (double d : ktDistances) 
            //     DBG(cout << d << " " << endl); 
            double min_val_kendel_vect = ktDistances[0];
            DBG(cout << min_val_kendel_vect << endl); 
            DBG(cout << " ------------------------------ ");
        }
    }
            
    // DBG(cout << "SIZE = " << set_possible_solution.size() << endl);

	// for (const auto& perm : set_possible_solution) {
	// 	for (int v : perm) {
	// 		DBG(cout << v << " ");
	// 	}
	// 	DBG(cout << endl);
	// }
	
}