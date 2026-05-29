import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

plt.style.use("graphs/style.mplstyle")

df = pd.read_pickle("results/isic_exp2.pickle")

fig,ax = plt.subplots(1,1)
ax = sns.lineplot(
    data      = df,
    x         = 'sparsity',
    y         = 'nopt',
    estimator = 'mean',
    errorbar  = ('ci', 95),
    ax        = ax,
)
ax.set_xlabel('Instance Sparsity')
ax.set_ylabel(r'# Global Optima')
ax.set_yscale('log')
plt.tight_layout()
plt.savefig('graphs/fig_isic_lineplot.pdf')
plt.close(fig)
