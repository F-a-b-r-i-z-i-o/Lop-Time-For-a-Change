import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
from scipy.stats import mannwhitneyu
from pathlib import Path
import matplotlib.ticker as mticker


filename_in = "stats.pickle"

# ---- Load & preprocess ----
df = pd.read_pickle(filename_in)


df["algname"] = df["algname"].str.replace(r"^MS-", "", regex=True)

# df["fit"] = df["fit_set"].apply(lambda s: np.max([int(x) for x in str(s).split(",")]))
# df["best_obj_value"] = df.groupby(["instance", "m"])["fit"].transform("max")
# df["rpd"] = 100 * (df["best_obj_value"] - df["fit"]) / df["best_obj_value"]

# g = df.groupby(["instance", "m"])["rpd"]
# df["median_rpd"] = g.transform("median")
# df["rpd_p10"] = g.transform(lambda s: s.quantile(0.10))
# df["rpd_p90"] = g.transform(lambda s: s.quantile(0.90))


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

### box for boxplot 
plt.figure(figsize=(8, 6))
metrics = ["rpd_phi", "delta_nn", "delta_sp"]

df_long = df.melt(
    id_vars=["instance_set", "algname", "m"],
    value_vars=metrics,
    var_name="metric",
    value_name="value",
)

g = sns.catplot(
    data=df_long,
    row="instance_set",
    col="metric",
    x="m",
    y="value",
    hue="algname",
    kind="box",
    row_order=["rxr", "pxp", "os300"],
    col_order=metrics,
    sharey=False
)

g.set_axis_labels("m", "RPD")
g.set_titles(row_template="{row_name}", col_template="{col_name}")
g.tight_layout()
plt.show()
#g.fig.savefig("fig_metrics_by_set.pdf")
plt.close()


# Sacatter plot
selected_instances = [
    "rxr_2021_n49",
    "rxr_2020_n49",
    "rxr_2019_n49",
    "pxp_kr_2022_n149",
    "pxp_ch_2022_n141",
    "pxp_pl_2022_n166",
    "os300_wa_2022_n300",
    "os300_bg_2022_n300",
    "os300_cn_2022_n300",
]

metrics = ["phi"]

df_sel = df[df["instance"].isin(selected_instances)].copy()

df_long = df_sel.melt(
    id_vars=["instance_set", "instance", "algname", "m", "delta_nn"],
    value_vars=metrics,
    var_name="metric",
    value_name="value",
)

g = sns.relplot(
    data=df_long,
    col="instance",
    col_wrap=3,              # 3x3
    x="delta_nn",
    y="value",               # phi
    hue="algname",
    style="m",           # marker for m
    kind="scatter",
    alpha=0.7,
    col_order=selected_instances,
    facet_kws={"sharex": False, "sharey": False},
)

g.set_axis_labels("delta_nn", "phi")
g.set_titles(col_template="{col_name}")
g.tight_layout()
plt.show()
plt.close()


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


def export_df_to_latex(df: pd.DataFrame, out_tex: str) -> None:
    out_path = Path(out_tex)
    max_cols = int(df.shape[1]) + 1

    with pd.option_context(
        "display.max_columns",
        max_cols,
        "display.max_colwidth",
        None,
    ):
        latex = df.to_latex(index=False, escape=False, longtable=True)

    out_path.write_text(latex, encoding="utf-8")


mwu_by_instance = mann_whitney_test(df)

print(
    mwu_by_instance[
        ["instance", "median_CDRVNS", "median_MA_EDM", "delta_median", "pvalue", "winner_sig"]
    ]
)
print(mwu_by_instance["winner_sig"].value_counts())


