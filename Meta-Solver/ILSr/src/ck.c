/************************************************************************/
/* Authors: Stefan Chanas and Przemyslaw Kobylanski    (Poland)		*/
/*   --------------------------------------------------   		*/
/* "A new heuristic Algorithm solving the linear ordering problem"	*/     
/* in Computational Optimization and Applications 6, (1996) 191-205     */
/************************************************************************/

/************************************************************************/
/* Modified by Tommaso Schiavinotto in 2004 for inclusion in other      */
/* metaheuristics							*/
/************************************************************************/

/************************************************************************/
/*	Input for the programm						*/
			
/*	TITLE								*/
/*	m								*/
/*	e11 ... e1m							*/
/*	...								*/
/*	em1 ... emm							*/
/************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include"../include/utilities.h"
#include"../include/lop.h"
#include"../include/ils_lop.h"
#include"../inlcude/instance.h"

static int TotalSum, StartCost;

long long int reverse (long int *p, long long int l)
{
 long int i;
 long long temp;
 
 for(i=0;i<(PSize / 2);i++)
  {
   temp=p[i];
   p[i]=p[PSize-1-i];
   p[PSize-1-i]=temp;
  }
 return(TotalSum-l);;
}

long int sort (long int *p)
{
 long int i, k, bestPosition;
 long long int temp, currCost;
 long long int sum, bestChange;

 currCost=0;
 for(i=1;i<PSize;i++)
  {
   sum = 0;
   for(k=0;k<i;k++)
     sum+=CostMat[p[k]][p[i]];
   bestPosition = i;
   bestChange = sum;
   for( k = i - 1; k >= 0 ; k--)
     {
      sum += CostMat[p[i]][p[k]] - CostMat[p[k]][p[i]];
      if (sum > bestChange)
       {
         bestPosition = k;
         bestChange = sum;
       }
     }
   if (bestPosition < i)
     {
      temp = p[i];
      for(k=i-1;k>=bestPosition;k--)
        p[k+1] = p[k];
      p[bestPosition] = temp;
     }
   currCost += bestChange;
 }
 return(currCost);
}

void sortIter (long int *p, long long int *l)
{
 long long int lastCost;

 do
 {
   lastCost=*l;
   *l = sort(p);
 } while(*l > lastCost);
}

void CalculateTotSum () {
    int i,k;

    TotalSum=0;
    for(i=0;i<PSize-1;i++)
	for(k=i+1;k<PSize;k++)
	    TotalSum += CostMat[i][k]+CostMat[k][i];
}

void CKPrepareSolution(long int *lo, long long int *l) {
    sortIter(lo, l);
    StartCost = *l;
}

long int CKLocalSearch(long int *lo) 
{
    long long int l, r;

    /* StartCost comes from the CKPrepareSolution or the previous cycle */
    l = reverse(lo, StartCost);
    sortIter(lo, &l);
    r = l - StartCost;
    StartCost = l;
    return(r);
}




















