import numpy as np
import itertools

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

def is_valid_permutation(perm):
    n = len(perm)
    return sorted(perm) == list(range(n))

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

def best_perm_bruteforce(C):
    n = C.shape[0]
    best_perm = None
    best_val = -float("inf")
    for perm in itertools.permutations(range(n)):
        val = lop_fitness(C, perm)
        if val > best_val:
            best_val = val
            best_perm = perm
    return list(best_perm), best_val

if __name__ == "__main__":
    instance_path = "/home/fabrizio/Scrivania/EXIO-ALGO/Create-instances/sub_matrix_IT_Z_10x10"

    C = read_lop_instance(instance_path)
    n = C.shape[0]
    print("n from matrix:", n)
    print("Use brute force exact (n! permutations)...")

    best_perm, best_val = best_perm_bruteforce(C)

    print("Permutation valid?", is_valid_permutation(best_perm))
    print("Best permutation:", best_perm)
    print("Best fitness:", best_val)
