import pandas as pd 
import seaborn as sns 
import matplotlib.pyplot as plt 

# =========================
# Isic lineplot
# =========================

df_isic = pd.read_pickle("results/isic_exp2.pickle")

df_plot = df_isic.copy()
df_plot["instance"] = "isic"
df_plot = df_plot[df_plot["nopt"] > 0]
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
ax.set(xlabel="Sparsity", ylabel="Numbers of Optima", yscale="log")
#ax.ticklabel_format(axis="both", style="plain", useOffset=False)

sns.move_legend(g, "upper center", title="Instance")


g.figure.tight_layout()
g.figure.savefig("fig_isic_lineplot.pdf", bbox_inches="tight")
#plt.show()
plt.close(g.figure)
