//
//  Tools.hpp
//  VNSforLOP
//
//  Created by Josu Ceberio Uribe on 21/09/17.
//  Copyright © 2017 Collaboration Santucci - Ceberio. All rights reserved.
//

#ifndef Tools_hpp
#define Tools_hpp
#include "LOP.h"
#include <stdio.h>

/*
 * Sorts the array of longs in the descending order.
 */
void QuickSort(double * arr,int low, int high);

/*
 * Prints in the standard output the given matrix.
 */
void PrintMatrix(long** matrix, int length, int length2);

/*
 * Prints in the standard output the given matrix.
 */
void PrintMatrix(int** matrix, int length, int length2);

/*
 * Generates a random permutation of size 'n' in the given array.
 */
void GenerateRandomPermutation(int size, int * permutation);

/*
 * This method performs moves stochastically in the insert neighbourhood considering information from the history and values of the instance.
 */
long double StochasticLocalSearch_Insert(int * solution, long double fitness, LOP * lop);
    
/*
 * This method performs moves to the best neighbour in the insert neighbourhood.
 */
long double GreedyLocalSearch_Insert(int * solution, long double fitness, LOP * lop);

/*
 * This method performs moves to the best neighbour in the interchange neighbourhood.
 */
long double Best_IntechangeStep(int * solution, long double fitness, LOP * lop);

/*
 * This method performs moves to the best neighbour in the interchange neighbourhood by performing an efficient evaluation of the neighborhood.
 */
long double Best_IntechangeStep_Efficient(int * solution, long double fitness, LOP * lop);

/*
 * This method performs moves to the best neighbour in the interchange neighbourhood by performing an efficient evaluation of the neighborhood with time as stopping criterion.
 */
long double Best_IntechangeStep_Efficient_time(int * solution, long double fitness, LOP * lop);

/*
 * It applies random shake_power times a perturbation on the given solution.
 */
void Shake_Swap(int * solution, int size, int num_mutations);

/*
 * Prints in the standard output the 'size' elements of the given array..
 */
void PrintArray(int * array, long size);
void PrintArray(long * array, long size);

bool isPermutation(int * permutation, int size);
#endif /* Tools_hpp */
