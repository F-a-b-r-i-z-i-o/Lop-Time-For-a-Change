import pandas as pd 
import numpy as np 

df = pd.read_csv(
    "results_MS_combined_sorted.csv",
    sep=";",
)

df["instance_set"] = "rxr"
fit_max = df["fit_set"].str.split(",").str[0].astype(int).max()
df["best_fit"] = fit_max
best_fit_for_set = df["fit_set"].str.split(",").apply(lambda xs: max(map(int, xs)))
df["best_fit_for_set"] = best_fit_for_set
rpd_fitness =  (fit_max - df["best_fit_for_set"]) / fit_max # change respect to formula latex
df["rpd_fitness"] = rpd_fitness
lens = df["fit_set"].str.split(",").str.len()
df["set_size"] = lens
prec_set = df["set_size"] / df["m"] # Probably n 
avg_fit = df["fit_set"].apply(lambda s: np.fromstring(s, sep=",").mean())
df["fit_set_mean"] = avg_fit
best_avg_fit = df["fit_set_mean"].max()
df["rpd_avg"] = (best_avg_fit - avg_fit) / best_avg_fit
