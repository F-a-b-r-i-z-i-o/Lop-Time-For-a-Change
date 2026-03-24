import numpy as np
import pandas as pd
import os


#Read LOP instance matrix
def read_lop_instance(filename):
    with open(filename, 'r') as f:
        n = int(f.readline())
        A = np.array([list(map(int, f.readline().split())) for _ in range(n)])
    return A


#Generate the swaps for the Steinhaus-Johnson-Trotter algorithm.
#code taken from https://ics.uci.edu/~eppstein/PADS/Permutations.py
def sjt_swaps(n):
    if n < 1:
        return
    up = range(n-1)
    down = range(n-2,-1,-1)
    recur = sjt_swaps(n-1)
    try:
        while True:
            yield from down
            yield next(recur) + 1
            yield from up
            yield next(recur)
    except StopIteration:
        pass


#Run exhaustive search using Steinhaus-Johnson-Trotter and return number of global optima
def exhaustive_search(A):
    n = A.shape[0]
    x = np.arange(n)
    fopt = fx = np.triu(A,k=1).sum()
    nopt = 1
    for i in sjt_swaps(n):
        x[i],x[i+1] = x[i+1],x[i]
        fx += +A[x[i],x[i+1]] -A[x[i+1],x[i]]
        if fx>fopt:
            fopt = fx
            nopt = 1
        elif fx==fopt:
            nopt += 1
    return nopt


#Read all isic instances
inst_dir = 'exiobase/isic'
As = {}
for filename in os.listdir(inst_dir):
    filepath = os.path.join(inst_dir, filename)
    A = read_lop_instance(filepath)
    As[filename] = A


#Initialize random number generator for reproducibility
np.random.seed(42)


#Run the experiment
n = 10
nentries = int(n*(n-1)/2)
data = []
for inst,A in As.items():
    initial_nzeros = nentries - A[A>0].size
    for nzeros in range(initial_nzeros,nentries+1):
        sp = nzeros/nentries
        if nzeros>initial_nzeros:
            pos_ind = np.argwhere(A>0)
            npos = pos_ind.shape[0]
            if npos>0:
                r = np.random.randint(npos)
                i,j = pos_ind[r]
                A[i,j] = 0
        nopt = exhaustive_search(A)
        data.append({
                'instance': inst,
                'nzeros':   nzeros,
                'sparsity': sp,
                'nopt':     nopt,
            })
        print(f'* nzeros={nzeros} sp={sp} inst={inst} nopt={nopt}')


#Build the dataframe and save it
df = pd.DataFrame(data)
df.to_pickle('results/isic_exp2.pickle')
