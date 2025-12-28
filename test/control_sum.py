import pymrio
import pandas as pd
import contextlib
import gc
from pathlib import Path


class LoadInstance:
    def __init__(self, file_path: str) -> None:
        self.file_path = file_path

    def check_x(self, show_examples: int = 20):
        matrix = pymrio.parse_exiobase3(self.file_path)
        try:
            Z = matrix.Z
            Y = matrix.Y
            x_off = matrix.x.sum(axis=1)

            x_calc = Z.sum(axis=1) + Y.sum(axis=1)

            print("\n============================================================")
            print(f"FILE: {self.file_path}")
            print("CHECK: x_off ?= Z.sum(axis=1) + Y.sum(axis=1)                ")
            print("============================================================")

            same_index = x_off.index.equals(x_calc.index)
            print("\n[INDEX]")
            print(f"same_index: {same_index}")
            print(f"len(x_off):  {len(x_off)}")
            print(f"len(x_calc): {len(x_calc)}")

            x_off = x_off.reindex(x_calc.index)
            diff = x_off - x_calc

            n_neq = int((diff != 0).sum())
            n_eq  = int((diff == 0).sum())

            print("\n[TOTALS]")
            print(f"sum(x_off) :  {float(x_off.sum())}")
            print(f"sum(x_calc):  {float(x_calc.sum())}")
            print(f"TOTAL DIFF (off - calc): {float(x_off.sum() - x_calc.sum())}")
            print(f"sum(diff)              : {float(diff.sum())}")

            print("\n[ROW-BY-ROW EXACT]")
            print(f"rows equal (diff==0): {n_eq}")
            print(f"rows diff  (diff!=0): {n_neq}")

            if n_neq > 0:
                table = pd.DataFrame({"x_off": x_off, "x_calc": x_calc, "diff": diff})
                table["abs_diff"] = table["diff"].abs()
                worst = table.sort_values("abs_diff", ascending=False).head(show_examples)
                print(f"\n[WORST {show_examples} rows by |diff|]")
                print(worst.drop(columns=["abs_diff"]).to_string())
        finally:
            # deallocate RAM
            del matrix
            gc.collect()


def compare_across_folders(folders, title: str, show_examples: int = 20):
    print("\n\n#############################")
    print(f"# {title} Sum log #")
    print("#############################")

    for fp in folders:
        inst = LoadInstance(fp)
        inst.check_x(show_examples=show_examples)
        del inst
        gc.collect()

if __name__ == "__main__":

    folders_ixi = [
        "../Compact-data/IOT_1995_ixi.zip",
        "../Compact-data/IOT_1996_ixi.zip",
        "../Compact-data/IOT_1997_ixi.zip",
        "../Compact-data/IOT_1998_ixi.zip",
        "../Compact-data/IOT_1999_ixi.zip",
        "../Compact-data/IOT_2000_ixi.zip",
        "../Compact-data/IOT_2001_ixi.zip",
        "../Compact-data/IOT_2002_ixi.zip",
        "../Compact-data/IOT_2003_ixi.zip",
        "../Compact-data/IOT_2004_ixi.zip",
        "../Compact-data/IOT_2005_ixi.zip",
        "../Compact-data/IOT_2006_ixi.zip",
        "../Compact-data/IOT_2007_ixi.zip",
        "../Compact-data/IOT_2008_ixi.zip",
        "../Compact-data/IOT_2009_ixi.zip",
        "../Compact-data/IOT_2010_ixi.zip",
        "../Compact-data/IOT_2011_ixi.zip",
        "../Compact-data/IOT_2012_ixi.zip",
        "../Compact-data/IOT_2013_ixi.zip",
        "../Compact-data/IOT_2014_ixi.zip",
        "../Compact-data/IOT_2015_ixi.zip",
        "../Compact-data/IOT_2016_ixi.zip",
        "../Compact-data/IOT_2017_ixi.zip",
        "../Compact-data/IOT_2018_ixi.zip",
        "../Compact-data/IOT_2019_ixi.zip",
        "../Compact-data/IOT_2020_ixi.zip",
        "../Compact-data/IOT_2021_ixi.zip",
        "../Compact-data/IOT_2022_ixi.zip",
    ]

    folders_pxp = [
        "../Compact-data/IOT_1995_pxp.zip",
        "../Compact-data/IOT_1996_pxp.zip",
        "../Compact-data/IOT_1997_pxp.zip",
        "../Compact-data/IOT_1998_pxp.zip",
        "../Compact-data/IOT_1999_pxp.zip",
        "../Compact-data/IOT_2000_pxp.zip",
        "../Compact-data/IOT_2001_pxp.zip",
        "../Compact-data/IOT_2002_pxp.zip",
        "../Compact-data/IOT_2003_pxp.zip",
        "../Compact-data/IOT_2004_pxp.zip",
        "../Compact-data/IOT_2005_pxp.zip",
        "../Compact-data/IOT_2006_pxp.zip",
        "../Compact-data/IOT_2007_pxp.zip",
        "../Compact-data/IOT_2008_pxp.zip",
        "../Compact-data/IOT_2009_pxp.zip",
        "../Compact-data/IOT_2010_pxp.zip",
        "../Compact-data/IOT_2011_pxp.zip",
        "../Compact-data/IOT_2012_pxp.zip",
        "../Compact-data/IOT_2013_pxp.zip",
        "../Compact-data/IOT_2014_pxp.zip",
        "../Compact-data/IOT_2015_pxp.zip",
        "../Compact-data/IOT_2016_pxp.zip",
        "../Compact-data/IOT_2017_pxp.zip",
        "../Compact-data/IOT_2018_pxp.zip",
        "../Compact-data/IOT_2019_pxp.zip",
        "../Compact-data/IOT_2020_pxp.zip",
        "../Compact-data/IOT_2021_pxp.zip",
        "../Compact-data/IOT_2022_pxp.zip",
    ]


    log_file = "test_sum.log"
    Path(log_file).parent.mkdir(parents=True, exist_ok=True)

    with open(log_file, "w", encoding="utf-8") as f, contextlib.redirect_stdout(f):
        compare_across_folders(folders_ixi, title="IXI", show_examples=10)
        compare_across_folders(folders_pxp, title="PXP", show_examples=10)

    print(f"Log write in: {log_file}")