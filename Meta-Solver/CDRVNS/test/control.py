import numpy as np 

def _control_idx(file_path):
    with open(file_path) as f:
        idx = f.read().strip().split()
        perm = [int(x) for x in idx]      
    return perm

def read_lop_instance(path):
    with open(path, "r") as f:
        first_line = f.readline().strip().split()
        n = int(first_line[0])

        values = []
        for line in f:
            parts = line.strip().split()
            if not parts:
                continue
            values.extend(map(float, parts)) 
            if len(values) >= n * n:
                break

    C = np.array(values[:n*n]).reshape((n, n))
    return C

def check_duplicate_trivial(items):
    for idx in range(len(items)):
        for inner in range(len(items)):
            if inner == idx:
                continue  # do not compare to itself
            if items[inner] == items[idx]:
                return True
    return False

def lop_fitness(C, perm):
    """Fitness LOP: sum_{i<j} C[perm[i], perm[j]]"""
    n = len(perm)
    val = 0.0
    for i in range(n - 1):
        pi_i = perm[i]
        for j in range(i + 1, n):
            pi_j = perm[j]
            val += C[pi_i, pi_j]
    return val


if __name__ == "__main__":
    C = read_lop_instance("/home/fabrizio/Scrivania/EXIO-ALGO/Create-instances/sub_matrix_IT_Z")
    listidx = _control_idx("results_cpp.txt")
    check = check_duplicate_trivial(listidx)    
    print(check)
    val_fitenss = lop_fitness(C, listidx)
    print(val_fitenss)