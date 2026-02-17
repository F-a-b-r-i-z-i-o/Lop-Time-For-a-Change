import numpy as np
import pandas as pd
from scipy.stats import mannwhitneyu
from pathlib import Path
import matplotlib.pyplot as plt 
import seaborn as sns 
import matplotlib.ticker as mticker
from matplotlib.ticker import ScalarFormatter
import matplotlib.patheffects as pe

# =========================
#  Long table
# =========================

filename_in = "stats.pickle"
out = "results-ss-table.tex"

df = pd.read_pickle(filename_in)
df["algname"] = df["algname"].str.replace(r"^MS-", "", regex=True)

# best per (instance,m) and RPD
df["best_obj_value"] = df.groupby(["instance","m"])["fit"].transform("max")
df["rpd"] = 100 * (df["best_obj_value"] - df["fit"]) / df["best_obj_value"]

# median e IQR of RPD per (instance,m)
g = df.groupby(["instance","m"])["rpd"]
df["median_rpd"] = g.transform("median")
df["rpd_iqr"] = g.transform(lambda s: s.quantile(0.75) - s.quantile(0.25))

#  one row per (instance, algname, m): fit max 
tab = df.loc[
    df.groupby(["instance","algname","m"])["fit"].idxmax(),
    ["instance_set","instance","algname","m","fit",
     "best_obj_value","rpd","median_rpd","rpd_iqr"]
].copy()

# order 
tab["instance_set"] = pd.Categorical(tab["instance_set"],
                                     ["rxr","pxp","os300","other"],
                                     ordered=True)


# best for m for (instance, algname)
best_per_alg = tab.loc[tab.groupby(["instance","algname"])["fit"].idxmax()].copy()

# best for 2 algo 
best = best_per_alg.loc[best_per_alg.groupby("instance")["fit"].idxmax()].copy()

# order and save
best = (best.sort_values(["instance_set","instance"])
            .reset_index(drop=True))

print("Rows table:", len(best))  

Path(out).write_text(
    best.to_latex(index=False, escape=False, longtable=True,
                  float_format="{:.3f}".format),
    encoding="utf-8"
)
print(f"Wrote: {out}")

# Create the boxplot
plt.figure(figsize=(8, 6))
ax = sns.boxplot(
    data        = df,
    x           = 'instance_set',
    y           = 'rpd',
    hue         = 'algname',
    order       = [ 'rxr', 'pxp', 'os300' ],
    gap         = 0.05,
    fliersize   = 1.50,
    #linewidth   = 0.05,
)
ax.set_xlabel(r'Instance Set')
ax.set_ylabel('Relative Percentage Deviation')
ax.legend(title='Algorithm')
ax.set_yscale('symlog', linthresh=1e-2)
ax.yaxis.set_major_locator(
    mticker.SymmetricalLogLocator(
        base=10,
        linthresh=1e-2
    )
)
ax.yaxis.set_minor_locator(
    mticker.SymmetricalLogLocator(
        base=10,
        linthresh=1e-2,
        subs=[1,2,3,4,5,6,7,8,9]
    )
)
ax.set_ylim(-1e-3, 1.0)
plt.legend(loc='upper left')
plt.tight_layout()
#plt.show()
plt.savefig('fig_ss_boxplot.pdf')
plt.close()


# =========================
# Isic lineplot
# =========================

df_isic = pd.read_pickle("isic_exp2.pickle")

df_plot = df_isic.copy()
df_plot["instance"] = "isic"

g = sns.relplot(
    data=df_plot,
    x="sparsity",
    y="nopt",
    kind="line",
    hue="instance",
    palette=["tab:red"],
    legend=True,
)

ax = g.ax
ax.set(xlabel="Sparsity", ylabel="Numbers of Optima")
ax.ticklabel_format(axis="both", style="plain", useOffset=False)

sns.move_legend(g, "upper center", title="Instance")


g.figure.tight_layout()
g.figure.savefig("fig_isic_lineplot.pdf", bbox_inches="tight")
plt.show()
plt.close(g.figure)



# =========================
# mann_whitney TEST
# =========================

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


