#ifndef LOP_H
#define LOP_H

//number of evaluations performed in the run
extern unsigned long nevals;

//read LOP instance file into a 1d matrix
void lop_readInstance(int*& lop1, int& n, int& opt, char* filename);

//print LOP instance as a 2d matrix
void lop_printInstance(int* lop1, int n);

//evaluate a permutation x using the 1d lop matrix
int lop_eval(int* x, int n, int* lop1);

//normalize the instance-matrix and optimum value if passed is normalized in the return
int lop_normalize(int* lop1, int n, int opt=-1);

//calculate upper and lower bounds of the lop instance (if normalized, lower bound is 0)
void lop_bounds(int* lop1, int n, int& lb, int& ub);

#endif

