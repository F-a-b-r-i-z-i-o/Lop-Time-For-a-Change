import os
import re
import numpy as np
import pandas as pd
import pymrio


def normalize_square_df(A_df: pd.DataFrame) -> pd.DataFrame:
    """
    Normalization with: 
        a*_ij = a_ij - min(a_ij, a_ji)
    """
    vals = A_df.to_numpy(dtype=np.float64, copy=True)
    vals = vals - np.minimum(vals, vals.T)
    return pd.DataFrame(vals, index=A_df.index, columns=A_df.columns)


def extract_year(path: str) -> int:
    m = re.search(r"IOT_(\d{4})", os.path.basename(path))
    return int(m.group(1))


def agg_region_region(df: pd.DataFrame, region_level_name: str = "region") -> pd.DataFrame:
    """
    Aggregate df with regionxregion with sum of the level 
    """
    # Rows
    lvl = region_level_name if region_level_name in df.index.names else 0
    df = df.groupby(level=lvl).sum()

    # Cols
    lvl = region_level_name if region_level_name in df.columns.names else 0
    df = df.T.groupby(level=lvl).sum().T

    return df


def load_A_region_agg(path: str, regions: list[str], c: int = 1_000_000_000_000_000,) -> pd.DataFrame:
    mrio = pymrio.parse_exiobase3(path)
    A = mrio.A
    A_norm = normalize_square_df(A)

    # scale + round to integer aggregation
    A_scaled_int = np.rint(A_norm.to_numpy(dtype=np.float64) * np.int64(c)).astype(np.int64)
    A_scaled_int = pd.DataFrame(A_scaled_int, index=A_norm.index, columns=A_norm.columns)

    # aggregate 
    A_rr_int = agg_region_region(A_scaled_int, region_level_name="region")

    # reorder subset to requested regions
    A_rr_int = A_rr_int.loc[regions, regions]

    return A_rr_int



def save_matrix(out_path: str, mat: np.ndarray) -> None:
    """
    Save format:
        n
        <matrix n x n>
    """
    with open(out_path, "w") as f:
        f.write(f"{mat.shape[0]}\n")
        np.savetxt(f, mat, fmt="%d", delimiter=" ")


if __name__ == "__main__":
    folders_ixi = [
        "../Compact-data/IOT_1995_ixi.zip",
        # "../Compact-data/IOT_1996_ixi.zip",
        # "../Compact-data/IOT_1997_ixi.zip",
        # "../Compact-data/IOT_1998_ixi.zip",
        # "../Compact-data/IOT_1999_ixi.zip",
        # "../Compact-data/IOT_2000_ixi.zip",
        # "../Compact-data/IOT_2001_ixi.zip",
        # "../Compact-data/IOT_2002_ixi.zip",
        # "../Compact-data/IOT_2003_ixi.zip",
        # "../Compact-data/IOT_2004_ixi.zip",
        # "../Compact-data/IOT_2005_ixi.zip",
        # "../Compact-data/IOT_2006_ixi.zip",
        # "../Compact-data/IOT_2007_ixi.zip",
        # "../Compact-data/IOT_2008_ixi.zip",
        # "../Compact-data/IOT_2009_ixi.zip",
        # "../Compact-data/IOT_2010_ixi.zip",
        # "../Compact-data/IOT_2011_ixi.zip",
        # "../Compact-data/IOT_2012_ixi.zip",
        # "../Compact-data/IOT_2013_ixi.zip",
        # "../Compact-data/IOT_2014_ixi.zip",
        # "../Compact-data/IOT_2015_ixi.zip",
        # "../Compact-data/IOT_2016_ixi.zip",
        # "../Compact-data/IOT_2017_ixi.zip",
        # "../Compact-data/IOT_2018_ixi.zip",
        # "../Compact-data/IOT_2019_ixi.zip",
        # "../Compact-data/IOT_2020_ixi.zip",
        # "../Compact-data/IOT_2021_ixi.zip",
        "../Compact-data/IOT_2022_ixi.zip",
    ]

    regions = ["AT","BE","BG","CY","CZ","DE","DK","EE","ES","FI","FR","GR","HR","HU","IE","IT","LT",
               "LU","LV","MT","NL","PL","PT","RO","SE","SI","SK","GB","US","JP","CN","CA","KR","BR",
               "IN","MX","RU","AU","CH","TR","TW","NO","ID","ZA","WA","WL","WE","WF","WM"]

    # Load dict for  A_regionxregion_normalize
    A_by_year: dict[int, pd.DataFrame] = {}
    for p in folders_ixi:
        y = extract_year(p)
        A_rr = load_A_region_agg(p, regions)
        A_by_year[y] = A_rr
        print(f"[{y}] A_rr shape = {A_rr.shape}")

    years_sorted = sorted(A_by_year.keys())

    # One matrix for year
    out_dir_year = f"../Dataset/rxr_n_{A_rr.shape[0]}"
    os.makedirs(out_dir_year, exist_ok=True)

    for y in years_sorted:
        df = A_by_year[y]
        # with labels for test
        #df.to_csv(os.path.join(out_dir_year, f"A_rr_norm_{y}.tsv"), sep="\t")
        
        # format
        save_matrix(os.path.join(out_dir_year, f"rxr_{y}_n{df.shape[0]}"),df.to_numpy())


