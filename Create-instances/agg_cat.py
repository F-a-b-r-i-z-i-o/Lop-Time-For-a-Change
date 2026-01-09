from utils import * 

def build_groups_by_consumption_categories(sectors_df: pd.DataFrame):
    df = sectors_df[["ExioName", "ConsumptionCategories"]].copy()

    df["ExioName"] = df["ExioName"].astype(str).str.strip()
    df["ConsumptionCategories"] = df["ConsumptionCategories"].astype(str).str.strip()

    unique_groups = list(pd.unique(df["ConsumptionCategories"]))
    group2names = df.groupby("ConsumptionCategories")["ExioName"].apply(list).to_dict()

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
    names_by_index
):
    # extract region submatrix
    A_df = mrio.A.xs(region, level="region", axis=0).xs(region, level="region", axis=1)

    # keep A_df (needed for sector labels)
    sector_labels = A_df.index.get_level_values("sector").astype(str)

    # normalize + scale + round 
    A_scaled = LoadInstance.scale_and_round_df(A_df)

    N = A_scaled.shape[0]
    G = len(unique_groups)

    name_to_pos = {name: i for i, name in enumerate(sector_labels)}

    S = np.zeros((G, N), dtype=np.int64)

    missing = []
    for g, names in enumerate(names_by_index):
        idxs = [name_to_pos[n] for n in names if n in name_to_pos]
        if len(idxs) != len(names):
            miss = [n for n in names if n not in name_to_pos]
            if miss:
                missing.extend((unique_groups[g], m) for m in miss)
        if idxs:
            S[g, np.array(idxs, dtype=np.int64)] = 1

    agg_int = S @ A_scaled @ S.T
    return pd.DataFrame(agg_int, index=unique_groups, columns=unique_groups)



if __name__ == "__main__":
    iot_path = "../Compact-data/IOT_2022_ixi.zip"
    inst = LoadInstance(iot_path)

    sectors_df = LoadInstance.load_sectors()
    unique_groups, names_by_index = build_groups_by_consumption_categories(sectors_df)

    print("Number of ConsumptionCategories:", len(unique_groups))
    print("Categories:", unique_groups)

    regions = ["AT", "BE", "BG", "CY", "CZ", "DE", "DK", "EE", "ES", "FI", "FR", "GR", "HR", "HU",
               "IE", "IT", "LT", "LU", "LV", "MT", "NL", "PL", "PT", "RO", "SE", "SI", "SK", "GB",
               "US", "JP", "CN", "CA", "KR", "BR", "IN", "MX", "RU", "AU", "CH", "TR", "TW", "NO",
               "ID", "ZA", "WA", "WL", "WE", "WF", "WM"]

    for region in regions:
        agg_df = aggregate_A_region_by_groups(inst.matrix, region, unique_groups, names_by_index)
        print("Shape matrix aggregate:", agg_df.shape)

        out_dir = f"../Dataset/cxc_n_{agg_df.shape[0]}"
        os.makedirs(out_dir, exist_ok=True)
        out_path = os.path.join(out_dir, f"mcxmc_{region}_2022_n{agg_df.shape[0]}")

        mat = agg_df.to_numpy(dtype=np.int64)
        LoadInstance.save_matrix(out_path, mat)


