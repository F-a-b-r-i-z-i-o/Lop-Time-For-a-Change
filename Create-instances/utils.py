import os
import re
import numpy as np
import pandas as pd
import pymrio
import pymrio.mrio_models.exio3_ixi as model


class LoadInstance:
    def __init__(self, file_path: str) -> None:
        self.file_path = file_path
        print(f"---Loading Matrix---")
        self.matrix = self._load_matrix()
        
    def _load_matrix(self):
        return pymrio.parse_exiobase3(self.file_path)

    @staticmethod
    def extract_year(path: str) -> int:
        m = re.search(r"IOT_(\d{4})", os.path.basename(path))
        if not m:
            raise ValueError(f"Cannot extract year from: {path}")
        return int(m.group(1))

    @staticmethod
    def load_sectors() -> pd.DataFrame:
        base_dir = os.path.dirname(model.__file__)
        sectors = os.path.join(base_dir, "sectors.tsv")
        return pd.read_csv(sectors, sep="\t")

    @staticmethod
    def key_extract(code: str) -> int:
        # i01 -> 1, i10 -> 10 ...
        return int(code[1:])
    
    @staticmethod
    def normalize_square_df(A_df: pd.DataFrame) -> pd.DataFrame:
        """
        Return the normalized matrix A* where:
            a*_ij = a_ij - min(a_ij, a_ji)
        """
        vals = A_df.to_numpy(dtype=np.float128, copy=True)
        vals = vals - np.minimum(vals, vals.T)
        return pd.DataFrame(vals, index=A_df.index, columns=A_df.columns)

    @staticmethod
    def scale_and_round_df(
        A_df: pd.DataFrame,
        c: int = 1_000,
    ) -> np.ndarray:
        """
        normalize A_df, then compute:
            A_scaled = round(A * c)
        returning an int64 numpy matrix.
        """
        
        A_df = LoadInstance.normalize_square_df(A_df)
        A = A_df.to_numpy(dtype=np.float128, copy=True)
        return np.rint(A * np.int64(c)).astype(np.int64)

    @staticmethod
    def save_matrix(out_path: str, mat: np.ndarray) -> None:
        """
        Save format:
            n
            <matrix n x n>
        """
        out_dir = os.path.dirname(out_path)
        if out_dir:
            os.makedirs(out_dir, exist_ok=True)

        with open(out_path, "w") as f:
            f.write(f"{mat.shape[0]}\n")
            np.savetxt(f, mat, fmt="%d", delimiter=" ")
        print("Saved in:", out_path)
