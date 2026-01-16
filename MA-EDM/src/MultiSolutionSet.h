#include <vector>
using namespace std;
struct MultiSolutionSet {
    int m;  // max dimension set
    int n;  // length permutation
    vector<vector<int>> set_possible_solution;
    vector<unsigned long> set_fx; 

    MultiSolutionSet(int m_, int n_);
    ~MultiSolutionSet();

    void update_set(const int* x, unsigned long fx);
};
