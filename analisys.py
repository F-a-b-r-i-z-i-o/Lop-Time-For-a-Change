import pandas as pd 
import numpy as np 

df = pd.read_csv(
    "results_rxr/results_MS_combined_sorted.csv",
    sep=";",
)

# Name of instance set 
df["instance_set"] = "rxr"

# Find overall best-fitness
fit_max = df["fit_set"].str.split(",").str[0].astype(int).max()
df["best_fit"] = fit_max

# Find set best-fitness
best_fit_for_set = df["fit_set"].str.split(",").apply(lambda xs: max(map(int, xs)))
df["best_fit_for_set"] = best_fit_for_set

# Calculate rpd (relative percentage deviation) 
rpd_fitness =  (fit_max - df["best_fit_for_set"]) / fit_max # change respect to formula latex
df["rpd_fitness"] = rpd_fitness

# len set fitness
lens = df["fit_set"].str.split(",").str.len()
df["set_size"] = lens

# ratio len set / n
prec_set = df["set_size"] / df["n"] # change respect to latex formula  

# Find mean-fitness for set 
avg_fit = df["fit_set"].apply(lambda s: np.fromstring(s, sep=",").mean())
df["fit_set_mean"] = avg_fit

# Overall best-fitness
best_avg_fit = df["fit_set_mean"].max()

# Calculate mean rpd (relative percentage deviation) 
df["rpd_avg_fit"] = (best_avg_fit - avg_fit) / best_avg_fit

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


def parse_perm(s):
    """
    Parse a single permutation from a string.

    Expected format:
      - integers separated by spaces
      - example: "0 1 2 3 4"
    """
    return [int(p) for p in str(s).split()]


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


def kendall_distance_matrix(perms):
    """
    Build the m×m Kendall distance matrix for a set of m permutations.

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


def add_kendall_matrix_per_row(df, sol_col="sol_set"):
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


kendall_matrices = add_kendall_matrix_per_row(df)


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

    # Exclude self-distance (diagonal) 
    np.fill_diagonal(D2, np.inf)

    nn = np.min(D2, axis=1)  # nearest-neighbor distance for each solution
    return float(np.sum(nn)) # probably /D.shape[0]


df["delta_nn"] = [delta_nn_from_kendall_matrix(D) for D in kendall_matrices]


def solow_polasky_from_kendall_matrix(kendall_matrix):
    """
    Solow–Polasky diversity computed from a Kendall distance matrix.

    Steps:
      1) Convert distances to a similarity/correlation matrix:
           F_ij = exp(-theta * D_ij)
      2) Compute the pseudoinverse F^+
      3) Return sum(F^+) / m  (m = number of solutions in the set)

      - here 'm' is the number of permutations in the set (matrix size)
      - theta is hard-coded as m(m-1)/4 
    """
    m = len(kendall_matrix)
    theta = m * (m - 1) / 4

    kendall_matrix = np.asarray(kendall_matrix, dtype=float)
    correlation_matrix = np.exp(-theta * kendall_matrix)

    inverse_matrix = np.linalg.pinv(correlation_matrix)
 
    return float(inverse_matrix.sum() / m)

df["delta_sp"] = [solow_polasky_from_kendall_matrix(D) for D in kendall_matrices]
    
