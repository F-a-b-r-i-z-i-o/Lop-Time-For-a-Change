import numpy as np
import pandas as pd
from scipy.stats import mannwhitneyu, false_discovery_control
from pathlib import Path
import matplotlib.pyplot as plt 
import seaborn as sns 
import matplotlib.ticker as mticker


# =========================
#  Long table
# =========================

filename_in = "results/stats.pickle"
out = "results-ss-table.tex"

df = pd.read_pickle(filename_in)
df["algname"] = df["algname"].str.replace(r"^MS-", "", regex=True)

# Compute per-(instance, algname) RPD statistics and broadcast them to each row
grp = df.groupby(["instance", "algname"])["rpd"]
df["median_rpd"] = grp.transform("median")
df["q1_rpd"] = grp.transform(lambda s: s.quantile(0.25))
df["q3_rpd"] = grp.transform(lambda s: s.quantile(0.75))
df["IQR_RPD"] = df["q3_rpd"] - df["q1_rpd"]

# Keep only one row per instance: the run with maximum fit
idx = df.groupby("instance")["fit"].idxmax()

tab = df.loc[idx, ["instance_set", "instance", "algname", "fit", "median_rpd", "IQR_RPD"]].copy()
tab = tab.rename(columns={"fit": "best_obj_values"})

# Order instance sets
tab["instance_set"] = pd.Categorical(
    tab["instance_set"],
    ["rxr", "pxp", "os300", "other"],
    ordered=True
)

tab = tab.sort_values(["instance_set", "instance"]).reset_index(drop=True)

print("Rows table:", len(tab))

latex = tab.to_latex(
    index=False,
    escape=False,
    longtable=True,
    float_format="{:.3f}".format
)

Path(out).write_text(latex, encoding="utf-8")
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
# mann_whitney TEST
# =========================

def mann_whitney_test(df: pd.DataFrame) -> pd.DataFrame:
    ALG_A = "CD-RVNS"
    ALG_B = "MA-EDM"
    value_col = "fit"
    alpha = 0.05

    rows = []
    sub = df[df["algname"].isin([ALG_A, ALG_B])]

    for inst, gg in sub.groupby("instance", sort=True):
        x = gg.loc[gg["algname"] == ALG_A, value_col].to_numpy()
        y = gg.loc[gg["algname"] == ALG_B, value_col].to_numpy()
      
        res = mannwhitneyu(x, y, alternative="two-sided", method="auto")

        U = float(res.statistic)
        mid = (x.size * y.size) / 2.0

        winner_dir = (
            ALG_A if U > mid else
            ALG_B if U < mid else
            "tie"
        )
        
        rows.append(
            {
                "instance": inst,
                "U": float(res.statistic),
                "pvalue": float(res.pvalue),
                "winner_dir": winner_dir,
            }
        )

    out = pd.DataFrame(rows).sort_values("instance").reset_index(drop=True)

    # FDR correction (Benjamini-Hochberg)
    out["pvalue_fdr"] = false_discovery_control(out["pvalue"].to_numpy(), method="bh")

    # winner SOLO se significativo (altrimenti ns)
    out["winner_sig"] = np.where(out["pvalue_fdr"] < alpha, out["winner_dir"], "ns")
    return out


mwu_by_instance = mann_whitney_test(df)

print(mwu_by_instance[["instance", "pvalue", "pvalue_fdr", "winner_sig"]])
print(mwu_by_instance["winner_sig"].value_counts())


