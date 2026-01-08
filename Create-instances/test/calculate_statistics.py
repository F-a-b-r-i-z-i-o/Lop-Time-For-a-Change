import sys
import os
import numpy as np
import pandas as pd


def read_lop_instance(filename):
    with open(filename, 'r') as f:
        n = int(f.readline())
        A = np.array([list(map(int, f.readline().split())) for _ in range(n)])
    return A


def calculate_statistics(A):
    #n, sparsity (percentage of zeros), st dev, coefficient of variation, mean
    #exclude diagonal from the statistics
    n = A.shape[0]
    mask = ~np.eye(n, dtype=bool)
    A_off_diag = A[mask] #è un array di dimensione n*n-n
    mean = A_off_diag.mean()
    std = A_off_diag.std()
    cv = std/mean
    sparsity = np.sum(A_off_diag==0) / A_off_diag.size * 100
    return n, sparsity, std, cv, mean


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
    if iset[-1] in {'/', '\\'}:
        iset = iset[:-1]
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
    all_stats.append({
            'instance set': iset,
            'instance': filename,
            'n': stats[0],
            'sparsity': stats[1],
            'std': stats[2],
            'cv': stats[3],
            'mean': stats[4],
        })
df = pd.DataFrame(all_stats)

print('-'*10)
pd.set_option('display.float_format', '{:.2f}'.format)
print( df.to_string(index=False) )
print('-'*10)

print('\n\n' + '#'*10)
print(f'Avg sparsity = {df["sparsity"].mean():.2f}')
print(f'Avg std      = {df["std"].mean():.2f}')
print(f'Avg cv       = {df["cv"].mean():.2f}')
print(f'Avg mean     = {df["mean"].mean():.2f}')
print('#'*10)

if arg_out is not None:
    #if already exists, so load and prepend to df
    if os.path.exists(arg_out):
        df_prev = pd.read_pickle(arg_out)
        df = pd.concat([df_prev,df])
    #save the pickle
    df.to_pickle(arg_out)













