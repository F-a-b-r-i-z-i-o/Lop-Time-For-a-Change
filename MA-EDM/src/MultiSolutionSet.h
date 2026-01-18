#include <vector>
using namespace std;
struct MultiSolutionSet {
    int m;  // max dimension set
    int n;  // length permutation
    vector<vector<int>> set_possible_solution;
    vector<unsigned long> set_fx;
    
    // fitness_distances[i][0] = fitness of i
    // fitness_distances[i][j+1] = KendallTau(i, j)
    vector<vector<double>> fitness_distances; 

    MultiSolutionSet(int m_, int n_);
    ~MultiSolutionSet();

    void update_set(const int* x, unsigned long fx);
    void rebuild_fitness_distances();
};
