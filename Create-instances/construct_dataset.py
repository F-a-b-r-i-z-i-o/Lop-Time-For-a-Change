import pymrio
import numpy as np
import argparse
import os

class LoadInstance:
    def __init__(self, file_path: str) -> None:
        """Load the EXIOBASE3 IO system once."""
        self.file_path = file_path
        self.matrix = self._load_matrix()

    def _load_matrix(self):
        """loads the EXIOBASE3 system."""
        exio_matrix = pymrio.parse_exiobase3(self.file_path)
        return exio_matrix

    def construct_sub_matrix_regions(self, region: str, type_of_matrix_input: str) -> np.array:
        """
        Create a square numeric sub-matrix for one region.
        """
        base_matrix = getattr(self.matrix, type_of_matrix_input)
        rows_region = base_matrix.xs(region, level='region', axis=0)
        cols_region = rows_region.xs(region, level='region', axis=1)

        numeric = cols_region.to_numpy(dtype=np.float128, copy=True)

        SCALE = np.int64(1000000000000000)
        numeric *= SCALE

        if np.isnan(numeric).any():
            raise ValueError("Sub-matrix contains NaN values.")

        return numeric
    

def parse_args():
    parser = argparse.ArgumentParser(
        description="Extract regional sub-matrices from an EXIOBASE3 IO system."
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
        "-t", "--types-of-matrix",
        dest="types_of_matrix",
        nargs="+",
        default=["z"],
        help="Matrix types to extract (e.g. z y a x). Default: z",
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
        default="sub_matrix_{regions}_{mtype}.txt",
        help=(
            "Template for output filename. You can use placeholders: "
            "Default: sub_matrix_{regions}_{mtype}.txt"
        ),
    )
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    print("--- Start Load Matrix ---")
    instance = LoadInstance(args.file_path)
    print("--- Matrix Loaded ---")

    os.makedirs(args.out_dir, exist_ok=True)

    for region in args.regions:
        for mtype in args.types_of_matrix:
            numeric = instance.construct_sub_matrix_regions(region, mtype)

            rows, cols = numeric.shape
            filename = args.output_name_template.format(
                regions=region,
                mtype=mtype,
                rows=rows,
                cols=cols
            )

            saved_path = os.path.join(args.out_dir, filename)
            header = str(rows)
            np.savetxt(
                saved_path,
                numeric,
                fmt="%d",
                delimiter=" ",
                header=header,
                comments=""
            )

            print(f"Saved: {saved_path} (shape={numeric.shape}), name={filename}")
