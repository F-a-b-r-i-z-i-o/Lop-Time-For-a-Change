from utils import * 

def build_names_by_index(sectors_df: pd.DataFrame):
    sectors_df = sectors_df.copy()
    sectors_df["base_code"] = sectors_df["ExioCode"].astype(str).str.split(".", n=1).str[0]

    unique_codes = sorted(sectors_df["base_code"].unique(), key=LoadInstance.key_extract)

    code2names = sectors_df.groupby("base_code")["ExioName"].apply(list).to_dict()
    names_by_index = [code2names[c] for c in unique_codes]  # index -> list ExioName

    return unique_codes, names_by_index

def aggregate_A_region_by_code(mrio, region: str, unique_codes, names_by_index, c: int = 1_000_000_000_000_000):
    """
    Aggregate A (region-region) into groups (i01, i02, ...) using names_by_index,
    with formula: Agg = S @ A @ S.T

    - unique_codes: list [“i01”,“i02”,...] in order (len = G)
    - names_by_index[g]: list of ExioName belonging to the group unique_codes[g]
    """
    # sub-matrix region
    A_df = mrio.A.xs(region, level="region", axis=0).xs(region, level="region", axis=1)
    sector_labels = A_df.index.get_level_values("sector").astype(str)

    A_scaled = LoadInstance.scale_and_round_df(A_df, c=c)
    
    N = A_scaled.shape[0]
    G = len(unique_codes)

    # mapping name -> index 
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
    agg = S @ A_scaled @ S.T
    agg_df = pd.DataFrame(agg, index=unique_codes, columns=unique_codes)

    return agg_df

if __name__ == "__main__":
    iot_path = "../Compact-data/IOT_2022_ixi.zip"
    inst = LoadInstance(iot_path)

    # mapping group -> name
    sectors_df = inst.load_sectors()
    unique_codes, names_by_index = build_names_by_index(sectors_df)

    print("Number code unique:", len(unique_codes))
    print(unique_codes, "-> number groups:", len(names_by_index))

    
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
        inst.save_matrix(out_path, mat)
        
