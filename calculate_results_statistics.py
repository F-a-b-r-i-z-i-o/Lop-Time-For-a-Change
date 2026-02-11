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
filename_out = 'stats.pickle'

# Read csv
df = pd.read_csv( filename_in, sep=";" )

df["algname"] = df["algname"].replace("MS-CDRVNS", "MS-CD-RVNS")

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
df['phi'] = df.apply(
                lambda row: np.mean([ int(s) for s in row['fit_set'].split(',') ]),
                axis=1 )

# Instance avg best-fitness
df['best_phi'] = df.groupby('instance')['phi'].transform('max')

# Calculate rpd on avg best fitness (relative percentage deviation)
df['rpd_phi'] = 100 * (df['best_phi'] - df['phi']) / df['best_phi'] 

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

# List of kendall matrices (THIS PART IS SLOW!!!)
kendall_matrices = []
sol_set_arr = df["sol_set"].to_numpy()

for sol_set in sol_set_arr:
    perms = [np.fromstring(p, sep=" ", dtype=np.int32) for p in sol_set.split(",")]
    KM = kendall_distance_matrix(perms)
    kendall_matrices.append(KM)

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
df["delta_nn"] = [ delta_nn_from_kendall_matrix(KM) for KM in kendall_matrices ]
df['delta_nn_norm'] = df["delta_nn"] / ( df.m * df.n * (df.n-1) / 2 )

# Function to calculate Solow Polasky diversity
def solow_polasky_from_kendall_matrix(kendall_matrix, theta=1.):
    """
    Solow–Polasky diversity computed from a Kendall distance matrix.

    Steps:
      1) Convert distances to a similarity/correlation matrix:
           F_ij = exp(-theta * D_ij)
      2) Compute the inverse F^-1
      3) Return sum(F^-1)

      - here 'n' is the length of a permutation
      - theta is the sensibility parameter
    """
    #theta = np.log(2) / ( 0.005 * (n*(n-1)/2) ) #midpoint anchoring to typical value (experimentally computed)
    C = np.exp(-theta*kendall_matrix)
    C_inv = np.linalg.inv(C)
    return C_inv.sum()

# Calculate theta (for Solow-Polasky) using midpoint anchoring to the median of the normalized Kendall distances (considering only off-diagonals, i.e., non-zero distances)
# ... in this way 0.5 similarity is assigned to the median normalized Kendall distance
norm_distances = np.concatenate(
    [ (KM / (KM.shape[0] * (KM.shape[0] - 1) / 2)).ravel() for KM in kendall_matrices ]
)
theta = np.log(2) / np.median(norm_distances[norm_distances>0])

# Delta_SP unnormalized and normalized
df["delta_sp"] = [ solow_polasky_from_kendall_matrix(KM,theta) for KM in kendall_matrices ]
df['delta_sp_norm'] = (df.delta_sp-1.) / (df.m-1) #in [0,1]



###BEGIN CALCULATION ACCURACY FOR FITNESS
# To calculate accuracy for fitness, I need to create a dictionary {solution:fitness}
df['sol_fit_dict'] = df.apply(
                        lambda row: dict( zip(row['sol_set'].split(','), [int(f) for f in row['fit_set'].split(',')]) ),
                        axis=1 )
# Function to be used with groupby ... transform
def top_fits_from_distinct_solutions(dicts):
    #merge all dictionaries into one
    all_sol_fit = {}
    for d in dicts:
        all_sol_fit.update(d)
    #now all fitnesses belong to different solutions, so just take the top Q
    Q = 15
    top_fits = sorted(all_sol_fit.values(), reverse=True)[:Q]
    #return string version of top_fits otherwise transform() complains
    return ','.join([ str(f) for f in top_fits ])
# Add column top_fits which contains the top m fitnesses (of the instance) in each row
df['top_fits'] = df.groupby('instance')['sol_fit_dict'].transform(top_fits_from_distinct_solutions)
df['top_fits'] = df.apply(
                        lambda row: [ int(f) for f in row.top_fits.split(',')[:row.m] ],
                        axis = 1 )
# Now calculate accuracy as the number of fitnesses in fit_set that belong to top_fits
def calc_acc_fit(row):
    #fitnesses as int
    fits = [ int(f) for f in row.fit_set.split(',') ]
    #count how many fitnesses are in top_fits
    cnt = len([ f for f in fits if f in row.top_fits ])
    #return accuracy
    return cnt / row.m
df['acc_fit'] = df.apply( calc_acc_fit, axis=1 )
# Remove columns which are no more useful
df.drop(columns=["sol_fit_dict"], inplace=True)
df.drop(columns=["top_fits"], inplace=True)
###END CALCULATION ACCURACY FOR FITNESS

# Remove sol_set and fit_set columns to save memory
df.drop(columns=["sol_set"], inplace=True)
df.drop(columns=["fit_set"], inplace=True)

# Save to pickle file
df.to_pickle(filename_out)