import pymrio
import numpy as np
import os
import pymrio.mrio_models.exio3_ixi as model
import pandas as pd

class LoadInstance:
    def __init__(self, file_path: str) -> None:
        self.file_path = file_path
        self.matrix = self._load_matrix()

    def _load_matrix(self):
        exio_matrix = pymrio.parse_exiobase3(self.file_path)
        return exio_matrix
    
def load_sectors():
    base_dir = os.path.dirname(model.__file__)
    sectors = os.path.join(base_dir, "sectors.tsv")
    df = pd.read_csv(sectors, sep="\t")
    return df

def normalize_square_df(A_df: pd.DataFrame) -> pd.DataFrame:
    """
    Return the normalized matrix A* where:
        a*_ij = a_ij - min(a_ij, a_ji)
    """
    vals = A_df.to_numpy(dtype=np.float128, copy=True)
    # Element wise symmetric cancellation
    vals = vals - np.minimum(vals, vals.T)
    return pd.DataFrame(vals, index=A_df.index, columns=A_df.columns)

def _key(code: str) -> int:
    # i01 -> 1, i10 -> 10 ...
    return int(code[1:])

def build_names_by_index(sectors_df: pd.DataFrame):
    sectors_df = sectors_df.copy()
    sectors_df["base_code"] = sectors_df["ExioCode"].astype(str).str.split(".", n=1).str[0]

    unique_codes = sorted(sectors_df["base_code"].unique(), key=_key)

    code2names = sectors_df.groupby("base_code")["ExioName"].apply(list).to_dict()
    names_by_index = [code2names[c] for c in unique_codes]  # indice -> lista ExioName

    return unique_codes, names_by_index

def aggregate_A_region_by_code(mrio, region: str, unique_codes, names_by_index, scale: int = 1_000_000_000_000_000):
    """
    Aggregate A (region-region) into groups (i01, i02, ...) using names_by_index,
    with formula: Agg = S @ A @ S.T

    - unique_codes: list [“i01”,“i02”,...] in order (len = G)
    - names_by_index[g]: list of ExioName belonging to the group unique_codes[g]
    """
    # sub-matrix region
    A_df = mrio.A.xs(region, level="region", axis=0).xs(region, level="region", axis=1)
    A_df = normalize_square_df(A_df)

    sector_labels = A_df.index.get_level_values("sector").astype(str)

    # Numeric matrix
    A = A_df.to_numpy(dtype=np.float128, copy=True)
    N = A.shape[0]
    G = len(unique_codes)

    # 4) mapping name -> index 
    name_to_pos = {name: i for i, name in enumerate(sector_labels)}

    # Construct S (GxN) indicator
    # S[g, i] = 1 if sector i in group g
    S = np.zeros((G, N), dtype=np.float128)

    missing = []  # for debug/report
    for g, names in enumerate(names_by_index):
        idxs = [name_to_pos[n] for n in names if n in name_to_pos]
        if len(idxs) != len(names):
            # track name not found in group
            miss = [n for n in names if n not in name_to_pos]
            if miss:
                missing.extend((unique_codes[g], m) for m in miss)
        if idxs:
            S[g, np.array(idxs, dtype=np.int64)] = 1.0

    # Aggregate: Agg = S @ A @ S.T
    agg = S @ A @ S.T
    agg = agg * np.int64(scale)
    # convert in int64
    agg_int = np.rint(agg).astype(np.int64)
    agg_df = pd.DataFrame(agg_int, index=unique_codes, columns=unique_codes)

    return agg_df

if __name__ == "__main__":
    # mapping group -> name
    sectors_df = load_sectors()
    unique_codes, names_by_index = build_names_by_index(sectors_df)

    print("Number code unique:", len(unique_codes))
    print(unique_codes, "-> number groups:", len(names_by_index))

    iot_path = "../Compact-data/IOT_2022_ixi.zip"
    inst = LoadInstance(iot_path)
    regions = ["AT", "BE", "BG", "CY", "CZ","DE", "DK", "EE", "ES", "FI", "FR", "GR", "HR", "HU", "IE", "IT", "LT", 
               "LU", "LV", "MT", "NL", "PL", "PT", "RO", "SE", "SI", "SK", "GB", "US", "JP", "CN", "CA", "KR", "BR", 
               "IN", "MX", "RU", "AU", "CH", "TR", "TW", "NO", "ID", "ZA", "WA", "WL", "WE", "WF", "WM"]
    
    for region in regions:
        agg_df = aggregate_A_region_by_code(inst.matrix, region, unique_codes, names_by_index)

        print("Shape matrix aggregate:", agg_df.shape)  

        # -- Test for dimensions -- 

        # out_dir = "out_aggregated"
        # os.makedirs(out_dir, exist_ok=True)
        # out_path = os.path.join(out_dir, f"A_aggregated_{region}")
        # agg_df.to_csv(out_path, sep="\t", index=True)

        out_dir = f"../Dataset/cxc_n_{agg_df.shape[0]}"
        os.makedirs(out_dir, exist_ok=True)
        out_path = os.path.join(out_dir, f"cxc_{region}_2022_n{agg_df.shape[0]}")

        mat = agg_df.to_numpy(dtype=np.int64)

        with open(out_path, "w") as f:
            f.write(f"{mat.shape[0]}\n")
            np.savetxt(f, mat, fmt="%d", delimiter="\t")

        print("Save in:", out_path)
