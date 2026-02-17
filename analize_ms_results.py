import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
from matplotlib import patheffects as pe
from pathlib import Path


# =========================
#  Long table
# =========================

filename_in = "stats.pickle"
out = "results-ms-table.tex"

df = pd.read_pickle(filename_in)

# Compute per-(instance, algname) RPD statistics and broadcast them to each row
grp = df.groupby(["instance", "algname"])["rpd"]
df["median_rpd"] = grp.transform("median")
df["q1_rpd"] = grp.transform(lambda s: s.quantile(0.25))
df["q3_rpd"] = grp.transform(lambda s: s.quantile(0.75))
df["IQR_RPD"] = df["q3_rpd"] - df["q1_rpd"]

# Keep only one row per instance: the run with maximum fit
idx = df.groupby("instance")["fit"].idxmax()

tab = df.loc[idx, ["instance_set", "instance", "algname", "m", "fit", "median_rpd", "IQR_RPD"]].copy()
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

# =========================
# BOXPLOT
# =========================
sns.set_theme(style="whitegrid", font_scale=1.5)

metrics = ["rpd_phi", "delta_nn_norm", "delta_sp_norm"]

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
    sharey=False,
    height=3.2,
    aspect=1.05,
)

g.set_axis_labels("m", "value")
g.set_titles(row_template="{row_name}", col_template="{col_name}")


# Seaborn puts a single "global" legend outside the grid (g._legend).
# grab its handles/labels, remove it, then add it inside the first panel.
handles = g._legend.legend_handles
labels = [t.get_text() for t in g._legend.get_texts()]
title = g._legend.get_title().get_text()
g._legend.remove()

ax0 = g.axes[0, 0]
leg = ax0.legend(
    handles,
    labels,
    title=title,
    loc="upper left",
    bbox_to_anchor=(0.02, 0.98),
    borderaxespad=0.0,
    frameon=False,
    fontsize=10,
    title_fontsize=10,
    handlelength=1.2,
    handletextpad=0.4,
    labelspacing=0.25,
)

# Make legend text readable over the plot by adding a white outline.
for txt in leg.get_texts() + [leg.get_title()]:
    txt.set_path_effects([pe.withStroke(linewidth=3, foreground="white")])

#g.savefig("fig_metrics_by_set_norm.pdf", bbox_inches="tight")
#plt.show()
plt.close()

# =========================
# SCATTER
# =========================
sns.set_theme(style="whitegrid", font_scale=1.2)

selected_instances = [
    "rxr_1996_n49",
    "pxp_it_2022_n165",
    "os300_it_2022_n300",
]

df_sel = df[df["instance"].isin(selected_instances)].copy()
df_sel["delta_sp"] = df_sel["delta_sp"].round(3)
df_sel["delta_nn"] = df_sel["delta_nn"].round(3)

g = sns.relplot(
    data=df_sel,
    col="m",
    row="instance",
    x="delta_sp",
    y="phi",
    hue="algname",
    kind="scatter",
    row_order=selected_instances,
    facet_kws={"sharex": False, "sharey": False},
    height=3.0,
    aspect=1.15,
)

for ax in g.axes.flat:
    for axis in (ax.xaxis, ax.yaxis):
        fmt = mticker.ScalarFormatter(useOffset=False)
        fmt.set_scientific(False)
        axis.set_major_formatter(fmt)
        axis.get_offset_text().set_visible(False)

g.set_axis_labels("delta_sp", "phi")
g.set_titles(col_template="m={col_name}", row_template="{row_name}")

handles = g._legend.legend_handles
labels = [t.get_text() for t in g._legend.get_texts()]
title = g._legend.get_title().get_text()
g._legend.remove()

ax0 = g.axes[0, 0]
leg = ax0.legend(
    handles,
    labels,
    title=title,
    loc="upper left",
    bbox_to_anchor=(0.02, 0.98),
    borderaxespad=0.0,
    frameon=False,
    fontsize=10,
    title_fontsize=10,
    handlelength=1.2,
    handletextpad=0.4,
    labelspacing=0.25,
)

for txt in leg.get_texts() + [leg.get_title()]:
    txt.set_path_effects([pe.withStroke(linewidth=3, foreground="white")])

#g.savefig("scatter_phi_vs_delta_sp.pdf", bbox_inches="tight")
#plt.show()
plt.close()