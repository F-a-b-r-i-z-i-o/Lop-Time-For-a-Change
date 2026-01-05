import pymrio
import numpy as np
import argparse
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


def parse_args():
    parser = argparse.ArgumentParser(
        description="Extract regional sub-matrices from EXIOBASE3 after normalizing+scaling the full A matrix."
    )
    parser.add_argument(
        "--file_path",
        help="Path to EXIOBASE3 file (e.g. IOT_2022_ixi.zip)",
        required=True
    )
    parser.add_argument(
        "-r", "--regions",
        nargs="+",
        required=True,
        help="Region codes (e.g. IT AT). You can pass one or more.",
    )
    parser.add_argument(
        "--out-dir",
        dest="out_dir",
        default="out_submatrices",
        help="Folder to save the dataset of sub-matrices (default: out_submatrices).",
    )
    parser.add_argument(
        "--out-files-names",
        dest="output_name_template",
        default="sub_matrix_{regions}_A.txt",
        help="Template for output filename. Default: sub_matrix_{regions}_A.txt",
    )

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    print("--- Start Load Matrix ---")
    instance = LoadInstance(args.file_path)
    print("--- Matrix Loaded + A normalized + scaled ---")

    os.makedirs(args.out_dir, exist_ok=True)

    for region in args.regions:
        numeric = instance.construct_sub_matrix_regions_A(region)

        rows, cols = numeric.shape
        filename = args.output_name_template.format(
            regions=region,
            rows=rows,
            cols=cols
        )

        saved_path = os.path.join(args.out_dir, filename)
        header = str(rows)
        numeric_to_save = np.rint(numeric)
        numeric_to_save = numeric_to_save.astype(np.int64)
        np.savetxt(saved_path, numeric_to_save, fmt="%d", delimiter=" ", header=header, comments="")
        
        print(f"Saved: {saved_path} (shape={numeric.shape}), name={filename}")
