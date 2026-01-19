//
//  main.cpp
//  CDRVNSforLOP
//
//  Created by Josu Ceberio Uribe on 09/01/2019.
//  Copyright © 2019 University of the Basque Country. All rights reserved.
//
#include "ParameterSetting.h"
#include "LOP.h"
#include <iostream>
#include <sys/time.h>
#include "PSolution.h"
#include "random.h"
#include <math.h>
#include "Tools.hpp"
#include <stdio.h>
#include <stdlib.h>
#include "MultiSolutionSet.h"//VALENTINO

void Compile_Precedences (long ** memory, int n, int * solution){
    int i,j;
    for (i=0;i<n-1;i++){
        for (j=i+1;j<n;j++){
            memory[solution[i]][solution[j]]++;
        }
    }
}


int main(int argc, char * argv[]) {
    
    //0. Get parameters from command line
    if(!GetParameters(argc,argv))
        return -1;
    
    //1. Start timing.
    struct timeval tim;
    gettimeofday(&tim, NULL);
    double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
 #ifdef VERBOSE
    cout<<"Reading instance..."<<endl;
#endif
    //1. Read LOP instance
    LOP * lop= new LOP();
    
    int n=lop->Read(INSTANCE_FILENAME);
    
    //calculate the sparsity of the algorithm
    int i;
    if (n<=3000){
        lop->m_max_evaluations=(long long int)1000*n*n;
    }else{
        lop->m_max_evaluations=(long long int)10*1000*1000*1000;
    }
	cout << n;
	MultiSolutionSet msset(5, n);//VALENTINO
    

#ifdef VERBOSE
    cout<<"Stopping criteria..."<<endl;
    cout<<"Evals: "<<lop->m_max_evaluations<<endl;
#endif
    initRand(SEED);
 #ifdef VERBOSE
    cout<<"Initializing valentinos code..."<<endl;
#endif
    //create precedences memory
    int compiled_solutions=0;
   long ** memory= new long*[n];
    for (i=0;i<n;i++)
    {
        memory[i]=new long[n];
        fill_n(memory[i],lop->m_problem_size,0);
    }

    int * arrayLOP= new int[n*n];
    for (int i=0;i<n*n;i++)arrayLOP[i]=-1;
    lop->GetMatrixAsArray(arrayLOP);
    //PrintArray(arrayLOP,n*n);
    PSolution::setLOPMatrix(arrayLOP, n);
    PSolution psol(n);
    psol.makeEmpty();
    initRand(SEED);
 #ifdef VERBOSE
    cout<<"Calculating restricted matrix..."<<endl;
#endif
    //2. Compute sparsity matrix
    lop->CalculateSparsityMatrix();

    // 3. Declare variables for optimization
    bool improvement=false;
    int * solution= new int[n];
    int * best_solution= new int[n];
    long int fitness=0;
    long int best_fitness=0;
    long int fit=0;
    float factor=0;
  
#ifdef VERBOSE
    cout<<"Running constructive algorithm..."<<endl;
#endif
    //3. Construct initial solution (Valentino)
    int * prev_solution= new int[n];
    Q=urand()*0.1+0.9; //according to Valentino it should be set in the range [0.9,1].
    psol.construct(Q);
    psol.toPermutation(solution);
    fitness=psol.fit;
    lop->m_evaluations++;
    
    memcpy(best_solution,solution,sizeof(int)*n);
    best_fitness=fitness;
    
    Compile_Precedences(memory, n, solution);
    compiled_solutions++;
    
#ifdef VERBOSE
    PrintArray(solution,n);
    printf("fitness. %ld   evals rem. %lld\n",best_fitness,lop->m_max_evaluations-lop->m_evaluations);
    cout<<"Running VNS..."<<endl;
#endif
    //4. Run VNS
    do{
       // cout<<"my code"<<endl;
        improvement=false;
        do{
            //4.1. Efficient local search on the insert neighbourhood
            
            fitness=GreedyLocalSearch_Insert(solution,fitness,lop);
            
            //4.2. One Greedy step in the interchange neighbourhood
            fit=Best_IntechangeStep_Efficient(solution, fitness, lop);

            if (fit>fitness){
                fitness=fit;
                improvement=true;
            }
        }while (improvement && lop->m_evaluations<lop->m_max_evaluations);
		
		msset.update_set(solution,fitness);//VALENTINO
		
       // cout<<"compile precendences"<<endl;
        Compile_Precedences(memory, n, solution);
        compiled_solutions++;
       // PrintArray(memory[0], n);
        
        //4.3. Update best solution if it is the case.
        if (fitness>best_fitness){
            memcpy(best_solution, solution, sizeof(int)*n);
            best_fitness=fitness;
#ifdef VERBOSE
            printf("fitness. %ld   evals rem. %lld\n",best_fitness,lop->m_max_evaluations-lop->m_evaluations);
            //PrintArray(best_solution, n);
#endif
        }
        else{
            memcpy(solution, best_solution, sizeof(int)*n);
        }
        
        if (lop->m_evaluations<lop->m_max_evaluations){
          //  cout<<"valentinos code"<<endl;
        //4.4. Destruction-Construction procedure on 'solution' vector
        memcpy(prev_solution,solution,sizeof(int)*n);
        psol.fromPermutation(solution);
        do{
            Q=urand()*0.1+0.9; //according to Valentino it should be set in the range [0.9,1].
            factor=((float)lop->m_evaluations)/((float)lop->m_max_evaluations);
            R=1-(factor*0.9);
            psol.destruct_sorted(memory,R);
            psol.construct(Q);
            psol.toPermutation(solution);
        } while(memcmp(prev_solution, solution, sizeof(int)*n)==0 );
        
        fitness=psol.eval();
        lop->m_evaluations++;
        
        if (fitness>best_fitness){
            memcpy(best_solution, solution, sizeof(int)*n);
            best_fitness=fitness;
#ifdef VERBOSE
            printf("fitness repair. %ld   evals rem. %lld R: %g\n",best_fitness,lop->m_max_evaluations-lop->m_evaluations,R);
#endif
        }
        }

    }while(lop->m_evaluations<lop->m_max_evaluations);
#ifdef VERBOSE
    cout<<"out of the loop"<<endl;
#endif
    //5. Calculate consumed time.
    gettimeofday(&tim, NULL);
    double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
    
    //6. Write results in file.
    FILE *result_file;
#ifndef RUNNING_ON_CLUSTER
    FILE * test;
    test= fopen("/Users/Josu/Desktop/results.csv","r");
    result_file= fopen("/Users/Josu/Desktop/results.csv","a+");
    if (test==NULL){
        fprintf(result_file,"\"Instance\";\"Repetition\";\"Algorithm\";\"Fitness\";\"Time\"\n");
    }
     fprintf(result_file,"\"%s\";%d;\"%s\";%ld;%.3f\n",INSTANCE_FILENAME,SEED,"CDRVNS",best_fitness,t2-t1);
    
#else
    result_file= fopen("./scratch_results.csv","a+");
    fprintf(result_file,"\"%s\";%d;\"%s\";%ld;%.3f\n",INSTANCE_FILENAME,SEED,"CDRVNS",best_fitness,t2-t1);
#endif
    fclose(result_file);
 
    
    // Free memory
    delete lop;
    delete [] solution;
    delete [] best_solution;
    for (int i=0;i<n;i++)
        delete [] memory[i];
    delete [] memory;
    delete [] prev_solution;
    delete [] arrayLOP;
    
    return 0;
}

