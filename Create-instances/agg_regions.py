from utils import *  


def agg_region_region(df: pd.DataFrame, region_level_name: str = "region") -> pd.DataFrame:
    """
    Aggregate df to region x region by summing across the region level in rows and cols.
    """
    # Rows
    lvl = region_level_name if region_level_name in df.index.names else 0
    df = df.groupby(level=lvl).sum()
    # Cols
    lvl = region_level_name if region_level_name in df.columns.names else 0
    df = df.T.groupby(level=lvl).sum().T

    return df



def load_A_region_agg(path: str, regions: list[str]) -> pd.DataFrame:
    inst = LoadInstance(path)
    A = inst.matrix.A

    # NaN check before converting to int64
    if A.isna().to_numpy().any():
        raise ValueError(f"BIG A contains NaN values for file: {path}")

    # normalize + scale + round using class method -> numpy int64
    A_scaled_int_np = LoadInstance.scale_and_round_df(A)

    # keep the same labels for grouping
    A_scaled_int = pd.DataFrame(A_scaled_int_np, index=A.index, columns=A.columns)

    # aggregate to region x region
    A_rr_int = agg_region_region(A_scaled_int, region_level_name="region")

    # reorder subset of regions (filter to those actually present)
    present = [r for r in regions if r in A_rr_int.index and r in A_rr_int.columns]
    missing = [r for r in regions if r not in present]
    
    if missing:
        print(f"Warning: missing regions in {os.path.basename(path)} -> {missing}")

    return A_rr_int.loc[present, present]



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

    # Load dict for A_regionxregion_normalize
    A_by_year: dict[int, pd.DataFrame] = {}
    for p in folders_ixi:
        y = LoadInstance.extract_year(p)
        A_rr = load_A_region_agg(p, regions)
        A_by_year[y] = A_rr
        print(f"[{y}] A_rr shape = {A_rr.shape}")

    years_sorted = sorted(A_by_year.keys())

    # One matrix for year
    n = A_by_year[years_sorted[0]].shape[0]
    out_dir_year = f"../Dataset/rxr_n_{n}"
    os.makedirs(out_dir_year, exist_ok=True)

    for y in years_sorted:
        df = A_by_year[y]
        out_path = os.path.join(out_dir_year, f"rxr_{y}_n{df.shape[0]}")
        LoadInstance.save_matrix(out_path, df.to_numpy(dtype=np.int64))

