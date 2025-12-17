/*    
    ILSLOP Iterated Lcaol Search Algorithm for Linear Ordering Problem
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

#include <stdio.h>
#include <stdlib.h>
#include "lop.h" 
#include "instance.h"
#include "utilities.h"
#include "timer.h"
#include "ck.h"
//#ifdef __MINGW32__
#include <float.h>
#define MAX_FLOAT FLT_MAX
//#else
//#define MAX_FLOAT FLT_MAX_EXP
//#endif
//Integer maximum value
#define MAX_INTEGER 100000000

//Integer minimum value
#define MIN_INTEGER -100000000


long long int ls_iteration;
char LocalOptType;
int LocalOptIdx;
unsigned int LocOptMax;
long long int (*LocOptArray[MAXLOCOPTTYPES])(long int *);
unsigned long long int evaluations=0;
typedef struct _heap_el {
  int hp_node;
  float hp_value;
} heap_el_t;


static int *m_restrictionBounds;
//static int **m_restrictionBounds;
static int m_max;
static int m_min;
int * sequence;
int * changes;
float * aux;
long * myvector;



long int **CostMat;
long int *RandomScan;
char PerturbationType;
int Greedy;
int NumSwaps;
int MaxPertLength;


long long int computeCost (long int *lo ) {
    int h,k;
    long long int t;
    
    /* Diagonal value are not considered */
    for (t = 0, h = 0; h < PSize; h++ ) 
	for ( k = h + 1; k < PSize; k++ )
	    t += CostMat[lo[h]][lo[k]];

    evaluations++;
    if (t<0) exit(1);
    return(t);
}

void initDS () {
}

#define REMEMBER_POS 1

long long int localOptInsertSmartBI (long int *lo ) {
    long long int gain, maxgain;
    long int k, j, t, best_i=-1, best_j=-1, i;
    static double mxw = 1.0;
    static double mnw = 0.8;
    long int mxwsize,mnwsize,inflim, suplim;
    static int t2cmod = 0;
    static int time2change = 0;

#ifdef REMEMBER_POS
    static int last_i = 0;
    int v;
#endif

    if (!time2change) time2change = PSize/2;



    maxgain = 0;
    /* BE careful depends on ls_iterations */
    if (mnw > 0) {
	if (ls_iteration > (t2cmod+time2change)) {
	    t2cmod = ls_iteration;
	    time2change /= 2;
	    mxw -= 0.2;
	    mnw -= 0.2;
	}
    }
    
    mxwsize = PSize * mxw;
    mnwsize = PSize * mnw;

#if REMEMBER_POS
    for ( v = 0, i = last_i; v < PSize; v++, i++ ) {
	if (i == PSize) i = 0;
#else
    for ( i = 0; i < PSize; i++ ) {
#endif
	
	inflim = i-mxwsize;
	if (inflim < 0) inflim = 0;
	suplim = i+mxwsize;
	if (suplim > PSize) suplim = PSize;
	gain = 0;
	for (j = i-1; j >= inflim; j--) {
	    gain += DiffMat[lo[i]][lo[j]];
	    //gain += CostMat[lo[i]][lo[j]] - CostMat[lo[j]][lo[i]];
	    if (gain > maxgain ) {
		maxgain = gain;
		best_i = i;
		best_j = j;
	    }
	}

	gain = 0;
	for (j = i+1; j < suplim; j++) {
	    gain += DiffMat[lo[j]][lo[i]]; 
	    //gain += CostMat[lo[j]][lo[i]] - CostMat[lo[i]][lo[j]]; 
	    if (gain > maxgain ) {
		maxgain = gain;
		best_i = i;
		best_j = j;
	    }
	}
	
	if (abs(best_i-best_j) > mnwsize) {
#ifdef REMEMBER_POS
	    last_i = i+1;
#endif
	    break;
	}
    }

    if ( maxgain > 0 ) {
	i = best_i;
	j = best_j;
	t = lo[i];
	if (i < j) 
	    for ( k = i+1; k <= j; k++ ) lo[k-1] = lo[k];
	else 
	    for ( k = i - 1; k >= j; k-- ) lo[k+1] = lo[k];
	lo[j] = t;
//	printf("LS: %Ld %d (%d)\n", ls_iteration, abs(i-j), maxgain);
    }
    mxw = 1.0;
    mnw = 0.8;
    t2cmod = 0;
    time2change = 0;
    return(maxgain);
}


long long int localOptInsertBI ( long int *lo ) {
    long long int gain, maxgain;
    long int k, j, t, best_i=-1, best_j=-1, i;
    
    maxgain = 0;
    for ( i = 0; i < PSize; i++ ) {
	gain = 0;

	for (j = i-1; j >= 0; j--) {
	    gain += DiffMat[lo[i]][lo[j]]; // - CostMat[lo[j]][lo[i]];
	    if (gain > maxgain ) {
		maxgain = gain;
		best_i = i;
		best_j = j;
	    }
	}

	gain = 0;
	for (j = i+1; j < PSize; j++) {
	    gain += DiffMat[lo[j]][lo[i]]; // - CostMat[lo[i]][lo[j]]; 
	    if (gain > maxgain ) {
		maxgain = gain;
		best_i = i;
		best_j = j;
	    }
	}
    }
    if ( maxgain > 0 ) {
	i = best_i;
	j = best_j;
	t = lo[i];
	if (i < j) 
	    for ( k = i+1; k <= j; k++ ) lo[k-1] = lo[k];
	else 
	    for ( k = i - 1; k >= j; k-- ) lo[k+1] = lo[k];
	lo[j] = t;
//	printf("LS: %Ld %d (%d)\n", ls_iteration, abs(i-j), maxgain);
	return(maxgain);
    }

    return(0);
}


long long int localOptInsert (long int *lo ) {
    long int s_i, v, j, g, t, k;
    long long int gain;
    static long int CurrentIdx = 0;


    s_i = CurrentIdx + PSize;
    gain = 0;
    for ( v = CurrentIdx; (v < s_i) && ( gain == 0 ); v++ ) {
	if (RandomScan) 
	    CurrentIdx = RandomScan[v%PSize];
	else 
	    CurrentIdx = v%PSize;
	/* printf("%d[%d] < %d\n", v, CurrentIdx, s_i); */

	for (j = 0; (j < PSize) && (gain == 0); j++) {
	    if ( j == CurrentIdx ) continue;
	    if ( j < CurrentIdx ) {
		for (g = 0, k = j; k < CurrentIdx; k++)
		    g += CostMat[lo[CurrentIdx]][lo[k]] - 
			CostMat[lo[k]][lo[CurrentIdx]];
            evaluations++;
		if ( g > 0 ) {
		    gain = g;
		    t = lo[CurrentIdx];
		    for ( k = CurrentIdx - 1; k >= j; k-- )
			lo[k+1] = lo[k];
		    lo[j] = t;
		}
	    } else {
		for (g = 0, k = CurrentIdx+1; k <= j; k++)
		    g += CostMat[lo[k]][lo[CurrentIdx]] - 
			CostMat[lo[CurrentIdx]][lo[k]];
                        evaluations++;
		if ( g > 0 ) {
		    gain = g;
		    t = lo[CurrentIdx];
		    for ( k = CurrentIdx + 1; k <= j; k++ )
			lo[k-1] = lo[k];
		    lo[j] = t;
		}
	    }
	}
    }
    CurrentIdx = (v-1+PSize)%PSize;
    return(gain);
}

int Continue(){
    if (evaluations_criterion==1)
        return evaluations<max_evaluations;
    else
        return elapsed_time(VIRTUAL)<try_time;
}

long long int localOptFastInsert (long int *lo ) {

    long int ri,i,j,t,k,v, best_j;
    long int maxgain;
    static long int last_i = 0;

    best_j = 0;     
    maxgain = 0;
    
    for ( v = 0, ri = last_i; v < PSize && Continue(); v++, ri++ ) {
        register long int gain;

        gain = maxgain = 0;
        if (ri == PSize) ri = 0;
        if (RandomScan) {
            i = RandomScan[ri];
        } else {
            i = ri;
        }
        
        for (j = i-1; j >= 0 && Continue(); j--) {
            gain += DiffMat[lo[i]][lo[j]];
            if (gain > maxgain ) {
                maxgain = gain;
                best_j = j;
            }
            evaluations++;
        }
        gain = 0;
        for (j = i+1; j < PSize && Continue(); j++) {
            gain += DiffMat[lo[j]][lo[i]];
            if (gain > maxgain ) {
                maxgain = gain;
                best_j = j;
            }
            evaluations++;
        }
        
        //update solution with the best movement.
        if ( maxgain > 0 ) {
            j = best_j;
            t = lo[i];
            if (i < j)
                for ( k = i+1; k <= j; k++ ) lo[k-1] = lo[k];
            else
                for ( k = i - 1; k >= j; k-- ) lo[k+1] = lo[k];
            lo[j] = t;
            last_i = ri+1;
            //	    printf("LS: %Ld %d (%d)\n", ls_iteration, abs(i-j), maxgain);
            //printf("LS: %Ld %ld %ld (%ld)\n", ls_iteration, i, j, maxgain);
//            printf("%g, buk \n",elapsed_time(VIRTUAL));
            return maxgain;
        }
    }
    return(0);
}
    
    
    long long int localOptFastInsertUbiquity (long int *lo ) {
        
        long int ri,i,j,t,k,v, best_j;
        long int maxgain;
        static long int last_i = 0;
        
        best_j = 0;
        maxgain = 0;
        int value;
        for ( v = 0, ri = last_i; v < PSize && Continue(); v++, ri++ ) {
            register long int gain;

            gain = maxgain = 0;
            if (ri == PSize) ri = 0;
            if (RandomScan) {
                i = RandomScan[ri];
            } else {
                i = ri;
            }

            value=m_restrictionBounds[lo[i]];
            if (value>0){
                m_min=value;
                m_max=(int)PSize;
            }
            else{
                m_min=0;
                m_max=(int)PSize+value;
            }
            
            for (j = i-1; j >= m_min && Continue(); j--) {

                gain += DiffMat[lo[i]][lo[j]];
                if (gain > maxgain ){
                    maxgain = gain;
                    best_j = j;
                }
                evaluations++;
            }
            gain = 0;
            for (j = i+1; j < m_max && Continue(); j++) {

                gain += DiffMat[lo[j]][lo[i]];
                if (gain > maxgain ){
                    maxgain = gain;
                    best_j = j;
                }
                evaluations++;
            }
            if ( maxgain > 0 ) {
                j = best_j;
                t = lo[i];
                if (i < j) 
                    for ( k = i+1; k <= j; k++ ) lo[k-1] = lo[k];
                else 
                    for ( k = i-1; k >= j; k-- ) lo[k+1] = lo[k];
                lo[j] = t;
                last_i = ri+1;
                return maxgain;
            }
        }
        return(0);
    }
    
    long long int localOptInsertUbiquity (long int *lo ) {

        long int s_i, v, j, g, t, k;
        long long int gain;
        static long int CurrentIdx = 0;
        
        
        s_i = CurrentIdx + PSize;
        gain = 0;
        for ( v = CurrentIdx; (v < s_i) && ( gain == 0 ); v++ ) {
            if (RandomScan)
                CurrentIdx = RandomScan[v%PSize];
            else
                CurrentIdx = v%PSize;
            /* printf("%d[%d] < %d\n", v, CurrentIdx, s_i); */
            
            for (j = 0; (j < PSize) && (gain == 0); j++) {

                if ( j == CurrentIdx ) continue;

                if ( j < CurrentIdx ) {
                    for (g = 0, k = j; k < CurrentIdx; k++)
                        g += CostMat[lo[CurrentIdx]][lo[k]] -
                        CostMat[lo[k]][lo[CurrentIdx]];
                                evaluations++;
                    if ( g > 0 ) {
                        gain = g;
                        t = lo[CurrentIdx];
                        for ( k = CurrentIdx - 1; k >= j; k-- )
                            lo[k+1] = lo[k];
                        lo[j] = t;
                    }
                } else {
                    for (g = 0, k = CurrentIdx+1; k <= j; k++)
                        g += CostMat[lo[k]][lo[CurrentIdx]] - 
                        CostMat[lo[CurrentIdx]][lo[k]];
                                evaluations++;
                    if ( g > 0 ) {
                        gain = g;
                        t = lo[CurrentIdx];
                        for ( k = CurrentIdx + 1; k <= j; k++ )
                            lo[k-1] = lo[k];
                        lo[j] = t;
                    }
                }
               // }
            }
        }
        CurrentIdx = (v-1+PSize)%PSize;

        return(gain);

    }


long long int localOptFastInsert_acceptzero (long int *lo ) {
    long int i,j,t,k,v, best_j;
    long int gain, maxgain;
    static int last_i = 0;

    best_j = 0;     
    maxgain = gain = -1;
    
    for ( v = 0, i = last_i; v < PSize; v++, i++ ) {
	gain = 0;
	maxgain = -1;
	if (i == PSize) i = 0;
	for (j = i-1; j >= 0; j--) {
	    gain += DiffMat[lo[i]][lo[j]]; // - CostMat[lo[j]][lo[i]];
	    if (gain > maxgain ) {
		maxgain = gain;
		best_j = j;
	    }
	}
	gain = 0;
	for (j = i+1; j < PSize; j++) {
	    gain += DiffMat[lo[j]][lo[i]]; // - CostMat[lo[i]][lo[j]]; 
	    if (gain > maxgain ) {
		maxgain = gain;
		best_j = j;
	    }
	}
	    
	if ( maxgain >= 0 ) {
	    j = best_j;
	    t = lo[i];
	    if (i < j) 
		for ( k = i+1; k <= j; k++ ) lo[k-1] = lo[k];
	    else 
		for ( k = i - 1; k >= j; k-- ) lo[k+1] = lo[k];
	    lo[j] = t;
	    last_i = i+1;
//	    printf("LS: %Ld %d %d (%Ld)\n", ls_iteration, i, j, maxgain);
	    return(maxgain);
	}
    }
    return(0);
}


long long int localOptInterchangeBI (long int *lo ) {
    long int j, k, g, gain, min, max, fmin, fmax;
    long int CurrentIdx;
    
    gain = 0;
    fmin = fmax = 0;
    for ( CurrentIdx = 0; (CurrentIdx < PSize); CurrentIdx++ ) {
	for (j = 0; (j < PSize); j++) {
	    if ( j == CurrentIdx ) continue;
	    min = (j > CurrentIdx)?CurrentIdx:j;
	    max = (j < CurrentIdx)?CurrentIdx:j;
	    
	    for ( g = 0, k = min + 1; k <= max; k++) 
		g += CostMat[lo[k]][lo[min]] - CostMat[lo[min]][lo[k]];
	    
	    for ( k = min + 1; k < max; k++) 
		g += CostMat[lo[max]][lo[k]] - CostMat[lo[k]][lo[max]];

	    if ( g > gain ) {
		gain = g;
		fmin = min;
		fmax = max;
	    }
	}
    }
    
    if ( gain > 0 ) {
	g = lo[fmin];
	lo[fmin] = lo[fmax];
	lo[fmax] = g;
    }

    return(gain);
}


long long int localOptInterchange (long int *lo ) {
    long int s_i, v, j, g, gain, min, max, k;
    static long int CurrentIdx = 0;
    
    s_i = CurrentIdx + PSize;
    gain = 0;
    for ( v = CurrentIdx; (v < s_i) && ( gain == 0 ); v++ ) {
	if (RandomScan) 
	    CurrentIdx = RandomScan[v%PSize];
	else 
	    CurrentIdx = v%PSize;
	for (j = 0; (j < PSize) && (gain == 0); j++) {
	    if ( j == CurrentIdx ) continue;
	    min = (j > CurrentIdx)?CurrentIdx:j;
	    max = (j < CurrentIdx)?CurrentIdx:j;
	    
	    
	    for ( g = 0, k = min + 1; k <= max; k++) 
		g += CostMat[lo[k]][lo[min]] - CostMat[lo[min]][lo[k]];
	    
	    for ( k = min + 1; k < max; k++) 
		g += CostMat[lo[max]][lo[k]] - CostMat[lo[k]][lo[max]];

	    //printf("Swapping: %ld <-> %ld. [%ld]\n", CurrentIdx, j, g); 
	    
	    if ( g > 0 ) {
		gain = g;
		k = lo[min];
		lo[min] = lo[max];
		lo[max] = k;
	    }
	}
    }
    CurrentIdx = (v-1+PSize)%PSize;
    return(gain);
}

/* This Local Optimization is done with a swap (first improvement)*/
long long int localOptSwap (long int *lo ) {
    long int s_i, v, next, itmp, gain; 
    static int CurrentIdx;
    
    s_i = CurrentIdx + PSize;
    gain = 0;
    for ( v = CurrentIdx; (v < s_i) && ( gain == 0 ); v++ ) {
	if (RandomScan) 
	    CurrentIdx = RandomScan[v%PSize];
	else 
	    CurrentIdx = v%PSize;
	next = CurrentIdx + 1;
	if ( next == PSize ) continue;
	itmp = CostMat[lo[next]][lo[CurrentIdx]] - 
	    CostMat[lo[CurrentIdx]][lo[next]];
	if ( itmp > 0 ) {
	    /*printf("Exchanged: %d<->%d [%d<->%d] (%d)\n", CurrentIdx, next,
	      lo[CurrentIdx], lo[next], itmp);*/
	    gain = itmp;
	    itmp = lo[CurrentIdx];
	    lo[CurrentIdx] = lo[next];
	    lo[next] = itmp;
	}
    }
    
    return(gain);
}
 
long long int localOpt (long int *lo ) {

    switch (LocalOptType) {
	case 'x':
	    return(localOptInterchange(lo));
	case 'X':
	    return(localOptInterchangeBI(lo));
	case 's':
	    return(localOptSwap(lo));
	case 'f':
	    return(localOptFastInsert(lo));
    case 'F':
        return(localOptFastInsertUbiquity(lo));
	case 'z':
	    return(localOptFastInsert_acceptzero(lo));
	case 'i':
	    return(localOptInsert(lo));
    case 'u':
        return(localOptInsertUbiquity(lo));
    case 'I':
	    return(localOptInsertBI(lo));
	case 'R':
	    return(localOptInsertSmartBI(lo));
	case 'c':
	case 'C':
	    return(CKLocalSearch(lo));
	case 'm':
	    return(LocOptArray[LocalOptIdx](lo));
	default:
	    fatal("Unknown Local Optimization type.");
    }
    /* It is never reached */
    return(0);
}
     

void perturbateInsert (long int * lobest, long int *lo, long long int *length) {
    long int j, n1, n2, t, k, w, wp;
    long long int g;
    
    for ( j = 0; j < PSize; j++ )
	lo[j] = lobest[j];


    for (j = 0; j < NumSwaps; j++) {
	n1 = (int)(ran01(&Seed)*(PSize)); 
	if ( n1 >= PSize ) n1 = PSize - 1;
	
	/* Possible window of choice for values > n1 */
	w = ((PSize-n1)<MaxPertLength)?(PSize-n1):MaxPertLength;
	/* Possible window of choice for values < n1 */
	wp = (n1<MaxPertLength)?n1:MaxPertLength;

	/* if MaxPertLength == PSize => w == PSize */
	
	do {
	    n2 = n1 - wp + (int)(ran01(&Seed)*(w+wp));
	    if ( n2 >= PSize ) n2 = PSize - 1;	    
	} while ( n1 == n2 );
	
	if ( n2 < n1 ) {
	    for (g = 0, k = n2; k < n1; k++)
		g += CostMat[lo[n1]][lo[k]] - 
		    CostMat[lo[k]][lo[n1]];
	    t = lo[n1];
	    for ( k = n1 - 1; k >= n2; k-- )
		lo[k+1] = lo[k];
	    lo[n2] = t;
	} else {
	    for (g = 0, k = n1+1; k <= n2; k++)
		g += CostMat[lo[k]][lo[n1]] - 
		    CostMat[lo[n1]][lo[k]];
	    t = lo[n1];
	    for ( k = n1 + 1; k <= n2; k++ )
		lo[k-1] = lo[k];
	    lo[n2] = t;
	}
	*length += g;
    }
} 


/* The perturbation is done swapping random positions */
void perturbateInterchange (long int *lobest, long int *lo, long long int *length) {

    long int j, n1, n2, min, max, k, w, wp;
    long long int g;
    /* computing the cost contributions removed */
    for ( j = 0; j < PSize; j++ )
	lo[j] = lobest[j];


    for (k = 0; k < NumSwaps; k++) {
	n1 = (int)(ran01(&Seed)*PSize); 
	if ( n1 >= PSize ) n1 = PSize - 1;

	/* Possible window of choice for values > n1 */
	w = ((PSize-n1)<MaxPertLength)?(PSize-n1):MaxPertLength;
	/* Possible window of choice for values < n1 */
	wp = (n1<MaxPertLength)?n1:MaxPertLength;

	/* if MaxPertLength == PSize => w == PSize */

	/* printf("With [%d] n1: %d w: %d [%d]\n", k, n1, w, MaxPertLength); */


	do {
	    n2 = n1 - wp + (int)(ran01(&Seed)*(w+wp));
	    if ( n2 >= PSize ) n2 = PSize - 1;	    
	} while ( n1 == n2 );

	/* printf("Got %d\n", n2); */

	min = (n1 > n2)?n2:n1;
	max = (n1 > n2)?n1:n2;

#ifdef VERBOSE_DEBUG
	printf("Swapping [%d]: %d <-> %d.\n", k, n1, n2);
#endif
	for ( g = 0, j = min + 1; j <= max; j++) 
	    g += CostMat[lo[j]][lo[min]] - CostMat[lo[min]][lo[j]];
	
	for ( j = min + 1; j < max; j++) 
	    g += CostMat[lo[max]][lo[j]] - CostMat[lo[j]][lo[max]];
	
	*length += g;

	j = lo[min];
	lo[min] = lo[max];
	lo[max] = j;
    }
}

/* The perturbation is done reverting a random central segment of the order */
void perturbateReverse (long int *lobest, long int *lo, long long int *length) {
    long int j, n1, g, n2, min, max, l, i;

    /* computing the cost contributions removed */
    
    n1 = (int)(ran01(&Seed)*PSize); 
    if ( n1 >= PSize ) n1 = PSize - 1;

    l = (PSize - n1 - 2);
    l = (l < MAXREV)?l:MAXREV;

    n2 = (int)(ran01(&Seed)*l);
    n2 += n1 + 2; /*not adjacent positions */
    
    if ( n2 >= PSize ) {
	n2 = PSize - 1;
	n1 = PSize - 3;
    }

    min = (n1 > n2)?n2:n1;
    max = (n1 > n2)?n1:n2;


#ifdef VERBOSE_DEBUG
    printf("Reverse %d <-> %d.\n", n1, n2);
#endif

    for ( j = 0; j < min; j++ )
	lo[j] = lobest[j];

    for ( g = 0, j = min, i = max; j <= max; j++, i-- ) {
	for ( l = min; l <= j; l++ ) 
	    g += CostMat[lobest[j]][lobest[l]] -
		CostMat[lobest[l]][lobest[j]];
	lo[j] = lobest[i];
    }
    for (j = max + 1; j < PSize; j++) 
	lo[j] = lobest[j];

    /* CurrentIdx = n2;*/
    *length += g;
}


/* The perturbation is done inserting the first element in a random position */
void perturbateInsert1 (long int *lobest, long int *lo, long long int *length) {
    long int j, n, g;

    /* computing the cost contributions removed */
    
    n = (int)(ran01(&Seed)*(PSize - 1)) + 1;


    if ( n >= PSize ) n = PSize - 1;


    for (g = 0, j = 1; j < n; j++) {
	g += CostMat[lobest[j]][lobest[0]] - CostMat[lobest[0]][lobest[j]];
	lo[j-1] = lobest[j];
    }
    
    lo[n-1] = lobest[0];
    
    for (j = n; j < PSize; j++) 
	lo[j] = lobest[j];

    *length += g;
}



heap_el_t *createHeap(int hl) {
    heap_el_t *h;
    int i;

    h = (heap_el_t *)malloc((hl+1)*sizeof(heap_el_t));
    if ( !h ) fatal("createHeap: Out of memory allocating heap.");
    for (i = 0; i <= hl; i++) h[i].hp_node = -1;
    return(h);
}

int insertHeap(heap_el_t *h,  heap_el_t *el, int hl) {
  int idx, p, r, ls, rs, c;
  heap_el_t n;

  for (idx = 0; (idx < hl) && (h[idx].hp_node >= 0); idx++);
  /* idx is the index of a free cell,
     possibly the last one (always free) */
  /* If there is no free place, and the el is too far, 
     the el is not inserted */
  if ( ( idx == hl ) && 
       (h[0].hp_value <= el->hp_value) ) 
    return(-1);

  if ( idx < hl ) {
    h[idx].hp_node = el->hp_node;
    h[idx].hp_value = el->hp_value;
    
    for(r = 0; (idx >= 0) && (r == 0); idx = (idx - 1)/2) {
      p = (idx - 1) / 2;
      if ( h[p].hp_value < h[idx].hp_value ) {
	n = h[idx];
	h[idx] = h[p];
	h[p] = n;
      } else 
	r = p;
      /* because (0 - 1) / 2 == 0 */
      if (idx == 0) idx--;
    }
    /* if we always swapped (never set r), then the new el will be on the
       root, hence r = 0; */
    
    h[hl].hp_node = -1;
  } else {
    h[0].hp_node = el->hp_node;
    h[0].hp_value = el->hp_value; 
    r = -1;
    for (idx = 0; ( (2 * idx + 1) < hl ) && (r < 0); ) {
      ls = 2 * idx + 1;
      rs = 2 * idx + 2;
      if ( ( rs >= hl ) ) 
	c = ls;
      else 
	c = (h[ls].hp_value > h[rs].hp_value)?ls:rs;
      /* The element in idx is not in the right place */
      if ( h[c].hp_value > h[idx].hp_value ) {
	n = h[idx];
	h[idx] = h[c];
	h[c] = n;
	idx = c;
      } else 
	r = idx;
    }
    if (r < 0) r = idx;
  }
  
  return r;
}

int extractHeap(heap_el_t *h, int hl) {
  int r, ls, rs, c, i;

  r = h[0].hp_node;
  for (i = 0; ((2 * i + 1) < hl) && (h[i].hp_node >= 0); ) {
    ls = 2 * i + 1;
    rs = 2 * i + 2;
    if ( ( rs >= hl ) ) 
      c = ls;
    else 
      c = (h[ls].hp_value > h[rs].hp_value)?ls:rs;
    h[i] = h[c];
    i = c;
  }

  h[i].hp_node = -1;
  h[i].hp_value = -1;
    
  return(r);
}



void beckersGreedy(long int *lo) {
    heap_el_t he, *h;
    int i, j;
    float ftmp1, ftmp2;

    h = createHeap(PSize);

    for (i = 0; i < PSize; i++) {
	he.hp_node = i;
	for ( j = 0, ftmp1 = 0, ftmp2 = 0; j < PSize; j++ ) {
	    ftmp1 += CostMat[i][j];
	    ftmp2 += CostMat[j][i];
	}
	if ( ftmp2 != 0) 
	    he.hp_value = (float )ftmp1 / (float )ftmp2;
	else /* What happens with a 0's column ? */
	    he.hp_value = MAX_FLOAT;
	insertHeap(h, &he, PSize);
    }
    
    for (i = 0; i < PSize; i++) 
	lo[i] = extractHeap(h, PSize);

    free(h);
}

void initialSolution(long int *lo) {
    int j; 
    long int *random;
   // Greedy=1;
    if ( Greedy )
        beckersGreedy(lo);
    
   // else if (GuidedUbiquityInitialization)
   //     CalculateGuidedUbiquityRandomSolution(lo);
    else{
        random = generate_random_vector(PSize);
        for ( j = 0 ; j < PSize ; j++ ) {
            lo[j] = random[j];
        }
        free ( random );
    }
   // printf("cost %lld\n",computeCost(lo));exit(-1);
    
}

void perturbate (long int *lobest, long int *lo, long long int *length) {
    switch ( PerturbationType ) {
	case 'x': 
	    perturbateInterchange( lobest, lo, length );
	    break;
	case 'r': 
	    perturbateReverse( lobest, lo, length );
	    break;
	case '1':
	    perturbateInsert1( lobest, lo, length );
	    break;
	case 'i':
	    perturbateInsert( lobest, lo, length );
	    break;
	case 'm':
	    
	default:
	    fatal("Unknown kind of perturbation.\n");
    }
}

void addLocOptFunction ( char f ) {
    static int cur = 0;

    switch (f) {
	case 'c':
	    LocOptArray[cur] = CKLocalSearch;
	    break;
	case 'x':
	    LocOptArray[cur] = localOptInterchange;
	    break;
	case 'X':
	    LocOptArray[cur] = localOptInterchangeBI;
	    break;
	case 's':
	    LocOptArray[cur] = localOptSwap;
	    break;
	case 'f':
	    LocOptArray[cur] = localOptFastInsert;
	    break;
  	case 'F':
        LocOptArray[cur] = localOptFastInsertUbiquity;
        break;
	case 'i':
	    LocOptArray[cur] = localOptInsert;
	    break;
	case 'I':
	    LocOptArray[cur] = localOptInsertBI;
	    break;
	default:
	    fatal("addLocOptFunction: Unknown Local Search type.");
    }
    /*for (i = 0; i < cur; i++) 
	if ( LocOptArray[i] == LocOptArray[cur] ) 
	    fatal("addLocOptFunction: Repeated Local Search type");
    */			   
    cur++;
}
    
    /*
     * Sorts the ints array in the descending sequence. Returns the values in descending order.
     */
    void quick_sort(long * arr,int low,int high)
    {
        long pivot,temp;
        int j,i;
        if(low<high)
        {
            pivot = low;
            i = low;
            j = high;
            
            while(i<j)
            {
                while((arr[i]>=arr[pivot])&&(i<high))
                {
                    i++;
                }
                
                while(arr[j]<arr[pivot])
                {
                    j--;
                }
                
                if(i<j)
                { 
                    temp=arr[i];
                    arr[i]=arr[j];
                    arr[j]=temp;
                }
            }
            
            temp=arr[pivot];
            arr[pivot]=arr[j];
            arr[j]=temp;
            quick_sort(arr,low,j-1);
            quick_sort(arr,j+1,high);
        }
    }

    /*
     * Analyzes the ubiquity of the indices, and it calculates a binary matrix.
     */
    void CalculateUbiquityMatrix(){
        
        m_restrictionBounds= (int*)malloc(PSize*sizeof(int));

        long * differencesArray= ( long *)malloc((PSize-1) * sizeof(long));
        int item=0;
        int beforeSum, afterSum;
        int min,max;
        int k,i;
        int restriction;
        for (item=0;item<PSize;item++)
        {
            //obtain the array of differences.
            for (i=0;i<item;i++)
                differencesArray[i]=-DiffMat[item][i];
            for (i=item+1;i<PSize;i++)
                differencesArray[i-1]=-DiffMat[item][i];
        
            //quick sort algorithm
            quick_sort(differencesArray, 0, (int)PSize-2);

            
        restriction=MIN_INTEGER;
        
        for (min=0;min<PSize;min++)
        {
            beforeSum=0;
            for (k=0;k<min;k++)
                beforeSum+=differencesArray[k];
            
            afterSum=0;
            for (k=min;k<PSize-1;k++)
                afterSum+=differencesArray[k];
            
            if (beforeSum>=0 && afterSum <=0)
            {
                restriction=min;
                break;
            }
        }
            if (restriction==0){
            
        for (max=PSize-1;max>=0;max--)
        {
            beforeSum=0;
            for (k=0;k<max;k++)
                beforeSum+=differencesArray[k];
            
            afterSum=0;
            for (k=max;k<PSize-1;k++)
                afterSum+=differencesArray[k];
            if (beforeSum>=0 && afterSum <=0)
            {
                restriction=max-(PSize-1);
                break;
            }
        }
            }
              m_restrictionBounds[item]=restriction;
        }
        //free(differencesArray);
    }

