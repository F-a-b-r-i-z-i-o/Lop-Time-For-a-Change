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
prec_set = df["set_size"] / df["m"] # Probably n 

# Find mean-fitness for set 
avg_fit = df["fit_set"].apply(lambda s: np.fromstring(s, sep=",").mean())
df["fit_set_mean"] = avg_fit

# Overall best-fitness
best_avg_fit = df["fit_set_mean"].max()

# Calculate mean rpd (relative percentage deviation) 
df["rpd_avg"] = (best_avg_fit - avg_fit) / best_avg_fit

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
    """Parse one permutation: numbers separated by spaces."""
    return [int(p) for p in str(s).split()]


def parse_sol_set_cell(sol_set_str):
    """
    Parse a cell like:
      '0 1 2, 2 1 0, ...'
    into: [perm1, perm2, ...]
    """
    parts = [p.strip() for p in str(sol_set_str).split(",") if p.strip()]
    return [parse_perm(p) for p in parts]


def kendall_distance_matrix(perms):
    """m x m Kendall distance matrix among perms."""
    m = len(perms)
    D = np.zeros((m, m), dtype=np.int32)
    for i in range(m):
        for j in range(i + 1, m):
            d = kendall_tau(perms[i], perms[j])
            D[i, j] = d
            D[j, i] = d
    return D


def add_kendall_matrix_per_row(df, sol_col="sol_set", out_col="kendall_matrix"):
    """
    For each row:
      - read sol_set string -> list of permutations (split by comma)
      - compute Kendall distance matrix m x m
      - store it in df[out_col]
    """
    df = df.copy()
    df[out_col] = None

    for idx in df.index:
        perms = parse_sol_set_cell(df.at[idx, sol_col])

        # DEBUG
        # m_expected = int(df.at[idx, m_col])
        # if len(perms) != m_expected:
        #     print(f"Warning row {idx}: m={m_expected} found {len(perms)} permutations in the cell")

        df.at[idx, out_col] = kendall_distance_matrix(perms)

    return df


def solow_polasky(perms):
    
    perms = list(perms)
    if not perms:
        return 0.0

    D = kendall_distance_matrix(perms)
    activity_param = (n * n-1)/4
  
    F = np.exp(-activity_param * D)

    invF = np.linalg.pinv(F)
    
    return float(invF.sum())

