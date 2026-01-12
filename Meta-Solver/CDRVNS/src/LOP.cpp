//
//  LOP.cpp
//  VNSforLOP
//
//  Created by Josu Ceberio Uribe on 21/09/17.
//  Copyright © 2017 Collaboration Santucci - Ceberio. All rights reserved.
//

#include "../include/LOP.h"
#include "../include/Tools.hpp"
#include <algorithm>
#include <climits>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

/*
 * Constructor method.
 */
LOP::LOP()
{
    m_problem_size=10;
    m_max_evaluations=0;
}

/*
 * Destructor method.
 */
LOP::~LOP()
{
    for (int i=0;i<m_problem_size;i++){
		delete [] m_instance[i];
        delete [] m_sparsity_boundaries[i];
        delete [] m_pairwise_differences[i];
    }
	delete [] m_instance;
    delete [] m_sparsity_boundaries;
    delete [] m_pairwise_differences;
}


/*
 * Read LOP instance from file.
 */
int LOP::Read(char * filename){
    
    // Open connection with file and read instance size.
    FILE *file=fopen(filename, "r");
    fscanf(file,"%d",&m_problem_size);
 #ifdef VERBOSE
    printf("%d\n", m_problem_size);
#endif
    // Initiaze structures.
    m_instance = new int*[m_problem_size];
    m_sparsity_boundaries= new int*[m_problem_size];
    m_pairwise_differences = new int*[m_problem_size];
    //cout<<"..... Allocating..."<<endl;
    for (int i=0;i<m_problem_size;i++)
    {
        m_instance[i]= new int[m_problem_size];
        m_sparsity_boundaries[i]= new int[2];
        m_pairwise_differences[i]= new int[m_problem_size];
    }
    
    // Read instance.
    int size=100192;
    char line[size];
    char * chop;
    const char * delimiter=" ";
    int i=0, j=0;
    fgets(line, size, file);
    //Read the whole file to get the dimensions of data.
    while (fgets(line, size, file)!=NULL){
        if(line[0]!='\n'){
            chop=strtok(line,delimiter);
            j=0;
            while(chop!=NULL){
                m_instance[i][j]=atoi(chop);
                chop=strtok(NULL,delimiter);
                j++;
            }
        }
        i++;
    }
    fclose(file);
    
    // Compute the pairwise differences. Very useful to calculate the sparsity matrix and other related issues.
    for ( i = 0; i < m_problem_size-1; i++){
        for ( j = i+1; j < m_problem_size; j++){
            m_pairwise_differences[i][j]=m_instance[i][j]-m_instance[j][i];
            m_pairwise_differences[j][i]=m_instance[j][i]-m_instance[i][j];
        }
    }
    m_evaluations=0;
    return m_problem_size;
}

/*
 * Calculates the corresponding objective value of the solution for the LOP problem.
 */
long LOP::Evaluate(int * solution)
{
	long value=0;
    int i,j;
    for (i=0;i<m_problem_size-1;i++){
		for (j=i+1;j<m_problem_size;j++){
			value+= m_instance[solution[i]][solution[j]];
        }
    }
    m_evaluations++;
	return value;
}

/*
 * Calculates the objective value variation due to the interchange of items k and j in the given solution for the LOP problem.
 */
long LOP::EvaluateDifference_Interchange(int * solution, int k, int j)
{
    long sum_out=0;
    long sum_in=0;
    int i;
    //sum_out
    for (i=k+1;i<j;i++){
        sum_out+=m_instance[solution[k]][solution[i]];
    }
    for (i=k;i<j;i++){
        sum_out+=m_instance[solution[i]][solution[j]];
    }
    
    //sum_in
    for (i=k;i<j;i++){
        sum_in+=m_instance[solution[j]][solution[i]];
    }
    for (i=k+1;i<j;i++){
        sum_in+=m_instance[solution[i]][solution[k]];
    }
    m_evaluations++;
    return sum_in-sum_out;
}

/*
 * Returns the size of the problem.
 */
int LOP::GetProblemSize()
{
    return m_problem_size;
}

/*
 * Returns the sparsity matrix of the LOP instance.
 */
int ** LOP::GetSparsityMatrix()
{
    return m_sparsity_boundaries;
}

/*
 * Calculates the sparsity matrix of the LOP instance.
 */
int ** LOP::CalculateSparsityMatrix(){
        
    long * differencesArray= new long[m_problem_size-1];
    int item=0;
    int beforeSum, afterSum;
    int min,max;
    int k,i;
    int restriction;
    int val=m_problem_size-1;
    for (item=0;item<m_problem_size;item++)
    {
        //obtain the array of differences.
        for (i=0;i<item;i++)
            differencesArray[i]=-m_pairwise_differences[item][i];
        for (i=item+1;i<m_problem_size;i++)
            differencesArray[i-1]=-m_pairwise_differences[item][i];
        
        
        //quick sort algorithm
        QuickSort(differencesArray, 0, val-1);
        
        restriction=INT_MIN;
        
        for (min=0;min<m_problem_size;min++)
        {
            beforeSum=0;
            for (k=0;k<min;k++)
                beforeSum+=differencesArray[k];
            
            afterSum=0;
            for (k=min;k<val;k++)
                afterSum+=differencesArray[k];
            
            if (beforeSum>=0 && afterSum <=0)
            {
                restriction=min;
                m_sparsity_boundaries[item][0]=min;
                m_sparsity_boundaries[item][1]=m_problem_size;
                //  printf("MIN: %d max: %d\n",min,PSize);
                break;
            }
        }
        if (restriction==0){
            
            for (max=val;max>=0;max--)
            {
                beforeSum=0;
                for (k=0;k<max;k++)
                    beforeSum+=differencesArray[k];
                
                afterSum=0;
                for (k=max;k<val;k++)
                    afterSum+=differencesArray[k];
                
                if (beforeSum>=0 && afterSum <=0)
                {
                    //     restriction=max-(val);
                    m_sparsity_boundaries[item][0]=0;
                    m_sparsity_boundaries[item][1]=max+1;
                    //       printf("min: %d MAX: %d\n",0,max+1);
                    break;
                }
            }
        }
    }
    delete [] differencesArray;
    return m_sparsity_boundaries;
}

/*
 * Converts the matrix of parameters to an array of parameters of length n^2.
 */
void LOP::GetMatrixAsArray(int * array){
    int i,z,j=0;
    
    for (i=0;i<m_problem_size;i++){
        for (z=0;z<m_problem_size;z++){
            array[j]=m_instance[i][z];
            j++;
        }
    }
    
    
}


/*
 * Computes the probability to perform an insert operation moving element at position i to position j.
 */
double LOP::PrecedenceProbability(int i, int j, int * solution){
    double up=0,down=0;
    int pos;
    
    if (i==j)
        return 0;
    
    if (i>j){
        for (pos=i;pos>j;pos--){
            up+=this->m_instance[solution[i]][solution[j]];
            down+=(this->m_instance[solution[i]][solution[j]]+this->m_instance[solution[j]][solution[i]]);
        }
    }
    else{
        for (pos=i;pos<j;pos++){
            up+=this->m_instance[solution[j]][solution[i]];
            down+=(this->m_instance[solution[j]][solution[i]]+this->m_instance[solution[i]][solution[j]]);
        }
    }
    if (down==0){
        return 0.5;
    }
    else{
        return (up/down);
    }
}




