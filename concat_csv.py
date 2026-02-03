import pandas as pd

df1 = pd.read_csv("results_rxr/results_MS_CDRVNS.csv", sep=";")
df2 = pd.read_csv("results_rxr/results_MS_MAEDM.csv", sep=";")


# concat verticale
df = pd.concat([df1, df2], ignore_index=True)

# salvo
df.to_csv("results_rxr/results_MS_combined.csv", sep=";", index=False)