import argparse
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
from scipy.stats import mannwhitneyu
from pathlib import Path

def compute_statistics(df: pd.DataFrame) -> pd.DataFrame:
    # Adjust name of the instance
    df["instance"] = df.apply(lambda row: row["instance"].split("/")[-1], axis=1)

    # remove all MS algorithms
    df["algname"] = df["algname"].str.replace(r"^MS-", "", regex=True)

    # Find set best-fitness
    df["fit"] = df.apply(
        lambda row: np.max([int(s) for s in row["fit_set"].split(",")]),
        axis=1,
    )

    # Instance best-fitness
    df["best_obj_value"] = df.groupby("instance")["fit"].transform("max")

    # Calculate rpd (relative percentage deviation)
    df["rpd"] = 100 * (df["best_obj_value"] - df["fit"]) / df["best_obj_value"]

    # # median rpd
    # df["median_rpd"] = df["rpd"].median()

    # median instance rpd
    g = df.groupby("instance")["rpd"]
    df["median_rpd_inst"] = g.transform("median")
    df["rpd_p10"] = g.transform(lambda s: s.quantile(0.10))
    df["rpd_p90"] = g.transform(lambda s: s.quantile(0.90))

    return df


def plot_boxen_m5(df: pd.DataFrame, out_pdf: str) -> None:
    df_ss = df[df["m"] == 5].copy()

    fig, ax = plt.subplots(figsize=(18, 18)) 

    sns.boxenplot(
        data=df_ss,
        x="instance",
        y="rpd",
        hue="algname",
        ax=ax
    )

    ax.set_ylim(0, 0.58)
    ax.tick_params(axis="x", rotation=90, labelsize=10)
    fig.subplots_adjust(bottom=0.35)
    fig.savefig(out_pdf, format="pdf")
    plt.show()
    plt.close(fig)
    plt.show()
    plt.close(fig)

def mann_whitney_test(df: pd.DataFrame) -> pd.DataFrame:
    # Stats test 
    ALG_A = "CDRVNS"
    ALG_B = "MA-EDM"
    value_col = "fit"
    alpha = 0.05

    rows = []
    sub = df[df["algname"].isin([ALG_A, ALG_B])]

    for inst, g in sub.groupby("instance"):
        x = g.loc[g["algname"] == ALG_A, value_col].to_numpy()
        y = g.loc[g["algname"] == ALG_B, value_col].to_numpy()

        res = mannwhitneyu(x, y)

        med_a = float(np.median(x))
        med_b = float(np.median(y))
        delta = med_a - med_b

        winner_median = "CDRVNS" if delta > 0 else ("MA-EDM" if delta < 0 else "tie")
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
        "display.max_columns", max_cols,
        "display.max_colwidth", None,
    ):
        latex = df.to_latex(index=False, escape=False, longtable=True)

    out_path.write_text(latex, encoding="utf-8")

def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input_csv", help="CSV of input (sep=';')")
    parser.add_argument("--out-pdf", default="boxplot.pdf")
    parser.add_argument("--out-tex", default=None, help="Export compute_statistics DF to LaTeX (.tex)")
    args = parser.parse_args()

    df = pd.read_csv(args.input_csv, sep=";")

    df = compute_statistics(df)

    if args.out_tex:
        df.drop(columns=["fit", "sol_set", "seed", "fit_set", "n", "nevals", "millis", "n_local_optima"], inplace=True)
        # Reorder columns: instance, algname, m, then the rest
        first = ["instance", "algname", "m"]
        rest = [c for c in df.columns if c not in first]
        df = df[first + rest]
        export_df_to_latex(df, args.out_tex)
        exit()

    plot_boxen_m5(df, args.out_pdf)

    mwu_by_instance = mann_whitney_test(df)

    print(mwu_by_instance[["instance", "median_CDRVNS", "median_MA_EDM", "delta_median", "pvalue", "winner_sig"]])
    print(mwu_by_instance["winner_sig"].value_counts())


if __name__ == "__main__":
    main()
