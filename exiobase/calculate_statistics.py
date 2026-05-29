import sys
import os
import numpy as np
import pandas as pd
import scipy.stats as ss


def read_lop_instance(filename):
    with open(filename, 'r') as f:
        n = int(f.readline())
        A = np.array([list(map(int, f.readline().split())) for _ in range(n)])
    return A


def calculate_statistics(A):
    #n, sparsity (percentage of zeros), st dev, coefficient of variation, mean, min, q1, median, q3, max, useless_items
    #exclude diagonal and zeros (due to normal form) from the statistics
    n = A.shape[0]
    values = []
    for i in range(n-1):
        for j in range(i+1,n):
            values.append( A[i,j] if A[i,j]>0 else A[j,i] )
    values = np.array(values) #contiene n*n-n elementi
    nzeros = values[values==0].size
    mean = values.mean()
    std = values.std()
    cv = std/mean
    sparsity = np.sum(values==0) / values.size * 100
    _min = values.min()
    q1 = np.quantile(values, 0.25)
    median = np.median(values)
    q3 = np.quantile(values, 0.75)
    _max = values.max()
    useless_items = len([ i for i in range(n) if A[i,:].sum()==0 and A[:,i].sum()==0 ])
    skew = ss.skew(values)
    kurt = ss.kurtosis(values)
    return n, sparsity, std, cv, mean, _min, q1, median, q3, _max, useless_items, skew, kurt, nzeros


if len(sys.argv)<2:
    print('USAGE:')
    print('- python calculate_statistics.py <filename> [output_pickle_filename_to_append]')
    print('- python calculate_statistics.py <directory> [output_pickle_filename_to_append]')
    sys.exit()
arg_in = sys.argv[1]
arg_out = sys.argv[2] if len(sys.argv)>=3 else None

filenames = []
iset = pd.NA #instance set
if os.path.isfile(arg_in):
    filenames.append(arg_in)
elif os.path.isdir(arg_in):
    iset = arg_in
    if iset[-1] in {'/', '\\'}: iset = iset[:-1]
    if '/' in iset: iset = iset[iset.rfind('/')+1:]
    if '\\' in iset: iset = iset[iset.rfind('\\')+1:]
    for filename in os.listdir(arg_in):
        full_path = os.path.join(arg_in,filename)
        filenames.append(full_path)
else:
    sys.exit('PROVIDED PATH DOES NOT EXIST')

ninstances = 0
all_stats = []
for filename in filenames:
    ninstances += 1
    A = read_lop_instance(filename)
    stats = calculate_statistics(A)
    instance_name = filename
    if '/' in instance_name: instance_name = instance_name[instance_name.rfind('/')+1:]
    if '\\' in instance_name: instance_name = instance_name[instance_name.rfind('\\')+1:]
    all_stats.append({
            'instance_set': iset,
            'instance': instance_name,
            'n': stats[0],
            'sparsity': stats[1],
            'std': stats[2],
            'cv': stats[3],
            'mean': stats[4],
            'min': stats[5],
            'q1': stats[6],
            'median': stats[7],
            'q3': stats[8],
            'max': stats[9],
            'useless_items': stats[10],
            'skewness': stats[11],
            'kurtosis': stats[12],
            'nzeros': stats[13],
        })
df = pd.DataFrame(all_stats)

print('-'*10)
pd.set_option('display.float_format', '{:.2f}'.format)
print( df.to_string(index=False) )
print('-'*10)

print('\n\n' + '#'*10)
print(f'Avg sparsity      = {df["sparsity"].mean():.2f}')
print(f'Avg std           = {df["std"].mean():.2f}')
print(f'Avg cv            = {df["cv"].mean():.2f}')
print(f'Avg mean          = {df["mean"].mean():.2f}')
print(f'Avg min           = {df["min"].mean():.2f}')
print(f'Avg q1            = {df["q1"].mean():.2f}')
print(f'Avg median        = {df["median"].mean():.2f}')
print(f'Avg q3            = {df["q3"].mean():.2f}')
print(f'Avg max           = {df["max"].mean():.2f}')
print(f'Avg useless_items = {df["useless_items"].mean():.2f}')
print(f'Avg nzeros        = {df["nzeros"].mean():.2f}')
print('#'*10)

if arg_out is not None:
    #if already exists, so load and prepend to df
    if os.path.exists(arg_out):
        df_prev = pd.read_pickle(arg_out)
        df = pd.concat([df_prev,df])
    #save the pickle
    df.to_pickle(arg_out)
