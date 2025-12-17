/*    
    ILSLOP Iterated Local Search Algorithm for Linear Ordering Problem
    Copyright (C) 2004  Tommaso Schiavinotto (tommaso.schiavinotto@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _LO_H_
#define _LO_H_

#define MAXLOCOPTTYPES 5

extern long int **CostMat;
extern long int *RandomScan;
extern char PerturbationType;
extern char LocalOptType;
extern int Greedy;
extern int NumSwaps;
extern int MaxPertLength;
extern int LocalOptIdx;
extern unsigned int LocOptMax;
extern long int **DiffMat;
extern long long int (*LocOptArray[MAXLOCOPTTYPES])(long int *);
extern long long int ls_iteration;
extern unsigned long long int evaluations;
#define MAXREV 8
#define NUMSWAPS 4


long long int computeCost ( long int *lo );
void initDS ();
long long int localOpt ( long int *lo );
void perturbate (long int *lobest, long int *lo, long long int *length);
void initialSolution(long int *lo);
void addLocOptFunction(char f);
void localSearch(long int *lo, long long int *l);
void CalculateTotSum ();
void SearchPositionsForLocalOptima(int * validPositions, int item);
void CalculateUbiquityMatrix();
#endif
