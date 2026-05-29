import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
from matplotlib import patheffects as pe
from matplotlib.patches import Patch
from pathlib import Path



# =========================
# BOXPLOT
# =========================

#Load graphic style
plt.style.use("graphs/style.mplstyle")

#Load data
df = pd.read_pickle('results/stats.pickle')

#Calculate delta_nn_rpd and delta_sp_rpd if they dont exists
#df = df.drop(['delta_nn_rpd', 'delta_sp_rpd'], axis=1)
to_save = False
if 'delta_nn_rpd' not in df.columns:
    df['delta_nn_max'] = df.groupby(['instance','m'])['delta_nn'].transform('max')
    df['delta_nn_rpd'] = 100 * (df['delta_nn_max'] - df['delta_nn']) / df['delta_nn_max']
    to_save = True
if 'delta_sp_rpd' not in df.columns:
    df['delta_sp_max'] = df.groupby(['instance','m'])['delta_sp'].transform('max')
    df['delta_sp_rpd'] = 100 * (df['delta_sp_max'] - df['delta_sp']) / df['delta_sp_max']
    to_save = True
if to_save:
    df.to_pickle('results/stats.pickle')

#Reorganize data in long format
metrics = ["rpd_phi", "delta_nn_rpd", "delta_sp_rpd"]
df_long = df.melt(
    id_vars     = ["instance_set", "algname", "m"],
    value_vars  = metrics,
    var_name    = "metric",
    value_name  = "value",
)

#Create 3x3 boxplots
g = sns.catplot(
    data        = df_long,
    kind        = "box",
    row         = "metric",
    #row_order  = 
    col         = "m",
    col_order   = [ 5, 10, 15 ],
    x           = "instance_set",
    order       = ["rxr", "pxp", "os300"],
    y           = "value",
    hue         = "algname",
    #hue_order  = 
    sharey      = 'row',
    legend_out  = False,
    gap         = 0.1,
    flierprops  = { 'marker': 'x',
                    'markersize': 1,
                    'markeredgewidth': 0.2,
                    'alpha': 0.4 },
)

#Set y-axis limits and scales: 1st row (rpd_phi) and 2nd row (delta_nn) in symlog scale, while 3rd row (delta_sp) normal scale
for ax in g.axes[0,:]: #rpd_phi
    ax.set_yscale('symlog', linthresh=1e-2)
    ax.yaxis.set_major_locator( mticker.SymmetricalLogLocator(base=10, linthresh=1e-2) )
    ax.yaxis.set_minor_locator( mticker.SymmetricalLogLocator(base=10, linthresh=1e-2, subs=range(1,9+1)) )
    ax.yaxis.set_major_formatter( mticker.FuncFormatter(lambda x, pos: f"{x:g}") )
    ax.set_ylim(-1e-3, 1.0)
for ax in g.axes[1,:]:
    #ax.set_ylim(-1e-3, 0.022) #delta_nn_norm
    ax.set_yscale('symlog', linthresh=1e-2)
    ax.yaxis.set_major_locator( mticker.SymmetricalLogLocator(base=10, linthresh=1e-2) )
    ax.yaxis.set_minor_locator( mticker.SymmetricalLogLocator(base=10, linthresh=1e-2, subs=range(1,9+1)) )
    ax.yaxis.set_major_formatter( mticker.FuncFormatter(lambda x, pos: f"{x:g}") )
    ax.set_ylim(-1e-3, 100.0)
    #ax.set_ylim(-3., 100.) #non log
for ax in g.axes[2,:]:
    #ax.set_ylim(0.3, 1.01) #delta_sp_norm
    ax.set_yscale('symlog', linthresh=1e-2)
    ax.yaxis.set_major_locator( mticker.SymmetricalLogLocator(base=10, linthresh=1e-2) )
    ax.yaxis.set_minor_locator( mticker.SymmetricalLogLocator(base=10, linthresh=1e-2, subs=range(1,9+1)) )
    ax.yaxis.set_major_formatter( mticker.FuncFormatter(lambda x, pos: f"{x:g}") )
    ax.set_ylim(-1e-3, 100.)
    #ax.set_ylim(-2., 60.) #non log

#Labels
g.set_axis_labels('Instance Set', '', fontsize=16)
g.axes[0,0].set_ylabel(r'$\Phi$ (RPD)')
g.axes[1,0].set_ylabel(r'$\Delta_\mathit{NN}$ (RPD)')
g.axes[2,0].set_ylabel(r'$\Delta_\mathit{SP}$ (RPD)')

#Tick labels
for ax in g.axes.flatten():
    ax.tick_params(axis='x', labelsize=12)
    ax.tick_params(axis='y', labelsize=12)

#Legend
leg = g.axes[0,0].get_legend()
labels = [t.get_text() for t in leg.get_texts()]
colors = [h.get_facecolor() for h in leg.legend_handles]
leg.remove()
g.axes[0,0].legend(
    [Patch(facecolor=c) for c in colors],
    labels,
    title="Algorithms",
    fontsize=12,
    title_fontsize=16,
    handlelength=1.5,
    handleheight=1.0,
)

#Titles
g.set_titles('')
g.axes[0,0].set_title(r'$m=5$',  fontsize=16)
g.axes[0,1].set_title(r'$m=10$', fontsize=16)
g.axes[0,2].set_title(r'$m=15$', fontsize=16)

#Tight layout
plt.tight_layout()

#Save to pdf
g.savefig("graphs/fig_metrics_by_set_norm.pdf", bbox_inches="tight")

#Close figure
plt.close(g.fig)

# =========================
# SINGLE SCATTER PLOT
# =========================

#selected instance and m
sel_inst = 'os300_it_2022_n300'
sel_m = 15

#create the scatter plot for DELTA_SP
fig,ax = plt.subplots(1,1)
ax = sns.scatterplot(
        ax      = ax,
        data    = df.query(f'instance=="{sel_inst}" and m=={sel_m}'),
        x       = 'delta_sp',
        y       = 'phi',
        hue     = 'algname',
    )
#y-axis in plain notation
ax.yaxis.set_major_formatter(mticker.ScalarFormatter())
ax.ticklabel_format(style='plain', axis='y')
#axes limits
ax.set_xlim(12.3, 15.1)
ax.set_ylim(27100000, 27245000)
#labels
ax.set_ylabel(r'$\Phi$')
ax.set_xlabel(r'$\Delta_\mathit{SP}$')
ax.legend(title='Algorithms')
#Save the file
plt.tight_layout()
plt.savefig('graphs/fig_scatterplot_sp.pdf')
#Close the figure
plt.close(fig)

#create the scatter plot for DELTA_NN
fig,ax = plt.subplots(1,1)
ax = sns.scatterplot(
        ax      = ax,
        data    = df.query(f'instance=="{sel_inst}" and m=={sel_m}'),
        x       = 'delta_nn',
        y       = 'phi',
        hue     = 'algname',
    )
#y-axis in plain notation
ax.yaxis.set_major_formatter(mticker.ScalarFormatter())
ax.ticklabel_format(style='plain', axis='y')
#axes limits
ax.set_xlim(10., 250.)
ax.set_ylim(27100000, 27245000)
#labels
ax.set_ylabel(r'$\Phi$')
ax.set_xlabel(r'$\Delta_\mathit{NN}$')
ax.legend(title='Algorithms')
#Save the file
plt.tight_layout()
plt.savefig('graphs/fig_scatterplot_nn.pdf')
#Close the figure
plt.close(fig)


# =========================
# MULTIPLE SCATTER PLOT
# =========================
for diversity_metric in ['delta_nn', 'delta_sp']:
    for instance_set in df.instance_set.unique():
        for m in df.m.unique():
            g = sns.relplot(
                data        = df.query(f'instance_set=="{instance_set}" and m=={m}'),
                kind        = 'scatter',
                x           = diversity_metric,
                y           = 'phi',
                hue         = 'algname',
                col         = 'instance',
                col_wrap    = 5,
                facet_kws   = { 'sharex': False,
                                'sharey': False, },
            )
            plt.tight_layout()
            g.savefig(f'graphs/fig_scatters_{instance_set}_m{m}_{diversity_metric}.pdf', bbox_inches='tight')
            plt.close(g.fig)





# =========================
# TABLE AND STATS TEST
# =========================
instance_sets = ["os300", "pxp", "rxr"]
m_values = [5, 10, 15]

ALG_A = "MS-CD-RVNS"
ALG_B = "MS-MA-EDM"
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
    for m in m_values:
        out = f"tables/results_{instance_set}_m{m}-ms-table.tex"

        instances = sorted(
            df.loc[
                (df["instance_set"] == instance_set) & (df["m"] == m),
                "instance"
            ].unique()
        )

        raw_rows = []
        pvalues = []

        for instance in instances:
            cd = df[
                (df["instance_set"] == instance_set) &
                (df["m"] == m) &
                (df["instance"] == instance) &
                (df["algname"] == ALG_A)
            ]

            ma = df[
                (df["instance_set"] == instance_set) &
                (df["m"] == m) &
                (df["instance"] == instance) &
                (df["algname"] == ALG_B)
            ]

            cd_best_val = cd["best_phi"].iloc[0]
            cd_median_val = cd["phi"].median()
            cd_iqr = cd["phi"].quantile(0.75) - cd["phi"].quantile(0.25)

            ma_best_val = ma["phi"].iloc[0]
            ma_median_val = ma["phi"].median()
            ma_iqr = ma["phi"].quantile(0.75) - ma["phi"].quantile(0.25)

            x = cd["phi"].to_numpy()
            y = ma["phi"].to_numpy()

            res = mannwhitneyu(x, y, alternative="two-sided", method="auto")
            pvalue = float(res.pvalue)

            raw_rows.append({
                "Instance": instance,
                "m": m,
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

            cd_median = f'{row["cd_median_val"]}{cd_symbol}'
            ma_median = f'{row["ma_median_val"]}{ma_symbol}'

            cd_best, ma_best = bold_best_values(row["cd_best_val"], row["ma_best_val"])

            rows.append([
                row["Instance"],
                row["m"],
                cd_best, cd_median, row["cd_iqr"],
                ma_best, ma_median, row["ma_iqr"],
            ])

        table = pd.DataFrame(
            rows,
            columns=pd.MultiIndex.from_tuples([
                ("Instance", ""),
                ("m", ""),
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
            float_format="{:.2f}".format,
            column_format=r"ll@{\hspace{1.2cm}}rrr@{\hspace{1.2cm}}rrr"
        )

        lines = latex_tabular.splitlines()
        inner_lines = lines[1:-1]
        inner_tabular = "\n".join(inner_lines)

        caption = f"Results for {instance_set} with m = {m} and phi"

        latex_table = (
            "\\begin{table}[]\n"
            "    \\centering\n"
            f"    \\caption{{{caption}}}\n"
            "    \\resizebox{0.8\\textwidth}{!}{\n"
            "    \\begin{tabular}{ll@{\\hspace{1.2cm}}rrr@{\\hspace{1.2cm}}rrr}\n"
            f"{inner_tabular}\n"
            "    \\end{tabular}\n"
            "    }\n"
            "\\end{table}\n"
        )

        Path(out).parent.mkdir(parents=True, exist_ok=True)
        Path(out).write_text(latex_table)

        print(f"Wrote: {out}")






