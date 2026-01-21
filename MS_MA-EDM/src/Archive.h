#include <string>
#include <ctime>
using namespace std;



struct Archive {
	
	int m;				//size of the final archive to return
	int n;				//permutation length
	int** sol;			//solution set (matrix m+1 by n)
	unsigned long* fit;	//fitness set (vector m+1)
	int** dmat;			//unordered distance matrix (matrix m+1 by m+1)
	int** fdp;			//fitness-distance profiles (matrix m+1 by m+1)
	int size;			//current size of the archive
	clock_t start_time;	//time when constructor called
	
	Archive(int m, int n);
	~Archive();
	
	void update(int* x, unsigned long fx);
	
	void print(string filename, string algname, string instance, unsigned long seed, int nevals) ;
	
};