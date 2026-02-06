import pandas as pd 
import numpy as np 
import sys


# Command line arguments
'''
if len(sys.argv)<3:
    sys.exit('USAGE: python analyze.py <RESULTS_CSV> <RESULTS_ELABORATED_PICKLE>')
filename_in = sys.argv[1]
filename_out = sys.argv[2]
'''
filename_in = 'all_results.csv'


# Read csv
df = pd.read_csv( filename_in, sep=";" )

# Adjust name of the instance
df['instance'] = df.apply(
                        lambda row: row['instance'].split('/')[-1],
                        axis=1 )

# Name of instance set 
df['instance_set'] = df.apply(
                        lambda row: row['instance'].split('/')[-1].split('_')[0],
                        axis=1 )

# Find set best-fitness
df['fit'] = df.apply(
                lambda row: np.max([ int(s) for s in row['fit_set'].split(',') ]),
                axis=1 )

# Instace best-fitness
df['best'] = df.groupby('instance')['fit'].transform('max')

# Calculate rpd (relative percentage deviation) 
df['rpd'] = 100 * (df['best'] - df['fit']) / df['best']

# Length set fitness
df["set_size"] = df.apply(
                    lambda row: len([ _ for _ in row['fit_set'].split(',') ]),
                    axis=1 )

# Ratio len set / m
df['set_size_perc'] = df['set_size'] / df['m']

# Find mean-fitness for set 
df['avg_fit'] = df.apply(
                lambda row: np.mean([ int(s) for s in row['fit_set'].split(',') ]),
                axis=1 )

# Instance avg best-fitness
df['best_avg'] = df.groupby('instance')['avg_fit'].transform('max')

# Calculate rpd on avg best fitness (relative percentage deviation)
df['rpd_avg'] = 100 * (df['best_avg'] - df['avg_fit']) / df['best_avg'] 

# Kendall-tau distance
def kendall_tau(x, y):
    """Number of disagreements (inversions) between permutations x and y."""
    n = len(x)
    pos_in_y = [0] * n
    for i, val in enumerate(y):
        pos_in_y[val] = i
    tau = 0
    for i in range(n - 1):
        xi = x[i]
        for j in range(i + 1, n):
            if pos_in_y[xi] > pos_in_y[x[j]]:
                tau += 1
    return tau

# Function to parse a permutation
def parse_perm(s):
    """
    Parse a single permutation from a string.

    Expected format:
      - integers separated by spaces
      - example: "0 1 2 3 4"
    """
    return [int(p) for p in str(s).split()]

# Function to parse a set of permutations
def parse_sol_set_cell(sol_set_str):
    """
    Parse one dataframe cell that contains multiple permutations.

    Expected format:
      - permutations separated by commas
      - within each permutation, integers separated by spaces
    Returns:
      - list[list[int]] where each inner list is one permutation
    """
    parts = [p.strip() for p in str(sol_set_str).split(",") if p.strip()]
    return [parse_perm(p) for p in parts]

# Function to build Kendall tau distance matrix from a set of permutations
def kendall_distance_matrix(perms):
    """
    Build the mxm Kendall distance matrix for a set of m permutations.

    D[i, j] = kendall_tau(perms[i], perms[j])
    """
    m = len(perms)
    D = np.zeros((m, m), dtype=np.int32)
    for i in range(m):
        for j in range(i + 1, m):
            d = kendall_tau(perms[i], perms[j])
            D[i, j] = d
            D[j, i] = d
    return D

# Calculate one Kendall tau matrix for each row of the dataframe
def series_of_kendall_matrices(df, sol_col="sol_set"):
    """
    Compute one Kendall distance matrix per dataframe row.

    Each row contains a "solution set" (sol_set) encoded as:
      - m permutations separated by commas

    Returns:
      - list[np.ndarray], same order as df rows
      - result[i] is the Kendall matrix for df.iloc[i]
    """
    matrices = []
    for idx in df.index:
        perms = parse_sol_set_cell(df.at[idx, sol_col])
        matrices.append(kendall_distance_matrix(perms))
    return matrices

# Series of kendall matrices
kendall_matrices = series_of_kendall_matrices(df)

# Function to calculate Delta_NN (unnormalized)
def delta_nn_from_kendall_matrix(D):
    """
    Nearest-neighbor diversity (Delta_NN) computed from a distance matrix.

    Definition:
      Delta_NN(S) = sum_i  min_{j != i} D[i, j]

      - low value => solutions are clustered (each has a very close neighbor)
      - high value => solutions are spread out
    """
    D = np.asarray(D, dtype=float)
    D2 = D.copy()
    np.fill_diagonal(D2, np.inf) # Exclude self-distance (diagonal) 
    nn = np.min(D2, axis=1)  # nearest-neighbor distance for each solution
    return int(np.sum(nn))

# Delta_NN unnormalized and normalized
df["delta_nn"] = [ delta_nn_from_kendall_matrix(D) for D in kendall_matrices ]
df['delta_nn_norm'] = df["delta_nn"] / ( df.m * df.n * (df.n-1) / 2 )

# Function to calculate Solow Polasky diversity
def solow_polasky_from_kendall_matrix(kendall_matrix, n):
    """
    Solow–Polasky diversity computed from a Kendall distance matrix.

    Steps:
      1) Convert distances to a similarity/correlation matrix:
           F_ij = exp(-theta * D_ij)
      2) Compute the inverse F^-1
      3) Return sum(F^-1)

      - here 'n' is the length of a permutation
      - theta is hard-coded
    """
    #theta = 1
    #theta = 0.5
    #theta = np.log(2) / (n*(n-1)/4) #midpoint anchoring #Theta was chosen so that permutations differing on half of all pairwise comparisons had similarity 0.5. #Half of the maximum Kendall disagreement corresponds to 50% similarity
    #theta = np.log(0.01) / (n*(n-1)/2) #maxdistance anchoring
    theta = np.log(2) / ( 0.005 * (n*(n-1)/2) ) #midpoint anchoring to typical value (experimentally computed)
    C = np.exp(-theta*kendall_matrix)
    C_inv = np.linalg.inv(C)
    return C_inv.sum()

# Delta_SP unnormalized and normalized
df["delta_sp"] = [ solow_polasky_from_kendall_matrix(D,n) for D,n in zip(kendall_matrices,df['n']) ]
df['delta_sp_norm'] = df.delta_sp / df.m


# ACCURACY PER FITNESS = quante fitness del set stanno nelle migliori m fitness mai trovate per l'istanza (normalizzato con m)
'''
df['sol_fit_dict'] = df.apply(
                        lambda row: dict(zip(row['sol_set'].split(','), [int(s) for s in row['fit_set'].split(',')])),
                        axis=1 )
def merge_dicts(dicts):
    result = {}
    for d in dicts:
        result.update(d)
    return result



all_fits = df.groupby('instance')['fit_set'].transform(','.join)
all_fits = [ s.split(',') for s in all_fits ]
all_fits = [ sorted([ int(s) for s in l ], reverse=True) for l in all_fits ]
df['all_fits'] = all_fits
def accuracy(row):
    all_fits = row.all_fits
    fit_set = [ int(s) for s in row.fit_set.split(',') ]
    m = row.m
    cnt = 0
    for fit in fit_set:
        if fit in all_fits[:m]:
            cnt += 1
    return cnt/m
df['accuracy'] = df.apply( accuracy, axis=1 )
df.drop(columns=["all_fits"], inplace=True)
'''

# Remove sol_set column to save memory
df.drop(columns=["sol_set"], inplace=True)

# Save to pickle file
df.to_pickle(filename_out)
