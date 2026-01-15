from utils import *



def construct_sub_matrix_regions_A(mrio, region: str) -> np.ndarray:
    """
    Scale+round the BIG A first (int64 numpy),
    then extract the region-region sub-matrix.
    """
    A_df = mrio.A  

    if A_df.isna().to_numpy().any():
        raise ValueError("BIG A contains NaN values (cannot scale+cast to int64).")

    # scale+round the BIG A (could be DF or ndarray depending on your impl)
    A_scaled = LoadInstance.scale_and_round_df(A_df)
    A_int = A_scaled.to_numpy() if hasattr(A_scaled, "to_numpy") else np.asarray(A_scaled)

    # build masks from the ORIGINAL labels
    row_mask = (A_df.index.get_level_values("region") == region).to_numpy()
    col_mask = (A_df.columns.get_level_values("region") == region).to_numpy()

    if not row_mask.any() or not col_mask.any():
        raise ValueError(f"Region '{region}' not found in A index/columns.")

    # extract region-region block
    sub_int = A_int[np.ix_(row_mask, col_mask)]

    # make it a DataFrame  
    sub_index = A_df.index[row_mask]
    sub_cols  = A_df.columns[col_mask]
    sub_df = pd.DataFrame(sub_int, index=sub_index, columns=sub_cols)

    #sub_df, _ = LoadInstance.remove_useless_items(sub_df)

    return sub_df.to_numpy()





if __name__ == "__main__":
    iot_path = "../Compact-data/IOT_2022_pxp.zip"
    inst = LoadInstance(iot_path)

    regions = [
        "AT", "BE", "BG", "CY", "CZ", "DE", "DK", "EE", "ES", "FI", "FR", "GR", "HR", "HU",
        "IE", "IT", "LT", "LU", "LV", "MT", "NL", "PL", "PT", "RO", "SE", "SI", "SK", "GB",
        "US", "JP", "CN", "CA", "KR", "BR", "IN", "MX", "RU", "AU", "CH", "TR", "TW", "NO",
        "ID", "ZA", "WA", "WL", "WE", "WF", "WM"
    ]

    for region in regions:
        mat = construct_sub_matrix_regions_A(inst.matrix, region)

        print("Shape matrix:", mat.shape)

        out_dir = f"../Dataset/pxp_n_{mat.shape[0]}"
        out_path = os.path.join(out_dir, f"pxp_{region}_2022_n{mat.shape[0]}")

        inst.save_matrix(out_path, mat)
