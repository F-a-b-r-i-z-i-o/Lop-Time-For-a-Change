import pymrio
import pandas as pd
import contextlib
import gc
from collections import defaultdict


class LoadInstance:
    def __init__(self, file_path: str) -> None:
        self.file_path = file_path
        self.matrix = self._load_matrix()

    def _load_matrix(self):
        return pymrio.parse_exiobase3(self.file_path)

    def _normalize_idx(self, idx):
        return pd.Index([str(v).strip() for v in idx], name=idx.name)

    def _check(self, name1, idx1, name2, idx2, show_examples: int = 10, normalize: bool = True):
        if normalize:
            idx1 = self._normalize_idx(idx1)
            idx2 = self._normalize_idx(idx2)

        equals_ok = idx1.equals(idx2)
        s1, s2 = set(idx1), set(idx2)
        same_elements_ok = (s1 == s2)

        dup1 = int(idx1.duplicated().sum()) if hasattr(idx1, "duplicated") else 0
        dup2 = int(idx2.duplicated().sum()) if hasattr(idx2, "duplicated") else 0

        print(f"\n[{name1}] vs [{name2}]")
        print(f"  equals (name+order): {equals_ok}")
        print(f"  same elements (name): {same_elements_ok}")
        print(f"  len: {len(idx1)} vs {len(idx2)}")
        print(f"  duplicates: {dup1} vs {dup2}")

        if not same_elements_ok:
            missing = sorted(s1 - s2)
            extra = sorted(s2 - s1)
            print(f" missing in {name2}: {len(missing)}")
            if missing:
                print(f" examples missing: {missing[:show_examples]}")
            print(f"  extra in {name2}: {len(extra)}")
            if extra:
                print(f" examples extra: {extra[:show_examples]}")
        else:
            if not equals_ok:
                print(" NOTE: same name but different order.")

        return equals_ok, same_elements_ok

    def check_internal_matrices(self, normalize: bool = True):
        Z = self.matrix.Z
        A = self.matrix.A
        Y = self.matrix.Y
        x = self.matrix.x

        print(f"\n=== INTERNAL CHECK for: {self.file_path} ===")
        self._check("Z.index", Z.index, "A.index", A.index, normalize=normalize)
        self._check("Z.index", Z.index, "Y.index", Y.index, normalize=normalize)
        self._check("Z.index", Z.index, "x.index", x.index, normalize=normalize)
        self._check("Z.columns", Z.columns, "A.columns", A.columns, normalize=normalize)


def _extract_labels(inst: LoadInstance):
    m = inst.matrix
    return {
        "Z.index": m.Z.index.copy(),
        "Z.columns": m.Z.columns.copy(),
        "A.index": m.A.index.copy(),
        "A.columns": m.A.columns.copy(),
        "Y.index": m.Y.index.copy(),
        "x.index": m.x.index.copy(),
    }


def compare_all_years(folders, normalize: bool = True, pairwise: bool = False, show_examples: int = 10):
    print("\n" + "=" * 100)
    print(f"GLOBAL LABEL CHECK ({'PAIRWISE' if pairwise else 'REFERENCE'}) for {len(folders)} files")
    print("=" * 100)

    labels_by_file = {}
    for fp in folders:
        print(f"\n--- Loading: {fp} ---")
        inst = LoadInstance(file_path=fp)
        inst.check_internal_matrices(normalize=normalize)

        labels_by_file[fp] = _extract_labels(inst)

        # free
        inst.matrix = None
        del inst
        gc.collect()

    keys = ["Z.index", "Z.columns", "A.index", "A.columns", "Y.index", "x.index"]

    if not pairwise:
        ref_fp = folders[0]
        ref = labels_by_file[ref_fp]

        print("\n" + "-" * 100)
        print(f"REFERENCE FILE: {ref_fp}")
        print("-" * 100)

        summary = {k: {"ok": [], "ko": []} for k in keys}

        for fp in folders[1:]:
            for k in keys:
                idx_ref = ref[k]
                idx_cur = labels_by_file[fp][k]
                equals_ok, same_elements_ok = LoadInstance(ref_fp)._check(
                    f"{ref_fp} {k}", idx_ref,
                    f"{fp} {k}", idx_cur,
                    normalize=normalize,
                    show_examples=show_examples
                )
                if equals_ok and same_elements_ok:
                    summary[k]["ok"].append(fp)
                else:
                    summary[k]["ko"].append(fp)

        print("\n" + "=" * 100)
        print("SUMMARY (vs reference)")
        print("=" * 100)
        for k in keys:
            print(f"\n[{k}]")
            print(f"  OK years: {len(summary[k]['ok'])}/{len(folders)-1}")
            if summary[k]["ko"]:
                print(f"  DIFFERENT years: {len(summary[k]['ko'])}")
                for fp in summary[k]["ko"]:
                    print(f"    - {fp}")
            else:
                print("  All years match reference.")

    else:
        print("\n" + "-" * 100)
        print("PAIRWISE COMPARISON (all pairs)")
        print("-" * 100)

        fps = list(folders)
        n = len(fps)

        diff_count = defaultdict(int)

        for i in range(n):
            for j in range(i + 1, n):
                fp1, fp2 = fps[i], fps[j]
                for k in keys:
                    idx1 = labels_by_file[fp1][k]
                    idx2 = labels_by_file[fp2][k]
                    equals_ok, same_elements_ok = LoadInstance(fp1)._check(
                        f"{fp1} {k}", idx1,
                        f"{fp2} {k}", idx2,
                        normalize=normalize,
                        show_examples=show_examples
                    )
                    if not (equals_ok and same_elements_ok):
                        diff_count[k] += 1

        print("\n" + "=" * 100)
        print("PAIRWISE SUMMARY (#pairs that differ)")
        print("=" * 100)
        total_pairs = n * (n - 1) // 2
        print(f"Total pairs: {total_pairs}")
        for k in keys:
            print(f"{k}: {diff_count[k]} differing pairs")


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

    log_file = "test_labels_check.log"
    with open(log_file, "w", encoding="utf-8") as f, contextlib.redirect_stdout(f), contextlib.redirect_stderr(f):
        compare_all_years(folders_ixi, normalize=True, pairwise=True)
        compare_all_years(folders_pxp, normalize=True, pairwise=True)

    print(f"Log write in: {log_file}")
