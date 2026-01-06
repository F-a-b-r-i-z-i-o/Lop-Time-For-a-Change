import pymrio
import numpy as np
import os
import pandas as pd


class LoadInstance:
    def __init__(self, file_path: str) -> None:
        """Load the EXIOBASE3 IO system once, then normalize+scale A."""
        self.file_path = file_path
        self.c = 1_000_000_000_000_000
        self.matrix = self._load_matrix()

        # Precompute: normalized and scaled A
        self.A_norm_scaled = self._normalize_and_scale_square_matrix(self.matrix.A, self.c)

    def _load_matrix(self):
        """Loads the EXIOBASE3 system."""
        exio_matrix = pymrio.parse_exiobase3(self.file_path)
        return exio_matrix

    @staticmethod
    def _normalize_and_scale_square_matrix(M: pd.DataFrame, c: np.int64) -> pd.DataFrame:
        """ 
        a*_ij = a_ij - min(a_ij, a_ji) on the full matrix, then scale by c.
        """

        # to numeric (high precision for stability)
        vals = M.to_numpy(dtype=np.longdouble, copy=True)

        # a*_ij = a_ij - min(a_ij, a_ji)
        min_vals = np.minimum(vals, vals.T)
        norm = vals - min_vals  # diagonal becomes 0 (since min(a_ii,a_ii)=a_ii)

        # scale after normalization
        scaled = norm * np.int64(c)

        return pd.DataFrame(scaled, index=M.index, columns=M.columns)


    def construct_sub_matrix_regions_A(self, region: str) -> np.ndarray:
        """
        Create a square numeric sub-matrix for one region,
        extracted from the already normalized+scaled big A.
        """
        base_matrix = self.A_norm_scaled

        rows_region = base_matrix.xs(region, level='region', axis=0)
        cols_region = rows_region.xs(region, level='region', axis=1)

        numeric = cols_region.to_numpy(dtype=np.longdouble, copy=True)

        if np.isnan(numeric).any():
            raise ValueError("Sub-matrix contains NaN values.")

        return numeric

def save_matrix(out_path: str, mat: np.ndarray) -> None:
    """
    Save format:
        n
        <matrix n x n>
    """
    os.makedirs(os.path.dirname(out_path), exist_ok=True)

    with open(out_path, "w") as f:
        f.write(f"{mat.shape[0]}\n")
        np.savetxt(f, mat, fmt="%d", delimiter=" ")


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
        mat = inst.construct_sub_matrix_regions_A(region)  # <-- this is already a numpy array

        print("Shape matrix:", mat.shape)

        out_dir = f"../Dataset/pxp_n_{mat.shape[0]}"
        out_path = os.path.join(out_dir, f"cxc_{region}_2022_n{mat.shape[0]}")

        save_matrix(out_path, mat)

        print("Saved in:", out_path)