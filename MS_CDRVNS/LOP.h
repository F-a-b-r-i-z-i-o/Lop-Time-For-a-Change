//
//  LOP.h
//  VNSforLOP
//
//  Created by Josu Ceberio Uribe on 21/09/17.
//  Copyright © 2017 Collaboration Santucci - Ceberio. All rights reserved.
//

#ifndef _LOP_H__
#define _LOP_H__

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdio.h>


using std::istream;
using std::ostream;
using namespace std;
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::stringstream;
using std::string;

class LOP 
{
	
public:

    /*
     * Matrix of parameters of the instance.
     */
    int ** m_instance;
    
    /*
     * Sparsity matrix. n x 2 size matrix.
     */
    int ** m_sparsity_boundaries;
    
    /*
     * Matrix with pairwise differences. That is m_instance[i][j]-m_instance[j][i].
     */
    int ** m_pairwise_differences;
    
    /*
     * The size of the problem.
     */
    int m_problem_size;
    
    /*
     * Maximum number of evaluations allowed to perform.
     */
    long long int m_max_evaluations;
        
    /*
     * The number of evaluations currently performed.
     */
    long long int m_evaluations;
    
    /*
     * The heuristic pairwise precedence of probability.
     */
    double ** m_heuristic_precedence_probabilities;
    
    /*
     * Constructor method.
     */
	LOP();
   
    /*
     * The destructor.
     */
    virtual ~LOP();
	
	/*
	 * Read LOP instance from file.
	 */
    int Read(char * filename);
    
	/*
	 * Calculates the corresponding objective value of the solution for the LOP problem.
	 */
	long Evaluate(int * solution);
	
    /*
     * Calculates the objective value variation due to the interchange of items k and j in the given solution for the LOP problem.
     */
    long EvaluateDifference_Interchange(int * solution, int k, int j);
    
    /*
     * Returns the size of the problem.
     */
    int GetProblemSize();
    
    /*
     * Returns the sparsity matrix of the LOP instance.
     */
    int ** GetSparsityMatrix();
    
    /*
     * Calculates the sparsity matrix of the LOP instance.
     */
    int ** CalculateSparsityMatrix();
        
    /*
     * Converts the matrix of parameters to an array of parameters of length n^2.
     */
    void GetMatrixAsArray(int * array);
    
    /*
     * Computes the probability to perform an insert operation moving element at position i to position j.
     */
    double PrecedenceProbability(int i, int j, int * solution);
    
private:


    
    
};
#endif





