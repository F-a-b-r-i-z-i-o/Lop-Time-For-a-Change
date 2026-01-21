//UTILITY FUNCTIONS TO INCLUDE IN PSOLUTION.CPP
#include <cstdlib> //for qsort
#include "random.h"
#include <math.h>

//add item j to list l with length ln and where li is the inversion of l
inline void addItemToList(int j, int* l, int* li, int& ln) {
	//do not check for free space in l
	l[ln] = j;
	li[j] = ln;
	ln++;
}



//remove item j from list l with length ln and where li is the inversion of l
inline void remItemFromList(int j, int* l, int* li, int& ln,int n) {
	//do not check that j is in l (i.e., k==-1)
	int k = li[j];		//where was j (useless if j is the only item in the list)
	li[j] = -1;			//j is no more in the list
	ln--;				//decrease list length
	if (ln>0 && k!=ln) {//list is non-empty and last item need to be moved
		l[k] = l[ln];	//move last item in place of j
		int t = l[k];	//this is the moved item
		li[t] = li[t]>=n ? k+n : k; //new index of the moved item (accounting for stacked items)
	}
}



//check if item j is in the inversion list li
inline bool isItemInList(int j, int* li) {
	return li[j]!=-1;
}



//mark precedence i<j as stacked by adding n to ui[i][j] (do not check that ui[i][j]!=-1)
inline void markPrecAsStacked(int j, int* ui, int n) {
	ui[j] += n; //assuming ui[j] was >=0
}



//rollback markPrecAsStacked
inline void unmarkPrecAsStacked(int j, int* ui, int n) {
	ui[j] -= n;
}



//check if precedence i<j is stacked
inline bool isPrecStacked(int j, int* ui, int n) {
	return ui[j]>=n;
}



//get (i,j) upper-triangular indexes given k unrolled index of a nxn matrix
inline void triuIndices(int& i, int& j, int k, int n) {
	//see https://stackoverflow.com/questions/27086195/linear-index-upper-triangular-matrix
	i = n - 2 - (int)(sqrt(-8*k + 4*n*(n-1) - 7)/2.0 - 0.5);
	j = k + i + 1 - n*(n-1)/2 + (n-i)*(n-i-1)/2;
}



//set up lists for scanning items b such that a<b<c
//after calling this function do: "for (k=0; k<l1n; k++) { b = l1[k]; if (isItemInList(b,l2i)) { DO_SOMETHING } }"
inline void listsForMiddleItemsScan(int*& l1, int& l1n, int*& l2i, int* sa, int san, int* sai, int* pc, int pcn, int* pci) {
	if (san<pcn) {
		l1 = sa;
		l1n = san;
		l2i = pci;
	} else {
		l1 = pc;
		l1n = pcn;
		l2i = sai;
	}
}






