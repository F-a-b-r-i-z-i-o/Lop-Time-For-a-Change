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
    if (set_possible_solution.size() >= (size_t)m) {
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
        size_t idx_max = 0;

        for (size_t i = 1; i < S; ++i) {
            //TODO: control this point for idx_max or min

            // if fitness_distances[idx_max] < fitness_distances[i], then i is larger -> update idx_max
            if (lexicographical_compare(fitness_distances[idx_max].begin(), fitness_distances[idx_max].end(),
                                        fitness_distances[i].begin(), fitness_distances[i].end()))
            {
                idx_max = i;
            }
        }

        DBG(cout << "REMOVE idx_max=" << idx_max
                << " fd0(fx)=" << (ulong)fitness_distances[idx_max][0] << endl);

        set_possible_solution.erase(set_possible_solution.begin() + idx_max);
        set_fx.erase(set_fx.begin() + idx_max);

        // Rebuild because indices and sizes changed
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
