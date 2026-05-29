#ifndef EXHAUSTIVE_EVALUATION_H
#define EXHAUSTIVE_EVALUATION_H

#include <vector>
using namespace std;

//H è la matrice LOP, nn è la dimensione della matrice
//optima sarà riempito con gli ottimi globali
//verbose stampa una riga per ogni valutazione
//il valore ritornato dalla funzione è la fitness ottima
int exhaustive_evaluation(int* H, int nn, vector<int*>& optima, bool verbose=false);

#endif
