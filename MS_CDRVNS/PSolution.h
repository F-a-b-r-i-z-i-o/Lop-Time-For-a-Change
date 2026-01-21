#ifndef PSOLUTION_H
#define PSOLUTION_H
#include "PSolution_util.cpp"
#include "random.h"

//structure to contain i<j and p(i<j)
struct PrecWithProb {
    int i,j;
    double p;
};


//data structure to speedup max-selection in construct
struct PrecWithValue {
    int i,j,vint;
};




struct PSolution {
    
    static int n;        //number of items
    static long int nc2;        //{n \choose 2} = n*(n-1)/2
    static long int n2;        //n^2
    static int* lop;    //1d matrix of the lop instance values
    static double* v;    //1d matrix of the precedences values
    static PrecWithValue* lopSorted; //lop entries sorted in descending order
    static int* invLopSorted; //inverse map of lopSorted
    static int* wmem;    //working memory (2*n^2 needed in construct/propagate, nc2 needed in destruct)
    static PrecWithProb* precs; //required by destruct_sorted
    
    unsigned long fit;    //(partial) fitness of this solution
    
    int* data;            //memory storage (useful to make it persistent)
    int** u;            //entry u[i] is the list of unset precedences involving item i (no more than n)
    int** s;            //entry s[i] is the list of items j such as i<j (no more than n)
    int** p;            //entry p[i] is the list of items j such as j<i (no more than n)
    int* un;            //entry un[i] is the length of u[i]
    int* sn;            //entry sn[i] is the length of s[i]
    int* pn;            //entry pn[i] is the length of p[i]
    int** ui;            //entry ui[i] is the inversion of u[i] (n entries)
    int** si;            //entry si[i] is the inversion of s[i] (n entries)
    int** pi;            //entry pi[i] is the inversion of p[i] (n entries)
    
    long int np;                //number of precedences in this partial solution
    long double vsum;    //sum of the v-values of available precedences
    long double* uvsum;    //array of the values' sums of unset precedences
    //double vmax;        //max of the v-values of available precedences
    //int imax,jmax;        //i,j indexes corresponding to vmax in v
    int pmax;            //pointer to max value in lopSorted
    bool vsum_valid;    //indicate when vsum has to be recalculated
    //bool vmax_valid;    //indicate when vmax has to be recalculated
    int rem_max_val,rem_max_i,rem_max_j; //used for fast update of pmax pointer after destruction
    
    //constructor and destructor only allocate and free memory
    PSolution(int n);
    ~PSolution();
    
    //static functions to setup lop matrix and free static memory
    static void setLOPMatrix(int* lop1, int n);
    static void freeStaticMemory();
    
    //functions to: set solution to an empty one (no precedence set), check if it is a total order (full permutation), evaluate this partial solution
    void makeEmpty();
    bool isTotal();
    unsigned long eval();
    
    //converstion from/to n-length permutation
    void fromPermutation(int* x, int fit=-1);
    void toPermutation(int* x);
    
    //constructive heuristic to make the solution complete starting from this partial solution
    void construct(double q=1.);    //main heuristic function, q is the probability to make greedy moves
    void propagate(int i, int j);    //set i<j and its induced precedences, it also updates np,fit and vsum_valid,vmax_valid
    
    //destruction procedure (usually applied to a total solution, no guarantees on partial solutions)
    //npr is the number of precedence to remove, and it returns the number of precedences removed (it is *practically* always equal to npr, btw they are guaranteed to be in [npr-n,npr+n])
    //the partial solution is guaranteed to be a set of precedences closed transitively
    long int destruct(long int npr=5);
    long int destruct(double r=0.1);    //r is the percentage of precedences to remove, returns the number of precedences removed
    int countPrecsToRemove(int a, int c);    //counts how many precedences (a<c + induced ones) will be removed without removing them
    int remPrecs(int a, int c);                //remove precedence a<c + induced ones and returns the number of removed precedences
    
    //new destruction procedures: with threshold, stochastic, as destruct(double) but with ordering
    int destruct_threshold(long** countersMatrix, double alpha=0.8);
    int destruct_stochastic(long** countersMatrix);
    long int destruct_sorted(long** countersMatrix, double r=0.1);
    long int destruct_sorted(long** countersMatrix, long int npr=5);
    
    //recalculate vsum and vmax/imax/jmax if not valid
    //void compute_vsum_vmax();
    void compute_vsum();
    //void compute_vmax();
    
    //printing functions for matrix display or debug
    void print();
    void printData();
    
};
#endif
