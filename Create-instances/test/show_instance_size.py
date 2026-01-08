import sys
import os
import numpy as np


def read_instance_size(filename):
    with open(filename, 'r') as f:
        n = int(f.readline())
    return n


if len(sys.argv)<2:
    print('USAGE:')
    print('- python show_instance_size.py <filename>')
    print('- python show_instance_size.py <directory>')
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

inst_size = {}
for filename in filenames:
    n = read_instance_size(filename)
    print(f'{filename} = {n}')
    if n not in inst_size:
        inst_size[n] = 1
    else:
        inst_size[n] += 1
print('\n\n\n' + '#'*10)
for key, value in inst_size.items():
    print(f"n={key} => {value} instances")
print('#'*10)
