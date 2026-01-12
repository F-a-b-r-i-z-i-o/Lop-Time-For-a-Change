#include <cmath>
#include "../include/PSolution.h"
#include <cstring>
#include <iostream>

using namespace std;



#define EPS            (1e-3)
#define PLUS_INF    (+1./0.)
#define MINUS_INF    (-1./0.)



int PSolution::n;
long int PSolution::nc2;
long int PSolution::n2;
int* PSolution::lop = 0;
double* PSolution::v = 0;
PrecWithValue* PSolution::lopSorted = 0;
int* PSolution::invLopSorted = 0;
int* PSolution::wmem = 0;
PrecWithProb* PSolution::precs = 0;


//constructor: only allocate memory considering n items
PSolution::PSolution(int n) {
    //everything stored in data with this order: sn-un-pn - s[0]-si[0]-u[0]-ui[0]-p[0]-pi[0] - .... - s[n-1]-si[n-1]-u[n-1]-ui[n-1]-p[n-1]-pi[n-1]
    this->n = n;
    nc2 = n*(n-1)/2;
    n2 = n*n;
    data = new int[3*n+n*(3*n+3*(n-1))]; //3*n = u/s/p-n ... 3*n = u/s/p-i ... 3*(n-1) = u/s/p
    s = new int*[n];
    si = new int*[n];
    u = new int*[n];
    ui = new int*[n];
    p = new int*[n];
    pi = new int*[n];
    sn = data;
    un = sn+n;
    pn = un+n;
    for (int i=0; i<n; i++) {
        s[i]  = data + (3*n+i*(3*n+3*(n-1)));
        si[i] = s[i]+(n-1);
        u[i]  = si[i]+n;
        ui[i] = u[i]+(n-1);
        p[i]  = ui[i]+n;
        pi[i] = p[i]+(n-1);
        ui[i][i] = si[i][i] = pi[i][i] = -1; //item i is always in none of its lists
    }
    uvsum = new long double[n];
    //allocate working memory if the case
    if (!wmem) wmem = new int[2*n2]; //2*n^2 needed in construct/propagate, nc2 needed in destruct, but 2*n^2 is always greater than nc2
    //at beginning, vsum and vmax are not valid
    //vsum_valid = vmax_valid = false;
    vsum_valid = false;
}



//destructor
PSolution::~PSolution() {
    delete[] data;
    delete[] s;
    delete[] si;
    delete[] u;
    delete[] ui;
    delete[] p;
    delete[] pi;
    delete[] uvsum;
    //do not free working memory here
}


//to use in qsort inside setLOPMatrix, it sorts in DESCENDING ORDER!!!
int precsValueComparator(const void* prec1, const void* prec2) {
    PrecWithValue* castedPrec1 = (PrecWithValue*)prec1;
    PrecWithValue* castedPrec2 = (PrecWithValue*)prec2;
    if (castedPrec1->vint < castedPrec2->vint)
        return 1; //DESCENDING ORDER
    else if (castedPrec1->vint > castedPrec2->vint)
        return -1; //DESCENDING ORDER
    else //castedPrec1->vint == castedPrec2->vint
        return irand(2) ? 1 : -1; //RANDOMLY IF THEY ARE EQUAL
}


//setup lop matrix and compute precedence values from it (using 1d matrices)
void PSolution::setLOPMatrix(int* lop1, int n) {
    int i,j,k;
    long int n2 = n*n;
    lop = lop1;
    if (!v) v = new double[n2];
    for (k=0; k<n2; k++) v[k] = lop1[k] + EPS;
    if (!lopSorted) lopSorted = new PrecWithValue[n2-n];
    k = 0;
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            if (i==j) continue;
            lopSorted[k].i = i;
            lopSorted[k].j = j;
            lopSorted[k++].vint = lop[i*n+j];
        }
    }
    qsort(lopSorted,n2-n,sizeof(PrecWithValue),precsValueComparator);
    if (!invLopSorted) invLopSorted = new int[n2];
    for (k=0; k<n2-n; k++) invLopSorted[n*lopSorted[k].i+lopSorted[k].j] = k;
    long int aux=n*(n-1)/2;
    if (!precs) precs = new PrecWithProb[aux]; //nc2
}



//free static memory
void PSolution::freeStaticMemory() {
    if (lop) delete[] lop;
    if (v) delete[] v;
    if (lopSorted) delete[] lopSorted;
    if (invLopSorted) delete[] invLopSorted;
    if (wmem) delete[] wmem;
    if (precs) delete[] precs;
}



//set the solution to an empty one (no precedence set)
void PSolution::makeEmpty() {
    int i,j,k;
    for (i=0; i<n; i++) {
        //s and p lists are all empty
        sn[i] = pn[i] = 0;
        for (j=0; j<n; j++)
            si[i][j] = pi[i][j] = -1;
        //u list is set to identity-except-i
        un[i] = n-1;
        k = 0;
        for (j=0; j<n; j++) {
            if (j!=i) {
                u[i][k] = j;
                ui[i][j] = k;
                k++;
            }
        }
    }
    np = 0;
    fit = 0;
    //vsum_valid = vmax_valid = false;
    vsum_valid = false;
    //rem_max_val = -1; //it is like -inf
    pmax = 0;
}



//check if the solution is a full permutation (total order)
bool PSolution::isTotal() {
    return np==nc2;
}



//evaluate this partial solution (set and return the fitness)
unsigned long PSolution::eval() {
    int i,j,k,t,in;
    fit = 0;
    for (i=0; i<n; i++) {
        t = sn[i];
        in = i*n;
        for (k=0; k<t; k++) {
            j = s[i][k];    //precedence i<j is considered
            fit += lop[in+j];
        }
    }
    return fit;
}



//set basing on the n-length permutation x
void PSolution::fromPermutation(int* x, int fit) {
    int i,j,ii,jj;
    makeEmpty(); //first step, make it empty
    for (ii=0; ii<n; ii++) {
        i = x[ii];
        for (jj=ii+1; jj<n; jj++) {
            j = x[jj];                                //the considered precedence is i<j
            remItemFromList(j,u[i],ui[i],un[i],n);    //remove item j from the u-list of item i
            addItemToList(j,s[i],si[i],sn[i]);        //add    item j to   the s-list of item i
            remItemFromList(i,u[j],ui[j],un[j],n);    //remove item i from the u-list of item j
            addItemToList(i,p[j],pi[j],pn[j]);        //add    item i to   the p-list of item j
        }
    }
    np = nc2;
    if (fit>=0) this->fit = fit;
    else eval();
    //vsum_valid = vmax_valid = false;
    vsum_valid = false;
    //rem_max_val = -1; //it is like -inf
    pmax = 0;
}



//convert to an n-length permutation x in output
void PSolution::toPermutation(int* x) {
    //do not check there is the right number of precedences (i.e., np==nc2)
    //item i goes in the position #items-preceding-i
    for (int i=0; i<n; i++) x[pn[i]] = i;
}



//construct a complete solution starting from this and using q as probability to make greedy moves
void PSolution::construct(double q) {
    //some variables
    int i,j,k,ii,jj,t,in,ilast,jlast;
    long double r,acc;
    //initialize vsum and vmax (the check if valid or not is made inside the function)
    //compute_vsum_vmax();
    compute_vsum();
    //pmax = rem_max_val<0 ? 0 : invLopSorted[rem_max_i*n+rem_max_j];
    //pmax = 0;
    //main loop until there are n*(n-1)/2 precedences set
    while (np<nc2) {
        //choose precedence i<j, basing on q: in a greedy manner or using roulette wheel
        if (urand()<q) { //greedy
            //greedy choice
            //compute_vmax(); //inside there is a check if it is the case or not
            //i = imax;
            //j = jmax;
            while (true) {
                i = lopSorted[pmax].i;
                j = lopSorted[pmax].j;
                if (isItemInList(i,ui[j])) //prec i<j or j<i is available
                    break;
                pmax++;
            }
            //greedy choice done
        } else { //roulette wheel
            //roulette wheel choice
            //compute_vsum(); //no need, because it is computed before the while, and construct code guarantees vsum remains valid
            i = j = ilast = jlast = -1;
            r = urand()*vsum;
            acc = 0.;
            for (ii=0; ii<n; ii++) {
                acc += uvsum[ii];
                if (r<acc) {
                    i = ii;
                    break;
                }
                if (un[ii]>0) ilast = ii; //avoid numerical precision problems
            }
            if (i==-1) i = ilast; //may rarely occur due to numerical precision problems
            in = i*n;
            t = un[i];
            r = urand()*uvsum[i];
            acc = 0.;
            for (k=0; k<t; k++) {
                jj = u[i][k];
                acc += v[in+jj];
                if (r<acc) {
                    j = jj;
                    break;
                }
                jlast = jj;
            }
            if (j==-1) j = jlast; //may rarely occur due to numerical precision problems
            //roulette wheel done
        }
        //set i<j and its induced precedences using propagate (internally, it also update np and fit)
        propagate(i,j);
    } //end main while loop
    //reset rem_max_val to -inf
    //rem_max_val = -1; //-1 is like -inf
    pmax = n2;
    //done
}



//set i<j and its induced precedences, it also updates np,fit and vsum_valid,vmax_valid
void PSolution::propagate(int i, int j) {
    //it uses st as a stack of precedences such that: 0-th prec is st[0]<st[1], 1-th prec is st[2]<st[3], ..., k-th prec is st[2*k]<st[2*k+1]
    //I use a stack (with a control that no two equal precedences go into the stack at any time), but any set data structure is fine
    int* st = wmem;    //adopt working memory (needs 2*n^2 entries at worse)
    int nst,k,t,w;    //nst is the number of precedences in the stack
    //push i<j into the stack
    st[0] = i;
    st[1] = j;
    nst = 1;
    markPrecAsStacked(i,ui[j],n);
    //propogation loop (until the stack becomes empty)
    while (nst) {
        //pop precedence i<j from the stack
        nst--;
        w = 2*nst;
        i = st[w];
        j = st[w+1];
        unmarkPrecAsStacked(i,ui[j],n);
        //in this partial solution: set precedence i<j
        remItemFromList(j,u[i],ui[i],un[i],n);
        addItemToList(j,s[i],si[i],sn[i]);
        remItemFromList(i,u[j],ui[j],un[j],n);
        addItemToList(i,p[j],pi[j],pn[j]);
        //update np and fit
        np++;
        fit += lop[i*n+j];
        //update vsum, uvsum, and vmax validity
        vsum -= v[i*n+j] + v[j*n+i]; //decrease vsum because both i<j and j<i are no more available (so vsum, continue to be valid)
        if (vsum<0.) vsum = 0.;
        uvsum[i] -= v[i*n+j]; //decrease rowsum of item i
        if (uvsum[i]<0.) uvsum[i] = 0.;
        uvsum[j] -= v[j*n+i]; //decrease rowsum of item j
        if (uvsum[j]<0.) uvsum[j] = 0.;
        //if (v[i*n+j]==vmax || v[j*n+i]==vmax) vmax_valid = false;
        //push into the stack the precedences of the types: (1) k<j with k<i, and (2) i<k with j<k
        //type (1) k<j because it already exists k<i
        for (t=0; t<pn[i]; t++) {
            k = p[i][t]; //k is such that k<i
            if (isItemInList(k,ui[j]) && !isPrecStacked(k,ui[j],n)) { //neither k<j nor j<k && k<j is not in the stack
                w = 2*nst;
                st[w] = k;
                st[w+1] = j;
                nst++;
                markPrecAsStacked(k,ui[j],n);
            }
        } //end-for (1)
        //type (2) i<k because it already exists j<k
        for (t=0; t<sn[j]; t++) {
            k = s[j][t]; //k is such that j<k
            if (isItemInList(i,ui[k]) && !isPrecStacked(i,ui[k],n)) { //neither i<k nor k<i && i<k is not in the stack
                w = 2*nst;
                st[w] = i;
                st[w+1] = k;
                nst++;
                markPrecAsStacked(i,ui[k],n);
            }
        } //end-for (2)
    } //end propagation while loop
    //done
}


//r is the percentage of (randomly chosen) precedences to remove, returns number of precedences removed
long int PSolution::destruct(double r) {
    //r has to be in [0,1] but no check is performed
    return destruct( (long int)(r*nc2) );
}



//remove (approximately) npr precedences chosen randomly, returns the number of precedences removed
long int PSolution::destruct(long int npr) {
    //some variables
    int i,j,k,a,c,aa,cc;
    long int removedPrecs,bestExcess,excess,precsToRemove;
    int* rp = wmem;    //adopt working memory (needs nc2 entries)
    //npr is the number of precedences to remove, but cannot remove more than nc2 preferences
    if (npr>nc2) npr = nc2;
    //generate a random permutation of "unrolled triu indices" with length nc2
    prand(nc2,rp);
    //initialize removedPrecs count and the "unrolled triu index" k to 0
    removedPrecs = k = 0;
    //initialize fallback variables to a non-sense status
    aa = cc = -1;
    bestExcess = n; //it is a strict upper-bound for the excess
    //remove (almost always) npr precedences following the order in the random permutation
    while (removedPrecs<npr) {
        //very last iteration or not
        if (k<nc2) { //not the very last iteration
            //convert the "unrolled triu index" rp[k] to (i,j) and increase k for next iteration
            triuIndices(i,j,rp[k++],n);
            //set a<c to i<j or j<i or, if neither of them, discard them and goto next while step
            if (isItemInList(i,pi[j])) { //it is i<j
                a = i;
                c = j;
            } else if (isItemInList(j,pi[i])) { //it is j<i
                a = j;
                c = i;
            } else //it is neither i<j nor j<i
                continue;
            //predict number of precedences removed by removing a<c and compute its excess
            precsToRemove = countPrecsToRemove(a,c);
            excess = precsToRemove+removedPrecs - npr;
            //check if there is excess or not
            if (excess<=0) {
                //no excess, so reset excess (if set) and remove a<c (and its induced precedences)
                aa = cc = -1;
                bestExcess = n;
                removedPrecs += remPrecs(a,c);
            } else if (excess<bestExcess) { //&& excess>0
                //there is excess and it is better than previous best excess, so update it
                aa = a;
                cc = c;
                bestExcess = excess;
            }
            //done the not very last iteration
        } else { //k==nc2, so very last iteration
            //if there is a fallback precedence to remove, then remove it
            if (aa!=-1) //&& cc!=-1 && bestExcess<n && k==nc2
                removedPrecs += remPrecs(aa,cc);
            //exit main while: no need if there was a fallback precedence, but the break avoid infinite loop also in the very unlucky case when the removed precedences are less than NPR
            break;
            //done the very last iteration
        }
    } //end main while
    //unvalidate vsum and vmax
    //vsum_valid = vmax_valid = false;
    vsum_valid = false;
    //return number of removedPrecs
    return removedPrecs;
}



//predict the total number of precedences to remove when a<c is removed
int PSolution::countPrecsToRemove(int a, int c) {
    //I assume a<c is set without checking
    int res,b,*l1,l1n,*l2i,k;
    res = 1;
    listsForMiddleItemsScan(l1,l1n,l2i,s[a],sn[a],si[a],p[c],pn[c],pi[c]);
    for (k=0; k<l1n; k++) {
        b = l1[k];
        if (isItemInList(b,l2i)) res++;
    }
    return res;
}



//remove prececedence a<c and its induced precedences, and return the number of precedences removed
int PSolution::remPrecs(int a, int c) {
    //some variables
    int removedPrecs,b,rnd,*l1,l1n,*l2i,k;
    //remove precedence a<c
    remItemFromList(c,s[a],si[a],sn[a],n);
    addItemToList(c,u[a],ui[a],un[a]);
    remItemFromList(a,p[c],pi[c],pn[c],n);
    addItemToList(a,u[c],ui[c],un[c]);
    //update pmax using a, c, lop[a,c]
    if (invLopSorted[a*n+c]<pmax) pmax = invLopSorted[a*n+c];
    if (invLopSorted[c*n+a]<pmax) pmax = invLopSorted[c*n+a];
    //init removedPrecs to 1
    removedPrecs = 1;
    //update fit, decrease np
    fit -= lop[a*n+c];
    np--;
    //choose the "remove direction" as a random number in {0,1}
    rnd = irand(2);
    //scan all the items b such that a<b and b<c
    listsForMiddleItemsScan(l1,l1n,l2i,s[a],sn[a],si[a],p[c],pn[c],pi[c]);
    for (k=0; k<l1n; k++) {
        b = l1[k];
        if (!isItemInList(b,l2i)) continue;
        //basing on rnd remove the precedence a<b or b<c
        if (rnd==0) {
            //remove precedence a<b
            remItemFromList(b,s[a],si[a],sn[a],n);
            addItemToList(b,u[a],ui[a],un[a]);
            remItemFromList(a,p[b],pi[b],pn[b],n);
            addItemToList(a,u[b],ui[b],un[b]);
            //update pmax using a, b, lop[a,b]
            if (invLopSorted[a*n+b]<pmax) pmax = invLopSorted[a*n+b];
            if (invLopSorted[b*n+a]<pmax) pmax = invLopSorted[b*n+a];
            //update fit
            fit -= lop[a*n+b];
        } else { //rnd==1
            //remove precedence b<c
            remItemFromList(c,s[b],si[b],sn[b],n);
            addItemToList(c,u[b],ui[b],un[b]);
            remItemFromList(b,p[c],pi[c],pn[c],n);
            addItemToList(b,u[c],ui[c],un[c]);
            //update pmax using b, c, lop[b,c]
            if (invLopSorted[b*n+c]<pmax) pmax = invLopSorted[b*n+c];
            if (invLopSorted[c*n+b]<pmax) pmax = invLopSorted[c*n+b];
            //update fit
            fit -= lop[b*n+c];
        }
        //decrease np and increase removedPrecs
        np--;
        removedPrecs++;
    } //end-for
    //return number of removed precs
    return removedPrecs;
}



//destroy all precedences (+induced ones) with probability > alpha
int PSolution::destruct_threshold(long** countersMatrix, double alpha) {
    //some variables
    int removedPrecs,ii,jj,i,j;
    double p;
    //init the number of removed precedences
    removedPrecs = 0;
    //scan all precedences i<j
    for (ii=0; ii<n-1; ii++) {
        for (jj=ii+1; jj<n; jj++) {
            //set i<j
            if (isItemInList(ii,ui[jj])) //neither ii<jj nor jj<ii
                continue;
            else if (isItemInList(ii,pi[jj])) { //ii<jj
                i = ii;
                j = jj;
            } else { //jj<ii
                i = jj;
                j = ii;
            }
            //compute p(i<j) basing on countersMatrix
            p = countersMatrix[i][j] / (double)(countersMatrix[i][j]+countersMatrix[j][i]);
            //if p(i<j) > alpha, remove i<j + induced precedences
            if (p>alpha)
                removedPrecs += remPrecs(i,j);
        }
    } //end precedences scan
    //unvalidate vsum and vmax
    //vsum_valid = vmax_valid = false;
    vsum_valid = false;
    //return the number of removed precedences
    return removedPrecs;
}



//destroy all precedences (+induced ones) with probability given by countersMatrix
int PSolution::destruct_stochastic(long** countersMatrix) {
    //some variables
    int removedPrecs,ii,jj,i,j;
    //init the number of removed precedences
    removedPrecs = 0;
    //scan all precedences i<j
    for (ii=0; ii<n-1; ii++) {
        for (jj=ii+1; jj<n; jj++) {
            //set i<j
            if (isItemInList(ii,ui[jj])) //neither ii<jj nor jj<ii
                continue;
            else if (isItemInList(ii,pi[jj])) { //ii<jj
                i = ii;
                j = jj;
            } else { //jj<ii
                i = jj;
                j = ii;
            }
            //with probability p(i<j), remove i<j + induced precedences
            if (irand(countersMatrix[i][j]+countersMatrix[j][i]) < countersMatrix[i][j]) //it is the same of "urand() < countersMatrix[i][j] / (double)(countersMatrix[i][j]+countersMatrix[j][i])"
                removedPrecs += remPrecs(i,j);
        }
    } //end precedences scan
    //unvalidate vsum and vmax
    //vsum_valid = vmax_valid = false;
    vsum_valid = false;
    //return the number of removed precedences
    return removedPrecs;
}



//sorted version of destruct
long int PSolution::destruct_sorted(long** countersMatrix, double r) {
    //r has to be in [0,1] but no check is performed
    return destruct_sorted(countersMatrix,(long int)(r*nc2));
}


//to use in qsort inside computeSortedPrecsList, it sorts in DESCENDING ORDER!!!
int precsProbComparator(const void* prec1, const void* prec2) {
    PrecWithProb* castedPrec1 = (PrecWithProb*)prec1;
    PrecWithProb* castedPrec2 = (PrecWithProb*)prec2;
    if (castedPrec1->p < castedPrec2->p)
        return 1; //DESCENDING ORDER
    else if (castedPrec1->p > castedPrec2->p)
        return -1; //DESCENDING ORDER
    else //castedPrec1->p == castedPrec2->p
        return irand(2) ? 1 : -1; //RANDOMLY IF THEY ARE EQUAL
}

//compute list of precedences in sol ordered by their probabilities obtained from countersMatrix
void computeSortedPrecsList(PrecWithProb* precs, long int& precsSize, PSolution* sol, long** countersMatrix) {
    int ii,jj,i,j,k,n;
    n = sol->n;
    k = 0;
    for (ii=0; ii<n-1; ii++) {
        for (jj=ii+1; jj<n; jj++) {
            if (isItemInList(ii,sol->pi[jj])) { //ii<jj
                i = ii;
                j = jj;
            } else if (isItemInList(ii,sol->si[jj])) { //jj<ii
                i = jj;
                j = ii;
            } else //neither ii<jj nor jj<ii
                continue;
            precs[k].i = i;
            precs[k].j = j;
            precs[k].p = countersMatrix[i][j] / (double)(countersMatrix[i][j]+countersMatrix[j][i]);
            k++;
        }
    }
    precsSize = k;
    qsort(precs,precsSize,sizeof(PrecWithProb),precsProbComparator);
}


long int PSolution::destruct_sorted(long** countersMatrix, long int npr) {
    //some variables
    long int precsSize,removedPrecs,bestExcess,precsToRemove,excess;
    int i,j,k,ii,jj;
    //PrecWithProb precs[nc2]; //too large to stay into stack memory area
    //compute sorted precedences-list basing on this partial(full) solution and countersMatrix
    computeSortedPrecsList(precs,precsSize,this,countersMatrix);
    //npr is the number of precedences to remove, but cannot remove more than precsSize (==nc2 for full permutation) preferences
    if (npr>precsSize) npr = precsSize;
    //initialize removedPrecs count and the k index (for precs) to 0
    removedPrecs = k = 0;
    //initialize fallback variables to a non-sense status
    ii = jj = -1;
    bestExcess = n; //it is a strict upper-bound for the excess
    //remove (almost always) npr precedences following the order in the random permutation
    while (removedPrecs<npr) {
        //very last iteration or not
        if (k<precsSize) { //not the very last iteration
            //get i<j indexes and increase k for next iteration
            i = precs[k].i;
            j = precs[k].j;
            k++;
            //if i<j is already removed, jump to next iteration
            if (isItemInList(i,ui[j])) continue;
            //predict number of precedences removed by removing i<j and compute its excess
            precsToRemove = countPrecsToRemove(i,j);
            excess = precsToRemove+removedPrecs - npr;
            //check if there is excess or not
            if (excess<=0) {
                //no excess, so reset excess (if set) and remove i<j (and its induced precedences)
                ii = jj = -1;
                bestExcess = n;
                removedPrecs += remPrecs(i,j);
            } else if (excess<bestExcess) { //&& excess>0
                //there is excess and it is better than previous best excess, so update it
                ii = i;
                jj = j;
                bestExcess = excess;
            }
            //done the not very last iteration
        } else { //k==precsSize, so very last iteration
            //if there is a fallback precedence to remove, then remove it
            if (ii!=-1) //&& jj!=-1 && bestExcess<n && k==precsSize
                removedPrecs += remPrecs(ii,jj);
            //exit main while: no need if there was a fallback precedence, but the break avoid infinite loop also in the very unlucky case when the removed precedences are less than NPR
            break;
            //done the very last iteration
        }
    } //end main while
    //unvalidate vsum and vmax
    //vsum_valid = vmax_valid = false;
    vsum_valid = false;
    //return number of removedPrecs
    return removedPrecs;
}



/*//calculate sum and max of the values of available precedences
 void PSolution::compute_vsum_vmax() {
 if (vsum_valid && vmax_valid) return;
 unsigned long int_vsum = 0;
 int int_vmax = -1; //-inf
 unsigned long int_isum; //values' sum of unassigned precedences i<*
 int i,j,k,t,val,in;
 for (i=0; i<n; i++) {
 in = i*n;
 t = un[i];
 int_isum = 0;
 for (k=0; k<t; k++) {
 j = u[i][k];
 val = lop[in+j];
 int_isum += val;
 if (val>int_vmax) {
 int_vmax = val;
 imax = i;
 jmax = j;
 }
 }
 int_vsum += int_isum;
 uvsum[i] = int_isum + t*EPS;
 }
 vsum = int_vsum + (n2-n-np)*EPS;
 vmax = int_vmax + EPS;
 vsum_valid = vmax_valid = true;
 }*/



//calculate only sum of the values of availalble precedences
void PSolution::compute_vsum() {
    //code based on compute_vsum_vmax
    if (vsum_valid) return;
    unsigned long int_vsum = 0;
    unsigned long int_isum; //values' sum of unassigned precedences i<*
    int i,j,k,t,in;
    for (i=0; i<n; i++) {
        in = i*n;
        t = un[i];
        int_isum = 0;
        for (k=0; k<t; k++) {
            j = u[i][k];
            int_isum += lop[in+j];
        }
        int_vsum += int_isum;
        uvsum[i] = int_isum + t*EPS;
    }
    vsum = int_vsum + (n2-n-np)*EPS;
    vsum_valid = true;
}



//calculate only sum of the values of availalble precedences
/*void PSolution::compute_vmax() {
 //code based on compute_vsum_vmax
 if (vmax_valid) return;
 int int_vmax = -1; //-inf
 int i,j,k,t,in,val;
 for (i=0; i<n; i++) {
 in = i*n;
 t = un[i];
 for (k=0; k<t; k++) {
 j = u[i][k];
 val = lop[in+j];
 if (val>int_vmax) {
 int_vmax = val;
 imax = i;
 jmax = j;
 }
 }
 }
 vmax = int_vmax + EPS;
 vmax_valid = true;
 }*/



//print matrix to cout
void PSolution::print() {
    int i,j;
    cout << "    ";
    for (i=0; i<n; i++) {
        if (i<10) cout << "   ";
        else if (i<100) cout << "  ";
        else cout << " ";
        cout << i;
    }
    cout << endl;
    for (i=0; i<4+4*n; i++)
        cout << "-";
    cout << endl;
    for (i=0; i<n; i++) {
        if (i<10) cout << "  ";
        else if (i<100) cout << " ";
        cout << i << "|";
        for (j=0; j<n; j++) {
            if (i==j)
                cout << "   0";
            else if (isItemInList(j,ui[i]))
                cout << "   0";
            else if (isItemInList(j,si[i])) //i<j
                cout << "   1";
            else if (isItemInList(j,pi[i])) //j<i
                cout << "  -1";
            else //error
                cout << "   E";
        }
        cout << endl;
    }
}



//debug print
void PSolution::printData() {
    cout << "---" << endl;
    cout << "n = " << n << endl;
    cout << "nc2 = " << nc2 << endl;
    cout << "n2 = " << n2 << endl;
    cout << "---" << endl;
    cout << "np = " << np << endl;
    cout << "fit = " << fit << endl;
    cout << "---" << endl;
    for (int i=0; i<n; i++) {
        cout << "[item " << i << "]" << endl;
        //print u-lists
        cout << "  un[" << i << "] = " << un[i] << endl;
        cout << "  u[" << i << "] = ";
        for (int j=0; j<un[i]; j++) cout << u[i][j] << " ";
        cout << endl;
        cout << "  ui[" << i << "] = ";
        for (int j=0; j<n; j++) cout << ui[i][j] << " ";
        cout << endl;
        //print s-lists
        cout << "  sn[" << i << "] = " << sn[i] << endl;
        cout << "  s[" << i << "] = ";
        for (int j=0; j<sn[i]; j++) cout << s[i][j] << " ";
        cout << endl;
        cout << "  si[" << i << "] = ";
        for (int j=0; j<n; j++) cout << si[i][j] << " ";
        cout << endl;
        //print p-lists
        cout << "  pn[" << i << "] = " << pn[i] << endl;
        cout << "  p[" << i << "] = ";
        for (int j=0; j<pn[i]; j++) cout << p[i][j] << " ";
        cout << endl;
        cout << "  pi[" << i << "] = ";
        for (int j=0; j<n; j++) cout << pi[i][j] << " ";
        cout << endl;
    }
    cout << "---" << endl;
}
