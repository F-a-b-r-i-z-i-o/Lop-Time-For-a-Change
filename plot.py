import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt 
import matplotlib.ticker as mticker

filename_in = "stats.pickle"
df = pd.read_pickle(filename_in)

### box for boxplot 
# plt.figure(figsize=(8, 6))
# metrics = ["rpd_phi", "delta_nn_norm", "delta_sp_norm"]

# df_long = df.melt(
#     id_vars=["instance_set", "algname", "m"],
#     value_vars=metrics,
#     var_name="metric",
#     value_name="value",
# )

# sns.set_theme(style="whitegrid", font_scale=1.5)

# g = sns.catplot(
#     data=df_long,
#     row="instance_set",
#     col="metric",
#     x="m",
#     y="value",
#     hue="algname",
#     kind="box",
#     row_order=["rxr", "pxp", "os300"],
#     col_order=metrics,
#     sharey=False,
# )

# g.set_axis_labels("m", "RPD")
# g.set_titles(row_template="{row_name}", col_template="{col_name}")
# #g.suptitle("Metrics vs m by instance set", y=1.05)

# sns.move_legend(
#     g,
#     "lower center",
#     bbox_to_anchor=(0.5, -0.10),  
#     ncol=df_long["algname"].nunique(),
#     title="Algorithms",
#     frameon=True,
# )

# #g.savefig("fig_metrics_by_set.pdf")
# #plt.show()
# plt.close()


sns.set_theme(style="whitegrid", font_scale=1.2)

# Sacatter plot
selected_instances = [
    # "rxr_2021_n49",
    "rxr_1996_n49",
    # "rxr_2019_n49",
    #"pxp_it_2022_n165",
    "pxp_ch_2022_n141",
    # "pxp_pl_2022_n166",
    # "os300_wa_2022_n300",
    # "os300_bg_2022_n300",
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
    facet_kws={"sharex":False, "sharey":False}
)

for ax in g.axes.flat:
    for axis in (ax.xaxis, ax.yaxis):
        fmt = mticker.ScalarFormatter(useOffset=False)
        fmt.set_scientific(False)
        axis.set_major_formatter(fmt)
        axis.get_offset_text().set_visible(False)

g.set_axis_labels("delta_sp", "phi")
g.set_titles(col_template="m={col_name}", row_template="{row_name}")

g.fig.suptitle("Solution quality: phi vs delta_sp (by instance and m)", y=1.02)

sns.move_legend(
    g,
    "lower center",
    bbox_to_anchor=(0.5, -0.10),
    ncol=df_sel["algname"].nunique(),
    title="Algorithms",
    frameon=True,
)

g.savefig("scatter_phi_vs_delta_sp.pdf", bbox_inches="tight")
plt.show()
plt.close()