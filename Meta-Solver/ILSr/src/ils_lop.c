/*    
    ILSLOP Iterated Local Search Algorithm for Linear Ordering Problem
    Copyright (C) 2004  Tommaso Schiavinotto (tommaso.schiavinotto@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <string.h>

#include "instance.h"
#include "utilities.h"
#include "timer.h"
#include "lop.h"
#include "ck.h"
#include "ac.h"

int n_tries;

long long int n_steps;
long long int ls_iteration;
long long int OptSol;
char *FileName;
char *FileNameResult;
int JustLS;
int TotalTime;
float MaxPertRatio;
long int **DiffMat;
int CoolLengthTimes;
int MaxLS;
int VeryVerbose;


#ifndef I_CFLAGS
#define I_CFLAGS  "-O -Wall -pedantic -ansi"
#endif

#define I_PRVERSION "0.2"

void printInfo(int argc, char **argv) {
  int i;
#if 0
  struct utsname buf[1];

  uname(buf);

  printf("SysName: %s\n",buf->sysname);
  printf("NodeName: %s\n",buf->nodename);
  printf("Release: %s\n",buf->release);
  printf("Version: %s\n",buf->version);
  printf("Machine: %s\n",buf->machine);
#ifdef _GNU_SOURCE
  printf("Domain Name: %s\n",buf->machine);
#endif
#endif  
  printf("Compiler gcc " __VERSION__ " Flags " I_CFLAGS "\n");
  printf("\nIterated Local Search for LOP " I_PRVERSION);
  printf("\nParameters-settings\n");
  printf("\nCommand line ");
  for (i=0; i < argc; i++) {
    printf(" %s", argv[i]);
  }
  printf("\nseed %ld\n", Seed);
  printf("n_tries %d\n", n_tries);
  printf("try_time %d\n", try_time);
  puts("");
}

void printOut(long long int lb, long long int cycle, long long int st, long int eval, float time) {
 // printf("best %lld cycle %lld step %lld eval %ld time %g\n", lb, cycle, st, eval,time);
     printf("%lld,%ld,%g\n", lb, eval, time);
}

#define MAXPREFIX 1024

void usage() {
    fprintf(stderr, 
	    "ils_lop -i <instance> [options ...]\n"
	    "   -a [b|p<par>|r|s<par>]\n"
	    "      Acceptance Criterion type:\n"
	    "       b: Accept only if the new local optima is the best so far;\n"
	    "       p: [default: par=0.0001] Accept only if the solution is larger than (1-par) times\n"
	    "          the current local optima;\n"
	    "       r: Random walk (always accept);\n" 
	    "       s: Use a simulated annealing like acceptance criterion\n"
	    "          here par is the alpha for the cooling\n"
	    "   -b <value>\n"
	    "      If the value is reached the run stop before the given time limit\n"
	    "   -c <value>\n"
	    "      The Number of iteration before the temperature is cooled down\n"
	    "      (value times the problem size), for the SA like acceptance criterion\n"
	    "   -g Start WITHOUT a greedy solution\n"
	    "   -i <file> Instance input file\n"
	    "   -k <value> Number of LS iterations per trial\n"
	    "   -n <value> Number of trials\n"
	    "   -o <path> Path of output file\n"
	    "   -p <path/prefix> Path prefix for the output file\n"
            "      If neither -p nor -o options are used stdout is used\n"
	    "   -r Scan randomly the neighborhood during Local Search\n"
	    "         (not for CK or Best Improvement)\n"
	    "   -s <seed> Seed to be used for pseudo-random numbers generator\n"
	    "   -t <time> Time limit for each try [Default: 60s]\n"
	    "   -T        The time value indicated is not per trial but overall.\n"
	    "   -u [{i|I|x|s|f|F|R|c}...] Type of Local Search:\n"
	    "      i: First Improvement INSERT operator;\n"
            "      I: Best Improvement INSERT operator;\n"
            "      R: Best Improvement INSERT operator, smarter;\n"
            "      f: [default] Fast Insert with First Improvement;\n"
            "      F: Fast Insert with First Improvement with Ubiquity;\n"
            "      z: Fast Insert with First Improvement, some side moves accepted;\n"
	    "      x: First Improvement INTERCHANGE operator;\n"
	    "      X: Best Improvement INTERCHANGE operator;\n"
	    "      s: First Improvement SWAP operator;\n"
	    "      c: CK ;\n"
            /*"      If not specified no local search is done, "*/
			"	   a composition up to 5 LS is accepted\n"
            "      (e.g -uixsf)\n"
	    "   -V print all the localoptima considered\n"
	    "   -v <value> Number of application of a perturbation in a row\n"
	    "   -x [{i|r|x|R|N}] Type of Perturbation:\n"
	    "      i: Perturbation with random INSERT;\n"
	    "      1: Perturbation INSERTing the first element in a random position;\n"
            "      r: Perturbation with random REVERSE;\n"
            "      x: [default] Perturbation with random INTERCHANGE;\n"
            "      R: Random Restart;\n"
	    "      N: No perturbation is done (a single LS is performed).\n"
	    "   -w <value> Size (in element) of the region interested by a single perturbation\n"
	    "   -W <value> Size (ratio on the size) of the region interested by a single perturbation\n");
    exit(-1);
	    
}


void readOpts(int argc, char **argv) {
  int s, o, p, k, w, W;
  char opt;
  char prefix[MAXPREFIX], stmp[MAXPREFIX*2], stmp2[MAXPREFIX*2];
  char lstr[MAXLOCOPTTYPES+1];

  o = FALSE;
  s = FALSE;
  w = FALSE;
  p = FALSE;
  W = FALSE;
  FileName = NULL;
  RandomScan = NULL;
  PerturbationType = 'x';
  try_time = 60000;
  n_tries = 1;
  Greedy = FALSE;
  NumSwaps = 7;
  MaxPertLength = 0;
  MaxPertRatio = 1.0;
  LocalOptType = 'f';
  TotalTime = FALSE;
  LocOptMax = 1;
  JustLS = FALSE;
  OptSol = -1;
  AcceptanceCriterionType = 'p';
  ACParameter = 0.0001;
  CoolLengthTimes=1;
  MaxLS = 0;
  VeryVerbose = FALSE;
    evaluations_criterion=1;

    
    while ( (opt = getopt(argc, argv, "a:grc:b:u:k:Vv:W:w:x:Tt:n:s:p:o:i:z:")) > 0 ){

      switch (opt) {
	  case '?':
	      usage();
	      break;
	  case 'a': /* Type of perturbation */
	      AcceptanceCriterionType = optarg[0];
	      ACParameter = atof(&(optarg[1]));
	      break;
	  case 'b': /* Upper bound where to stop */
	      OptSol = atoll(optarg);
	      break;    
	  case 'c': /* Cooling length for SA */
	      CoolLengthTimes = atoi(optarg);
	      break;    
	  case 'u': /* Type of local optimization */
	      if ( ( LocOptMax = strlen(optarg) ) > 1 ) {
		  LocalOptType = 'm';
		  if ( LocOptMax > MAXLOCOPTTYPES ) 
		      fatal("Too many localopt types specified");
		  for ( k = 0; k < LocOptMax; k++ ) 
		      addLocOptFunction(optarg[k]);
	      } else {		  
		  LocalOptType = optarg[0];
	      }
	      strncpy(lstr, optarg, LocOptMax);
	      break;	      
	  case 'g': /* Start using random solution */
	      Greedy = FALSE;
	      break;
	  case 'k': /* Maximum number of LS for iteration */
	      MaxLS = atoi(optarg);
	      break;
	  case 'n': /* Number of tries */
	      n_tries = atoi(optarg);
	      break;
	  case 't': /* Seconds per try */
	      try_time = atoi(optarg);
          evaluations_criterion=0;
	      break;
	  case 'T': /* Seconds per try */
	      TotalTime = TRUE;
	      break;
	  case 'i': /* Instance file */
	      FileName = (char *)malloc(strlen(optarg)+1);
	      strncpy(FileName, optarg, strlen(optarg));
	      break;
	  case 's': /* Seed to use */
	      Seed = atol(optarg);
          printf("%ld\n",Seed);
	      s = TRUE;
	      break;
	  case 'o': /* Output file */
	      if ( fopen( optarg, "w" )==NULL )
              fatal("Error on opening output file\n");
              FileNameResult=(char*)malloc(strlen(optarg)+1);
              strncpy(FileNameResult, optarg, strlen(optarg));              
	      o = TRUE;
	      break;
	  case 'p': /* Prefix for the output file */
	      strncpy(prefix, optarg, MAXPREFIX);
	      p = TRUE;
	      break;
	  case 'r': /* Scan randomly the indexes */
	      RandomScan = (long int *)0xdeadbeef;
	      break;
	  case 'x': /* Type of perturbation */
	      PerturbationType = optarg[0];
	      JustLS = optarg[0]=='N';
	      break;
	  case 'V': /* Number of perturbation iteration */
	      VeryVerbose = TRUE;
	      break;
	  case 'v': /* Number of perturbation iteration */
	      NumSwaps = atoi(optarg);
	      break;
	  case 'w': /* Max Length of the segment interested by a single
		       perturbation */
	      MaxPertLength = atoi(optarg);
	      w = TRUE;
	      break;
	  case 'W':
	      MaxPertRatio = atof(optarg);
	      W=TRUE;
	      break;
	  default:
	      fprintf(stderr, "Option %c not managed.\n", opt);
	      usage();
      }

    }
    
  if ( !FileName ) usage();
  if ( p ) {
      if ( o ) {
	  puts("Options -p and -o cannot be used together.");
	  usage();
      }
      /* After the prefix the search type, and possibly the seed is indicated
	 in the output-file name */
      stmp2[0] = '\0';

      if ( w ) {
	  sprintf(stmp, "w%d", MaxPertLength);
	  strncat(stmp2, stmp, strlen(stmp));
      }

      if ( W ) {
	  sprintf(stmp, "W%.2f", MaxPertRatio);
	  strncat(stmp2, stmp, strlen(stmp));
      }

      if ( s ) {
	  sprintf(stmp, "s%ld", Seed);
	  strncat(stmp2, stmp, strlen(stmp));
      }
      
      sprintf(stmp, "%s_%s%sU%s_X%d%c_n%d_t%d_%s.out", 
	      prefix, 
	      RandomScan?"r":"",
	      Greedy?"g":"",
	      lstr,
	      JustLS?0:NumSwaps,
	      PerturbationType,
	      n_tries,
	      try_time,
	      stmp2);
      if ( !freopen(stmp, "w", stdout) )
	  fatal("Error on opening output file with given prefix\n");
  }
}



int main (int argc, char **argv) 
{

  long int i,j;
  long int *lo, *lobest, *locurrbest, *ptmp, *t1, *t2, *t3; 
  long long int length, best, gain, currbest;    
  int z;
  int nonimpr;
  long long  int step, k;
  long long int TotMat;
  float best_time;
  setbuf(stdout,NULL);
	  setbuf(stderr,NULL);
  
  if (argc < 2) usage();
  
  /* initialize random number generator, can be changed in ReadOpts */
  //Seed = (long int) time(NULL); 
  step = 0;

  readOpts(argc, argv);

  start_timers(); /* starts time measurement */

  CostMat = readInstance(FileName);
  TotMat=0;
  for ( i = 0; i < PSize; i++) 
      for ( j = 0; j < PSize; j++) 
	  TotMat += CostMat[i][j];
  DiffMat = createMatrix(PSize);
  for ( i = 0; i < PSize; i++) 
      for ( j = 0; j < PSize; j++) 
	  DiffMat[i][j]=CostMat[i][j]-CostMat[j][i];

  if (AcceptanceCriterionType == 's') {
      InitSAAC(ACParameter, CoolLengthTimes*PSize);
  }

  if (!MaxPertLength) {
      if ( MaxPertRatio < 1.0 ) 
	  MaxPertLength = PSize*MaxPertRatio;
      else
	  MaxPertLength = PSize;
  }
  //printInfo(argc, argv);
 
  /*printDistances();*/
  t1 = (long int *)malloc(PSize * sizeof(long int)); 
  t2 = (long int *)malloc(PSize * sizeof(long int)); 
  t3 = (long int *)malloc(PSize * sizeof(long int)); 

  //printf("Initialization time %g\n\n", elapsed_time(VIRTUAL));
  //printf("begin problem %s\n", FileName);

  CalculateTotSum();

 // for (i = 0; i < n_tries; i++) {
  //    printf("begin try %ld\n", i);
      best = -1;
      currbest = -1;
      if ( (i==0) || !JustLS || !TotalTime ) /* If not JustLS the total time can not be measured */
	  start_timers();
      lo = t1;
      lobest = t2; 
      locurrbest = t3;
    
    evaluations=0;
    if(LocalOptType=='F' ){
        CalculateUbiquityMatrix();
       // printf("Initialization time after ubiquity matrix %g\n\n", elapsed_time(VIRTUAL));
    }
    
      initialSolution(lo);
     // for (int i=0;i<PSize;i++)
     //     lo[i]=i;
     // printf("Initial solution: %ld  ",computeCost(lo));
     // for (int i=0;i<PSize;i++)
     //     printf("%ld ",lo[i]);
     // printf("\n");
      
      
      best = currbest = length = computeCost( lo );
      best_time=elapsed_time(VIRTUAL);
      ls_iteration = 0;

      k = 0;
     // FILE * flog= fopen("log.csv","w");

      LocalOptIdx = 0;
      if (LocalOptType == 'C') JustLS = FALSE;
        nonimpr=0;
        printOut(best, k, ls_iteration, evaluations, elapsed_time(VIRTUAL));
      do {

	  /* In the first iteration tbest is "undefined", but there is
	     no need of permutation, if JustLS => (k == 0) */
	  /* printf("Perturbating %d \n", k ); */
	  if ( (!JustLS && (PerturbationType == 'R'))) {
	      /* ((k > 0) && (k < 5)) ) { Random Restart*/
              free(lo); 
	      lo = generate_random_vector(PSize);
              length = computeCost( lo );
	  } else if ( ( k > 0 ) && ( LocalOptType != 'C' )  ) {
	      length = currbest;
	      perturbate(locurrbest, lo, &length);
	  }
	  
	  if (RandomScan) 
	      RandomScan = generate_random_vector(PSize);

	  initDS();
	  z = 0;
	  LocalOptIdx = 0;
      
	  while ( z < LocOptMax && Continue()==1 ) {
	      z++;
	      if ( LocalOptType == 'c' || 
		   ( (LocalOptType == 'm') && (LocOptArray[LocalOptIdx] == CKLocalSearch) ) ||
		   ( (LocalOptType == 'C') && (k == 0) ) )
		  CKPrepareSolution(lo, &length);
	      //printf ("Before %Ld\n", computeCost(lo));
	      while ( ( gain = localOpt(lo) ) > 0 ) {
              length += gain;
             // printf ("After %Ld<->%Ld\n", computeCost(lo), length);
              ls_iteration++;
              z = 0;

              if ( JustLS ) gain = 0;
	      }
	      LocalOptIdx = (LocalOptIdx + 1)%LocOptMax;
	  }
	  //printf("B: %Ld, CB: %Ld, L: %Ld\n", best, currbest, length);
          nonimpr++;
	  if (RandomScan) free(RandomScan); 

/*          printf("C[%Ld]", length);
          for (j=0; j < PSize; j++) 
	      printf(" %d", lo[j]);
	      puts("");*/
	  if (VeryVerbose) {
	      printf("C %Ld [%Ld]", k,length);
	      for (j=0; j < PSize; j++) 
		  printf(" %ld", lo[j]);
	      puts("");
	  }//

	  if ( (k == 0) || JustLS || (length > best) ) {
	      /* If it is the first iteration best is defined */
              nonimpr = 0;
	      best = length;
          best_time=elapsed_time(VIRTUAL);
	      if  (LocalOptType == 'C') 
		  memcpy(lobest, lo, sizeof(long int)*PSize);
	      else if ( PerturbationType == 'R' ) {
		  if (k > 0) free(lobest);
		  lobest = lo;
	      } else {
		  ptmp = lobest;
		  lobest = lo;
		  lo = ptmp;
		  memcpy(locurrbest, lobest, sizeof(long int)*PSize);
		  currbest = best;
	      }
	      printOut(best, k, ls_iteration,evaluations, elapsed_time(VIRTUAL));
	  }
	  else if ( (PerturbationType != 'R') && 
		    AcceptanceCriterion(length, currbest) ) {
	      /* It means that we use an AC and it is satisfied */
	      ptmp = locurrbest;
	      locurrbest = lo;
	      lo = ptmp;
	      currbest = length;
	      //printf("acc");
	      //printOut(length, k, ls_iteration, elapsed_time(VIRTUAL)); 
	  } else if (PerturbationType == 'R') /*Random Restart*/
		    free(lo);
	  else {
	      //printf("not");
	      //printOut(length, k, ls_iteration, elapsed_time(VIRTUAL));
	  }     
	  k++;
              
   //      fprintf(flog,"%ld, %ld, %g\n",evaluations,best,elapsed_time(VIRTUAL));
      } while (!JustLS && 
	       ( !MaxLS || (k < MaxLS) ) &&
	       ( (OptSol < 0) || (best < OptSol) ) && Continue());
      printOut(best, k, ls_iteration, evaluations,elapsed_time(VIRTUAL));

      /* printf("Expected [%d]: ", best);
	 printf("[%d]\n", computeCost(lobest)); */
  //    printf("end try %ld\n", i);
//      printf("Best%Ld\n", TotMat-best);
    //  printf("Best found[%Ld]:", best);
    //  for (k=0; k < PSize; k++)
	//  printf(" %ld", lobest[k]);
    //  puts("");
//  }
    
    /* FILE *f;
      f = fopen(FileNameResult, "w");
    fprintf(f,"Final solution: %ld", best);
    for (k=0; k < PSize; k++)
       fprintf(f,",%ld", lobest[k]);
    fprintf(f,"\n");
    fprintf(f,"Best fitness: %ld\n", best);
    fprintf(f,"Total time: %G\n",elapsed_time(VIRTUAL));
    fprintf(f,"Best time: %G\n",best_time);
    fclose(f);
*/
    FILE *f;
    f = fopen(FileNameResult,"a+");
    printf("%ld\n",Seed);
    fprintf(f,"\"%s\";%d;\"%s\";%ld;%.3f;%.3f\n",FileName,Seed,"ILSr",best,elapsed_time(VIRTUAL),best_time);
    fclose(f);

  return 0;
}













