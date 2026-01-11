from utils import *


def construct_sub_matrix_regions_A(mrio, region: str) -> np.ndarray:
    """
    Scale+round the BIG A first (int64 numpy),
    then extract the region-region sub-matrix.
    """
    # Big A as labeled DataFrame (MultiIndex on rows/cols: region, sector)
    A_df = mrio.A

    # NaN check BEFORE scaling 
    if A_df.isna().to_numpy().any():
        raise ValueError("BIG A contains NaN values (cannot scale+cast to int64).")

    # scale+round the BIG A (returns numpy int64, same order as A_df)
    A_int = LoadInstance.scale_and_round_df(A_df)

    # build masks from the ORIGINAL labels (because A_int has no labels)
    row_mask = (A_df.index.get_level_values("region") == region)
    col_mask = (A_df.columns.get_level_values("region") == region)

    if not row_mask.any() or not col_mask.any():
        raise ValueError(f"Region '{region}' not found in A index/columns.")

    # extract region-region block from the integer matrix
    sub_int = A_int[np.ix_(row_mask, col_mask)]

    return sub_int


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
