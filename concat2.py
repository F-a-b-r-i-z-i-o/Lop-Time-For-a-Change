import pandas as pd

files = [
    "results_os300/results_MS_CDRVNS.csv",
    "results_os300/results_MS_MAEDM.csv",
    "results_pxp/results_MS_CDRVNS.csv",
    "results_pxp/results_MS_MAEDM.csv",
    "results_rxr/results_MS_CDRVNS.csv",
    "results_rxr/results_MS_MAEDM.csv",
]

combined = pd.concat([pd.read_csv(f, sep=";") for f in files], ignore_index=True)

# sort 
combined = combined.sort_values(by="seed").reset_index(drop=True)

combined.to_csv("all_results.csv", sep=";", index=False)
print("Saved: all_results.csv")
