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
    vals = vals - np.minimum(vals, vals.T)
    return pd.DataFrame(vals, index=A_df.index, columns=A_df.columns)

def build_groups_by_consumption_categories(
    sectors_df: pd.DataFrame,
    category_col: str = "ConsumptionCategories",
    sector_name_col: str = "ExioName",
):
    """
    Build aggregation groups using ConsumptionCategories.
    Returns:
      - unique_groups: list of category labels in order of appearance in sectors.tsv
      - names_by_index[g]: list of ExioName belonging to that category
    """

    df = sectors_df[[sector_name_col, category_col]].copy()

    # Clean strings
    df[sector_name_col] = df[sector_name_col].astype(str).str.strip()
    df[category_col] = df[category_col].astype(str).str.strip()

    # Keep categories in file order 
    unique_groups = list(pd.unique(df[category_col]))

    # category -> list of ExioName
    group2names = df.groupby(category_col)[sector_name_col].apply(list).to_dict()

    # De-duplicate names inside each group (preserving order)
    names_by_index = []
    for g in unique_groups:
        names = group2names.get(g, [])
        names = list(dict.fromkeys(names))
        names_by_index.append(names)

    return unique_groups, names_by_index

def aggregate_A_region_by_groups(
    mrio,
    region: str,
    unique_groups,
    names_by_index,
    scale: int = 1_000_000_000_000_000
):
    """
    Aggregate A (region-region) into groups using names_by_index,
    with formula: Agg = S @ A @ S.T

    - unique_groups: list of group labels (len = G)
    - names_by_index[g]: list of ExioName belonging to group unique_groups[g]
    """
    # sub-matrix region
    A_df = mrio.A.xs(region, level="region", axis=0).xs(region, level="region", axis=1)
    A_df = normalize_square_df(A_df)

    sector_labels = A_df.index.get_level_values("sector").astype(str)

    # Numeric matrix
    A = A_df.to_numpy(dtype=np.float128, copy=True)
    N = A.shape[0]
    G = len(unique_groups)

    # mapping ExioName -> index in A
    name_to_pos = {name: i for i, name in enumerate(sector_labels)}

    # Construct S (GxN) indicator
    S = np.zeros((G, N), dtype=np.float128)

    missing = []
    for g, names in enumerate(names_by_index):
        idxs = [name_to_pos[n] for n in names if n in name_to_pos]
        if len(idxs) != len(names):
            miss = [n for n in names if n not in name_to_pos]
            if miss:
                missing.extend((unique_groups[g], m) for m in miss)
        if idxs:
            S[g, np.array(idxs, dtype=np.int64)] = 1.0

    # Aggregate: Agg = S @ A @ S.T
    agg = S @ A @ S.T
    agg = agg * np.int64(scale)
    agg_int = np.rint(agg).astype(np.int64)

    agg_df = pd.DataFrame(agg_int, index=unique_groups, columns=unique_groups)

    return agg_df

if __name__ == "__main__":
    # grouping by ConsumptionCategories
    sectors_df = load_sectors()
    unique_groups, names_by_index = build_groups_by_consumption_categories(sectors_df)

    print("Number of ConsumptionCategories:", len(unique_groups))
    print("Categories:", unique_groups)

    iot_path = "../Compact-data/IOT_2022_ixi.zip"
    inst = LoadInstance(iot_path)

    regions = ["AT", "BE", "BG", "CY", "CZ","DE", "DK", "EE", "ES", "FI", "FR", "GR", "HR", "HU", "IE", "IT", "LT",
               "LU", "LV", "MT", "NL", "PL", "PT", "RO", "SE", "SI", "SK", "GB", "US", "JP", "CN", "CA", "KR", "BR",
               "IN", "MX", "RU", "AU", "CH", "TR", "TW", "NO", "ID", "ZA", "WA", "WL", "WE", "WF", "WM"]

    for region in regions:
        agg_df = aggregate_A_region_by_groups(inst.matrix, region, unique_groups, names_by_index)
        print("Shape matrix aggregate:", agg_df.shape)

        out_dir = f"../Dataset/cxc_n_{agg_df.shape[0]}"
        os.makedirs(out_dir, exist_ok=True)
        out_path = os.path.join(out_dir, f"mcxmc_{region}_2022_n{agg_df.shape[0]}")

        mat = agg_df.to_numpy(dtype=np.int64)

        with open(out_path, "w") as f:
            f.write(f"{mat.shape[0]}\n")
            np.savetxt(f, mat, fmt="%d", delimiter=" ")

        print("Save in:", out_path)
