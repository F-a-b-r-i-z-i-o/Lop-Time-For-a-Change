import numpy as np
import pandas as pd
from scipy.stats import mannwhitneyu
from pathlib import Path


filename_in = "stats.pickle"

out2 = "tab2.tex"

df = pd.read_pickle(filename_in)

df["algname"] = df["algname"].str.replace(r"^MS-", "", regex=True)
df["best_obj_value"] = df.groupby(["instance", "m"])["fit"].transform("max")
df["rpd"] = 100 * (df["best_obj_value"] - df["fit"]) / df["best_obj_value"]

g = df.groupby(["instance", "m"])["rpd"]
df["median_rpd"] = g.transform("median")
df["rpd_p10"] = g.transform(lambda s: s.quantile(0.10))
df["rpd_p90"] = g.transform(lambda s: s.quantile(0.90))

# Best of m 
tab = (
    df.loc[df.groupby(["instance", "algname", "m"])["fit"].idxmax(),
           ["instance_set", "seed", "instance", "algname", "m",
            "fit", "best_obj_value", "rpd", "median_rpd", "rpd_p10", "rpd_p90"]]
    .reset_index(drop=True)
)

# best-of-best su m: 1 rows
best_all_m = (
    tab.loc[tab.groupby(["instance", "algname"])["fit"].idxmax()]
       .rename(columns={"m": "best_m", "fit": "best_fit"})
       .reset_index(drop=True)
)

best_all_m["instance_set"] = pd.Categorical(best_all_m["instance_set"], ["rxr","pxp","os300","other"], ordered=True)
best_all_m["_m_sort"] = pd.to_numeric(best_all_m["best_m"])
best_all_m = (best_all_m.sort_values(["instance_set","instance","seed","algname","_m_sort"])
                        .drop(columns=["_m_sort","instance_set", "seed", "best_m", "best_fit"])
                        .reset_index(drop=True))


latex2 = best_all_m.to_latex(index=False, escape=False, longtable=True, float_format="{:.3f}".format)
Path(out2).write_text(latex2, encoding="utf-8")
print(f"Wrote: {out2}")



def mann_whitney_test(df: pd.DataFrame) -> pd.DataFrame:
    ALG_A = "CD-RVNS"
    ALG_B = "MA-EDM"
    value_col = "fit"
    alpha = 0.05

    rows = []
    sub = df[df["algname"].isin([ALG_A, ALG_B])]

    for inst, gg in sub.groupby("instance"):
        x = gg.loc[gg["algname"] == ALG_A, value_col].to_numpy()
        y = gg.loc[gg["algname"] == ALG_B, value_col].to_numpy()

        if x.size == 0 or y.size == 0:
            continue

        res = mannwhitneyu(x, y)

        med_a = float(np.median(x))
        med_b = float(np.median(y))
        delta = med_a - med_b

        winner_median = ALG_A if delta > 0 else (ALG_B if delta < 0 else "tie")
        winner_sig = winner_median if res.pvalue < alpha else "ns"

        rows.append(
            {
                "instance": inst,
                "n_CDRVNS": int(x.size),
                "n_MA_EDM": int(y.size),
                "median_CDRVNS": med_a,
                "median_MA_EDM": med_b,
                "delta_median": delta,
                "U": float(res.statistic),
                "pvalue": float(res.pvalue),
                "winner_median": winner_median,
                "winner_sig": winner_sig,
            }
        )

    return pd.DataFrame(rows).sort_values("instance").reset_index(drop=True)


mwu_by_instance = mann_whitney_test(df)

print(
    mwu_by_instance[
        ["instance", "median_CDRVNS", "median_MA_EDM", "delta_median", "pvalue", "winner_sig"]
    ]
)
print(mwu_by_instance["winner_sig"].value_counts())


