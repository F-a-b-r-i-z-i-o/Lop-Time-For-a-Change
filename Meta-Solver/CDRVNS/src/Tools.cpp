//
//  Tools.cpp
//  VNSforLOP
//
//  Created by Josu Ceberio Uribe on 21/09/17.
//  Copyright © 2017 Collaboration Santucci - Ceberio. All rights reserved.
//

#include "../include/Tools.hpp"
#include <cstdlib>  
#include <stdio.h>
#include <string.h>

/*
 * Sorts the array of longs in the descending order.
 */
void QuickSort(long * arr,int low, int high)
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
        QuickSort(arr,low,j-1);
        QuickSort(arr,j+1,high);
    }
}
/*
 * Prints in the standard output the given matrix.
 */
void PrintMatrix(int** matrix, int length, int length2){
    int i=0,j=0;
    for (i=0;i<length;i++){
        for (j=0;j<length2;j++)
        {
            printf("%d ",matrix[i][j]);
        }
        printf("\n");
    }
}

/*
 * Prints in the standard output the given matrix.
 */
void PrintMatrix(long** matrix, int length, int length2){
    int i=0,j=0;
    for (i=0;i<length;i++){
        for (j=0;j<length2;j++)
        {
            printf("%ld ",matrix[i][j]);
        }
        printf("\n");
    }
}

/*
 * Generates a random permutation of size 'n' in the given array.
 */
void GenerateRandomPermutation(int n, int * permutation)
{
    for (int i = 0; i < n; ++i)
    {
        int j = rand() % (i + 1);
        permutation[i] = permutation[j];
        permutation[j] = i;
    }
}


/*
 * This method moves the value in position i to the position j.
 */
void InsertAt2 (int * array, int i, int j, int n)
{
    if (i!=j)
    {
        int res[n];
        int val=array[i];
        if (i<j)
        {
            memcpy(res,array,sizeof(int)*i);
            
            for (int k=i+1;k<=j;k++)
                res[k-1]=array[k];
            
            res[j]=val;
            
            for (int k=j+1;k<n;k++)
                res[k]=array[k];
        }
        else if (i>j)
        {
            memcpy(res,array,sizeof(int)*j);
            
            res[j]=val;
            
            for (int k=j;k<i;k++)
                res[k+1]=array[k];
            
            for (int k=i+1;k<n;k++)
                res[k]=array[k];
        }
        memcpy(array,res,sizeof(int)*n);
    }
}

float HistoricProbability(int i, int j, int ** memory){
    
    return 1;
}





/*
 * This method performs moves to the best neighbour in the insert neighbourhood.
 */
long GreedyLocalSearch_Insert(int * solution, long fitness, LOP * lop){
    
    int f_i,f_j,f_t,f_k,f_v, f_best_j;
    long int f_maxgain,improvement ;
    long int f_last_i = 0;
    int min,max;
    
    do{
       
        improvement=0;
            
        f_best_j = 0;
        f_maxgain = 0;
            
        for ( f_v = 0, f_i = f_last_i; f_v < lop->m_problem_size && (lop->m_max_evaluations>lop->m_evaluations); f_v++, f_i++ ) {
            long int f_gain;
            
            f_gain = f_maxgain = 0;
            if (f_i == lop->m_problem_size) f_i = 0;
                
                min=lop->m_sparsity_boundaries[solution[f_i]][0];
                max=lop->m_sparsity_boundaries[solution[f_i]][1];
                
                for (f_j = f_i-1; f_j >= min && (lop->m_max_evaluations>lop->m_evaluations); f_j--) {
                    f_gain += lop->m_pairwise_differences[solution[f_i]][solution[f_j]];
                    if (f_gain > f_maxgain ) {
                        f_maxgain = f_gain;
                        f_best_j = f_j;
                    }
                    lop->m_evaluations++;
                }
                f_gain = 0;
                for (f_j = f_i+1; f_j < max && (lop->m_max_evaluations>lop->m_evaluations); f_j++) {
                    f_gain += lop->m_pairwise_differences[solution[f_j]][solution[f_i]];
                    if (f_gain > f_maxgain ) {
                        f_maxgain = f_gain;
                        f_best_j = f_j;
                    }
                    lop->m_evaluations++;
                }
                
                //update solution with the best movement.
                if ( f_maxgain > 0 ) {
                    f_j = f_best_j;
                    f_t = solution[f_i];
                    if (f_i < f_j)
                        for ( f_k = f_i+1; f_k <= f_j; f_k++ ) solution[f_k-1] = solution[f_k];
                    else
                        for ( f_k = f_i - 1; f_k >= f_j; f_k-- ) solution[f_k+1] = solution[f_k];
                    solution[f_j] = f_t;
                    f_last_i = f_i+1;
                    improvement=f_maxgain;
                    break;
                }
            }
        fitness += improvement;
    }
    while (improvement >0 && (lop->m_max_evaluations>lop->m_evaluations));
    return fitness;
    
}



/*
 * This method performs moves to the best neighbour in the interchange neighbourhood.
 */
long Best_IntechangeStep(int * solution, long fitness, LOP * lop){
    int k,j, best_k=0,best_j=0;
    int aux;
    long cost_neigh=0, cost_opt=fitness;
    
    for (k=0; k<lop->m_problem_size-1 && lop->m_evaluations<lop->m_max_evaluations; k++) {
        for (j=k+1; j<lop->m_problem_size && lop->m_evaluations<lop->m_max_evaluations; j++) {
            
            aux=solution[k];
            solution[k]=solution[j];
            solution[j]=aux;
            
            cost_neigh=lop->Evaluate(solution);
           
            if (cost_neigh>cost_opt) {
                best_k=k;
                best_j=j;
                cost_opt=cost_neigh;
            }
            aux= solution[j];
            solution[j]=solution[k];
            solution[k]=aux;
        }
    }
    
    aux= solution[best_j];
    solution[best_j]=solution[best_k];
    solution[best_k]=aux;
    
    return cost_opt;
}

/*
 * This method performs moves to the best neighbour in the interchange neighbourhood by performing an efficient evaluation of the neighborhood.
 */
long Best_IntechangeStep_Efficient(int * solution, long fitness, LOP * lop){
    int k,j, best_k=0,best_j=0;
    int aux;
    long cost_neigh=0, cost_opt=fitness, difference;
    int neighborhood_size=(lop->m_problem_size*(lop->m_problem_size-1))/2;
    if ((lop->m_max_evaluations-lop->m_evaluations)<neighborhood_size){
        //the evaluations will  run out while revising neighborhood.
        for (k=0; k<lop->m_problem_size-1 && lop->m_evaluations<lop->m_max_evaluations; k++) {
        for (j=k+1; j<lop->m_problem_size && lop->m_evaluations<lop->m_max_evaluations; j++) {
            
            difference=lop->EvaluateDifference_Interchange(solution, k,j);
            cost_neigh=fitness+difference;
            
            if (cost_neigh>cost_opt) {
                best_k=k;
                best_j=j;
                cost_opt=cost_neigh;
            }
        }
    }
    
    aux= solution[best_j];
    solution[best_j]=solution[best_k];
    solution[best_k]=aux;
    
    }else{ //the evaluations will not run out.
        for (k=0; k<lop->m_problem_size-1; k++) {
            for (j=k+1; j<lop->m_problem_size; j++) {
                
                //old version
               // aux=solution[k];
               // solution[k]=solution[j];
               // solution[j]=aux;
                
               // cost_neigh=lop->Evaluate(solution);
               // cout<<"old version: "<<cost_neigh<<endl;
               // aux= solution[j];
               // solution[j]=solution[k];
               // solution[k]=aux;
                
                difference=lop->EvaluateDifference_Interchange(solution, k,j);
                cost_neigh=fitness+difference;
               // cout<<"new version: "<<cost_neigh<<endl;
               // exit(1);
                if (cost_neigh>cost_opt) {
                    best_k=k;
                    best_j=j;
                    cost_opt=cost_neigh;
                }
            }
        }
        
        aux= solution[best_j];
        solution[best_j]=solution[best_k];
        solution[best_k]=aux;
    }
    return cost_opt;
}

/*
 * This method performs moves to the best neighbour in the interchange neighbourhood by performing an efficient evaluation of the neighborhood with time as stopping criterion.
 */
long Best_IntechangeStep_Efficient_time(int * solution, long fitness, LOP * lop){
    int k,j, best_k=0,best_j=0;
    int aux;
    long cost_neigh=0, cost_opt=fitness, difference;
    //the evaluations will not run out.
        for (k=0; k<lop->m_problem_size-1; k++) {
            for (j=k+1; j<lop->m_problem_size; j++) {
                
                difference=lop->EvaluateDifference_Interchange(solution, k,j);
                cost_neigh=fitness+difference;
                // cout<<"new version: "<<cost_neigh<<endl;
                // exit(1);
                if (cost_neigh>cost_opt) {
                    best_k=k;
                    best_j=j;
                    cost_opt=cost_neigh;
                }
            }
        }
        
        aux= solution[best_j];
        solution[best_j]=solution[best_k];
        solution[best_k]=aux;
    
    return cost_opt;
}

/*
 * It applies random shake_power times a perturbation on the given solution.
 */
void Shake_Swap(int * solution, int size, int num_mutations)
{
    int i,j,aux;
    for (int iter=0;iter<num_mutations;iter++)
    {
        i = rand() % size;
        j = rand() % size;
        
        aux=solution[j];
        solution[j]=solution[i];
        solution[i]=aux;
    }
}

bool isPermutation(int * permutation, int size)
{
    int * flags=new int[size];
    for (int i=0;i<size;i++) flags[i]=1;
    
    for (int i=0;i<size;i++)
    {
        int value=permutation[i];
        flags[value]=0;
    }
    
    int result,sum=0;
    for(int i=0;i<size;i++)
        sum+=flags[i];
    if (sum==0) result=true;
    else result=false;
    delete [] flags;
    return result;
}

/*
 * Prints in the standard output the 'size' elements of the given array..
 */
void PrintArray(int * array, long size)
{
    for (long i=0;i<size;i++){
        cout<<array[i]<<" ";
    }
    cout<<" "<<endl;
}

void PrintArray(long * array, long size)
{
    for (long i=0;i<size;i++){
        cout<<array[i]<<" ";
    }
    cout<<" "<<endl;
}

