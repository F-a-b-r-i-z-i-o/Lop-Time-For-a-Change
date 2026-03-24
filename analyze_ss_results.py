import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
from scipy.stats import mannwhitneyu, false_discovery_control
from pathlib import Path
import numpy as np

#Load data
df = pd.read_pickle('results/stats.pickle')

#Modify algorithm names for single-solution setting
df.algname = df.algname.str.replace(r'^MS-', '', regex=True)

#Filter out data of multiple-solutions setting
df = df[ df.m==5 ]

#Load graphic style
plt.style.use("graphs/style.mplstyle")

#Create the boxplot
fig,ax = plt.subplots(1,1)
ax = sns.boxplot(
    ax          = ax,
    data        = df,
    x           = 'instance_set',
    y           = 'rpd',
    hue         = 'algname',
    order       = [ 'rxr', 'pxp', 'os300' ],
    gap         = 0.2,
    flierprops  = { 'marker': 'x',
                    'markersize': 1,
                    'markeredgewidth': 0.2,
                    'alpha': 0.4
                  },
    #linewidth   = 0.05,
)

#Labels
ax.set_xlabel(r'Instance Set')
ax.set_ylabel('RPD')
ax.legend(title='Algorithm')

#Legend
ax.legend(loc='upper left')

#Y axis logscale and limits
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

#Y axis range
ax.set_ylim(-1e-3, 1.0)

#Save the file
plt.tight_layout()
#plt.show()
plt.savefig('graphs/fig_ss_boxplot.pdf')

#Close the figure
plt.close(fig)


### TABLE AND STATS TEST
instance_sets = ["os300", "pxp", "rxr"]

ALG_A = "CD-RVNS"
ALG_B = "MA-EDM"
alpha = 0.05


def median_symbol(median_a, median_b, pvalue_adj):
    if pvalue_adj <= alpha:
        if median_a > median_b:
            return r" $\blacktriangle$"
        elif median_a < median_b:
            return r" $\triangledown$"
    return r" \phantom{$\blacktriangle$}"


def bold_best_values(best_a, best_b):
    if best_a > best_b:
        return rf"\textbf{{{best_a}}}", f"{best_b}"
    elif best_b > best_a:
        return f"{best_a}", rf"\textbf{{{best_b}}}"
    else:
        return rf"\textbf{{{best_a}}}", rf"\textbf{{{best_b}}}"


for instance_set in instance_sets:
    out = f"tables/results_{instance_set}-ss-table.tex"

    instances = sorted(
        df.loc[df["instance_set"] == instance_set, "instance"].unique()
    )

    raw_rows = []
    pvalues = []

    for instance in instances:
        cd = df[
            (df["instance_set"] == instance_set) &
            (df["instance"] == instance) &
            (df["algname"] == ALG_A)
        ]

        ma = df[
            (df["instance_set"] == instance_set) &
            (df["instance"] == instance) &
            (df["algname"] == ALG_B)
        ]

        cd_best_val = int(round(cd["best"].iloc[0]))
        cd_median_val = cd["fit"].median()
        cd_iqr = cd["fit"].quantile(0.75) - cd["fit"].quantile(0.25)

        ma_best_val = int(round(ma["best"].iloc[0]))
        ma_median_val = ma["fit"].median()
        ma_iqr = ma["fit"].quantile(0.75) - ma["fit"].quantile(0.25)

        x = cd["fit"].to_numpy()
        y = ma["fit"].to_numpy()

        res = mannwhitneyu(x, y, alternative="two-sided", method="auto")
        pvalue = float(res.pvalue)

        raw_rows.append({
            "Instance": instance,
            "cd_best_val": cd_best_val,
            "cd_median_val": cd_median_val,
            "cd_iqr": cd_iqr,
            "ma_best_val": ma_best_val,
            "ma_median_val": ma_median_val,
            "ma_iqr": ma_iqr,
        })
        pvalues.append(pvalue)


    pvalues_adj = false_discovery_control(np.array(pvalues), method="bh")

    rows = []
    for row, pvalue_adj in zip(raw_rows, pvalues_adj):
        cd_symbol = median_symbol(row["cd_median_val"], row["ma_median_val"], pvalue_adj)
        ma_symbol = median_symbol(row["ma_median_val"], row["cd_median_val"], pvalue_adj)

        cd_median = f'{row["cd_median_val"]:.0f}{cd_symbol}'
        ma_median = f'{row["ma_median_val"]:.0f}{ma_symbol}'

        cd_best, ma_best = bold_best_values(row["cd_best_val"], row["ma_best_val"])

        rows.append([
            row["Instance"],
            cd_best, cd_median, f'{row["cd_iqr"]:.0f}',
            ma_best, ma_median, f'{row["ma_iqr"]:.0f}',
        ])

    table = pd.DataFrame(
        rows,
        columns=pd.MultiIndex.from_tuples([
            ("Instance", ""),
            (ALG_A, "Best"),
            (ALG_A, "Median"),
            (ALG_A, "IQR"),
            (ALG_B, "Best"),
            (ALG_B, "Median"),
            (ALG_B, "IQR"),
        ])
    )

    latex_tabular = table.to_latex(
        index=False,
        escape=False,
        multicolumn=True,
        multicolumn_format="c",
        column_format=r"l@{\hspace{1.2cm}}rrr@{\hspace{1.2cm}}rrr"
    )

    lines = latex_tabular.splitlines()
    inner_lines = lines[1:-1]
    inner_tabular = "\n".join(inner_lines)

    caption = (
        f"Comparison between {ALG_A} and {ALG_B} on the {instance_set} instances. "
        f"Best values are highlighted in bold. Median values are annotated according "
        f"to the Mann--Whitney test with Benjamini--Hochberg correction "
        f"($\\alpha={alpha}$)."
    )

    latex_table = (
        "\\begin{table}[]\n"
        "    \\centering\n"
        f"    \\caption{{{caption}}}\n"
        "    \\resizebox{0.8\\textwidth}{!}{\n"
        "    \\begin{tabular}{l@{\\hspace{1.2cm}}rrr@{\\hspace{1.2cm}}rrr}\n"
        f"{inner_tabular}\n"
        "    \\end{tabular}\n"
        "    }\n"
        "\\end{table}\n"
    )

    Path(out).parent.mkdir(parents=True, exist_ok=True)
    Path(out).write_text(latex_table)

    print(f"Wrote: {out}")
