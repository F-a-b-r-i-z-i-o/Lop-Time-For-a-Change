import pandas as pd 
from diversipy import indicator
import re
import numpy as np 
from scipy.stats import kendalltau

df = pd.read_csv(
    "results.csv",
    sep=";",
)

# Clean column names 
df.columns = [c.strip() for c in df.columns]

# convert to numerics
df["seed"] = pd.to_numeric(df["seed"], errors="coerce")
df["m"] = pd.to_numeric(df["m"], errors="coerce")

# Keep only seeds 1..30
df_1_30 = df[df["seed"].between(1, 30)].copy()

# m == 5 
m5 = df_1_30[df_1_30["m"] == 5].copy()

# mean for row
fit_means = []

for s in m5["fit_set"]:
    vals = str(s).split(",")        # clean data
    vals = [float(x) for x in vals] # convert to number 
    mean = sum(vals) / len(vals)   # mean for rows
    fit_means.append(mean)

# Local mean 
m5["fit_set_mean"] = fit_means

# Global mean 
global_mean = m5["fit_set_mean"].mean()

print(m5[["fit_set", "fit_set_mean"]])
print("Global:", global_mean)


