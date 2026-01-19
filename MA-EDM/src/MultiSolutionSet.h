#include <vector>
#include <string>
using namespace std;
struct MultiSolutionSet {
    int m;  // max dimension set
    int n;  // length permutation
    vector<vector<int>> set_possible_solution;
    vector<unsigned long> set_fx;
    
    // fitness_distances[i][0] = fitness of i
    // fitness_distances[i][j+1] = KendallTau(i, j)
    vector<vector<unsigned long>> fitness_distances; 

    MultiSolutionSet(int m_, int n_);
    // ~MultiSolutionSet();

    void pdifference(int *diff,int *p2,int *p3,int n);
    void pcompose(int *p3,int *p1,int *p2,int n);
    void pinverse(int *pinv,int *p,int n);
    int countInversions(int* a, int size);
    int mergesortCount(int* a, int* temp, int begin, int end);
    int kendallTauDistance(int *p1,int *p2,int n);
    void update_set(const int* x, unsigned long fx);
    void rebuild_fitness_distances();

    void print_final_results(string& path,
                        int seed,
                        const string& algorithm,
                        int nevals,
                        double time_sec,
                        string outputfile, 
                        int m
                    );

};
