import pandas as pd

df1 = pd.read_csv("results_pxp/results_MS_CDRVNS.csv", sep=";")
df2 = pd.read_csv("results_pxp/results_MS_MAEDM.csv", sep=";")

df = pd.concat([df1, df2], ignore_index=True)   # concat vertical
df.to_csv("results.csv", sep=";", index=False)
