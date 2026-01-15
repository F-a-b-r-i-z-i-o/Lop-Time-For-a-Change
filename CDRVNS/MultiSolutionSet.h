struct MultiSolutionSet {
	
	MultiSolutionSet(int m);
	~MultiSolutionSet();
	
	void update_set(int* x, unsigned long fx);
	void print_result(int seed, string algorithm, int m, string name_instance, int nevals, double time, vector<int> solution_set, vector<double> fitness);
	
	
};
