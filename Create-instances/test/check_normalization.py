import sys
import os
import numpy as np


def read_lop_instance(filename):
    with open(filename, 'r') as f:
        n = int(f.readline())
        A = np.array([list(map(int, f.readline().split())) for _ in range(n)])
    return A


def is_non_negative(A):
    n = A.shape[0]
    for i in range(n-1):
        for j in range(i+1,n):
            if A[i,j]<0:
                print(f'WE HAVE A PROBLEM: Negative number at indexes {i},{j}')
                return False
    return True


def is_diagonal_zero(A):
    n = A.shape[0]
    for i in range(n):
        if A[i,i]!=0:
            print(f'WE HAVE A PROBLEM: Diagonal not zero at index {i}')
            return False
    return True


def is_normalized(A):
    n = A.shape[0]
    for i in range(n-1):
        for j in range(i+1,n):
            if A[i,j]!=0 and A[j,i]!=0:
                print(f'WE HAVE A PROBLEM: Both A_ij and A_ji not zero at indexes {i},{j}')
                return False
    return True


if len(sys.argv)<2:
    print('USAGE:')
    print('- python check_normalization.py <filename>')
    print('- python check_normalization.py <directory>')
    sys.exit()
arg = sys.argv[1]

filenames = []
if os.path.isfile(arg):
    filenames.append(arg)
elif os.path.isdir(arg):
    for filename in os.listdir(arg):
        full_path = os.path.join(arg,filename)
        filenames.append(full_path)
else:
    sys.exit('PROVIDED PATH DOES NOT EXIST')

ninstances = 0
negatives = 0
diagonal_not_zero = 0
symmetric_entries_not_ok = 0
for filename in filenames:
    ninstances += 1
    print('-'*10)
    print(filename)
    A = read_lop_instance(filename)
    if is_non_negative(A):
        print('NON-NEGATIVITY IS OK')
    else:
        negatives += 1
    if is_diagonal_zero(A):
        print('DIAGONAL IS ZERO')
    else:
        diagonal_not_zero += 1
    if is_normalized(A):
        print('SYMMETRIC ENTRIES ARE OK')
    else:
        symmetric_entries_not_ok += 1
    print('-'*10)
print('\n\n\n' + '#'*10)
print(f'NEGATIVES = {negatives}/{ninstances}')
print(f'DIAGONAL NOT ZERO = {diagonal_not_zero}/{ninstances}')
print(f'SYMMETRIC ENTRIES NOT OK = {symmetric_entries_not_ok}/{ninstances}')
print('#'*10)