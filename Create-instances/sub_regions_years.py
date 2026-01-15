from utils import *
import os
import numpy as np
import pandas as pd  # ✅ add this


def construct_sub_matrix_regions_A_scaled_int(mrio, region: str) -> pd.DataFrame:
    """
    Scale+round BIG A first (int64 numpy),
    then extract the region-region sub-matrix,
    returning a DataFrame (keeps labels for remove_useless_items).
    """
    A_df = mrio.A

    if A_df.isna().to_numpy().any():
        raise ValueError("BIG A contains NaN values (cannot scale+cast to int64).")

    # scale+round BIG A (numpy int64, same order as A_df)
    A_int = LoadInstance.scale_and_round_df(A_df)

    row_mask = (A_df.index.get_level_values("region") == region)
    col_mask = (A_df.columns.get_level_values("region") == region)

    if not row_mask.any() or not col_mask.any():
        raise ValueError(f"Region '{region}' not found in A index/columns.")

    sub_int = A_int[np.ix_(row_mask, col_mask)]

    sub_df = pd.DataFrame(
        sub_int.astype(np.int64, copy=False),
        index=A_df.index[row_mask],
        columns=A_df.columns[col_mask],
    )
    return sub_df


def sum_sub_matrices_over_paths(iot_paths, region: str) -> np.ndarray:
    """
    For a given region, load each MRIO instance from iot_paths,
    extract scaled+rounded A[region,region] and sum across paths.
    """
    acc_df = None

    for p in iot_paths:
        inst = LoadInstance(p)
        sub_df = construct_sub_matrix_regions_A_scaled_int(inst.matrix, region)

        if acc_df is None:
            acc_df = sub_df.copy()
        else:
            if not acc_df.index.equals(sub_df.index) or not acc_df.columns.equals(sub_df.columns):
                raise ValueError(
                    f"Index/columns mismatch for region {region} between instances. Problem file: {p}"
                )
            acc_df.iloc[:, :] = acc_df.to_numpy(np.int64) + sub_df.to_numpy(np.int64)

    if acc_df is None:
        raise ValueError("No paths provided (iot_paths is empty).")

    acc_df, _ = LoadInstance.remove_useless_items(acc_df)

    return acc_df.to_numpy(dtype=np.int64)



if __name__ == "__main__":
    iot_paths = [
        "../Compact-data/IOT_1995_pxp.zip",
        # "../Compact-data/IOT_2019_pxp.zip",
        # "../Compact-data/IOT_2020_pxp.zip",
        # "../Compact-data/IOT_2021_pxp.zip",
        "../Compact-data/IOT_2022_pxp.zip",
    ]

    regions = [
        "AT", "BE", "BG", "CY", "CZ", "DE", "DK", "EE", "ES", "FI", "FR", "GR", "HR", "HU",
        "IE", "IT", "LT", "LU", "LV", "MT", "NL", "PL", "PT", "RO", "SE", "SI", "SK", "GB",
        "US", "JP", "CN", "CA", "KR", "BR", "IN", "MX", "RU", "AU", "CH", "TR", "TW", "NO",
        "ID", "ZA", "WA", "WL", "WE", "WF", "WM"
    ]

    saver_inst = LoadInstance(iot_paths[0])

    for region in regions:
        mat_sum = sum_sub_matrices_over_paths(iot_paths, region)

        print(region, "Shape matrix:", mat_sum.shape)

        out_dir = f"../Dataset/pxp_SUM_n_{mat_sum.shape[0]}"
        os.makedirs(out_dir, exist_ok=True)

        out_path = os.path.join(out_dir, f"pxp_{region}_SUM_n{mat_sum.shape[0]}")

        saver_inst.save_matrix(out_path, mat_sum)
