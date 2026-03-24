#include "lop.h"
#include <cstring>
#include <cstdio>
#include <climits>
#include <cstdlib>
#include <iostream>

using namespace std;


#define STR_SIZE 512



//global counter of the evaluations performed in the run
unsigned long nevals = 0;



void lop_readInstance(int*& lop1, int& n, int& opt, char* filename) {
	FILE* f = fopen(filename,"r");
	if (!f) {
		cerr << "ERROR: Unable to open " << filename << endl;
		exit(EXIT_FAILURE);
	}
	bool dotmat = false;
	if (strstr(filename,".mat")) dotmat = true;
	if (dotmat) while (fgetc(f)!='\n');	//jump the first line
	if (fscanf(f,"%d",&n)!=1) {
		cerr << "ERROR: Unable to read n from " << filename << endl;
		exit(EXIT_FAILURE);
	}
	int n2 = n*n;
	if (!lop1) lop1 = new int[n2];
	for (int i=0; i<n2; i++)
		if (fscanf(f,"%d",&lop1[i])!=1) {
			cerr << "ERROR: Unable to read " << i << "-th (0-based) entry from " << filename << endl;
			exit(EXIT_FAILURE);
		}
	if (fscanf(f,"%d",&opt)!=1)
		opt = INT_MAX;
	fclose(f);
	if (dotmat) {
		char optfile[STR_SIZE];
		int j;
		for (j=0; filename[j]!='.'; j++) optfile[j] = filename[j];
		optfile[j] = '.';
		optfile[j+1] = 'o';
		optfile[j+2] = 'p';
		optfile[j+3] = 't';
		optfile[j+4] = '\0';
		f = fopen(optfile,"r");
		if (f) {
			char line[STR_SIZE];
			if (!fgets(line,STR_SIZE,f)) {
				cerr << "ERROR: Unable to read optimum from " << optfile << endl;
				exit(EXIT_FAILURE);
			}
			fclose(f);
			if (!strstr(line,"optimum solution")) {
				//line has this structure:   Value               : 3095130
				char s1[STR_SIZE],semicolon;
				if (sscanf(line,"%s %c %d",s1,&semicolon,&opt)!=3) {
					cerr << "ERROR: Unable to read optimum (2) from " << optfile << endl;
					exit(EXIT_FAILURE);
				}
			} else {
				//line has this structure: BELGIUM T59B11XXB, optimum solution (value 245750)
				char s2[STR_SIZE];
				int k;
				for (j=0; line[j]!='('; j++);
				for (k=j+1; line[k]!=')'; k++)
					s2[k-j-1] = line[k];
				s2[k-j-1] = '\0';
				if (sscanf(s2,"%s %d",line,&opt)!=2) {
					cerr << "ERROR: Unable to read optimum (3) from " << optfile << endl;
					exit(EXIT_FAILURE);
				}
			}
		}
		fclose(f);
	}
	//done
}




void lop_printInstance(int* lop1, int n) {
	int i,j,k;
	k = 0;
	for (i=0; i<n; i++) {
		for (j=0; j<n; j++) {
			int v = lop1[k];
			cout << (v<10?"   ":(v<100?"  ":(v<1000?" ":""))) << v << " ";
			k++;
		}
		cout << "\n";
	}
}



int lop_eval(int* x, int n, int* lop1) {
	int i,j,xin,r;
	r = 0;
	for (i=0; i<n; i++) {
		xin = x[i]*n;
		for (j=i+1; j<n; j++)
			r += lop1[xin+x[j]];
	}
	nevals++;
	return r;
}



int lop_normalize(int* lop1, int n, int opt) {
	//opt<0 means "we dont know the optimum of this istance", so do nothing
	for (int i=0; i<n; i++)
		lop1[i*n+i] = 0;
	for (int i=0; i<n; i++) {
		for (int j=i+1; j<n; j++) {
			int min = lop1[i*n+j]<lop1[j*n+i] ? lop1[i*n+j] : lop1[j*n+i];
			lop1[i*n+j] -= min;
			lop1[j*n+i] -= min;
			if (opt>=0) opt -= min;
		}
	}
	return opt;
}



void lop_bounds(int* lop1, int n, int& lb, int& ub) {
	int a,b,min,max;
	lb = ub = 0;
	for (int i=0; i<n; i++) {
		for (int j=i+1; j<n; j++) {
			a = lop1[i*n+j];
			b = lop1[j*n+i];
			if (a<b) {
				min = a;
				max = b;
			} else {
				min = b;
				max = a;
			}
			lb += min;
			ub += max;
		}
	}
}
