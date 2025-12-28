import pymrio
import pandas as pd
import contextlib


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


import gc
import pymrio
import pandas as pd

def compare_across_folders(folders, normalize: bool = True):
    # ---- Load base only once ----
    print(f"\n--- Loading base: {folders[0]} ---")
    base = LoadInstance(file_path=folders[0])
    base.check_internal_matrices(normalize=normalize)

    # Copy only labels
    base_Zi = base.matrix.Z.index.copy()
    base_Zc = base.matrix.Z.columns.copy()
    base_Ai = base.matrix.A.index.copy()
    base_Ac = base.matrix.A.columns.copy()
    base_Yi = base.matrix.Y.index.copy()
    base_xi = base.matrix.x.index.copy()

    base_name = folders[0]

    # Compare one file at a time 
    for name in folders[1:]:
        print(f"\n--- Loading: {name} ---")
        inst = LoadInstance(file_path=name)
        inst.check_internal_matrices(normalize=normalize)

        print(f"\n=== Comparing {name} vs {base_name} ===")
        inst._check(f"{base_name} Z.index", base_Zi, f"{name} Z.index", inst.matrix.Z.index, normalize=normalize)
        inst._check(f"{base_name} Z.columns", base_Zc, f"{name} Z.columns", inst.matrix.Z.columns, normalize=normalize)

        inst._check(f"{base_name} A.index", base_Ai, f"{name} A.index", inst.matrix.A.index, normalize=normalize)
        inst._check(f"{base_name} A.columns", base_Ac, f"{name} A.columns", inst.matrix.A.columns, normalize=normalize)

        inst._check(f"{base_name} Y.index", base_Yi, f"{name} Y.index", inst.matrix.Y.index, normalize=normalize)
        inst._check(f"{base_name} x.index", base_xi, f"{name} x.index", inst.matrix.x.index, normalize=normalize)

        # FREE MEMORY of this instance 
        inst.matrix = None
        del inst
        gc.collect()
    
    base.matrix = None
    del base
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

    log_file = "test_labels_check.log"
    with open(log_file, "w", encoding="utf-8") as f, contextlib.redirect_stdout(f), contextlib.redirect_stderr(f):
        compare_across_folders(folders_ixi, normalize=True)
        compare_across_folders(folders_pxp, normalize=True)

    print(f"Log write in: {log_file}")
