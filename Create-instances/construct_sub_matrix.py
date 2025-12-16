import pymrio
import numpy as np
import argparse
import os 

class LoadInstance:
    def __init__(self, file_path: str) -> None:
        """
        Load the EXIOBASE3 IO system once.
        """
        self.file_path = file_path
        self.matrix = self._load_matrix()
    
    def _load_matrix(self):
        """
        loads the EXIOBASE3 system.
        """
        exio_matrix = pymrio.parse_exiobase3(self.file_path)
        return exio_matrix

    def construct_sub_matrix(self, region: str, type_of_matrix_input: str) -> np.array: 
        """
        Create a square numeric sub-matrix for one region and save it as
        a CSV with NO index, NO header (so no leading comma).
        """
        # choose the matrix type: Z, Y, A, ...
        base_matrix = getattr(self.matrix, type_of_matrix_input)
        rows_region = base_matrix.xs(region, level='region', axis=0)
        cols_region = rows_region.xs(region, level='region', axis=1)

        # Convert to pure numpy array: no index, no header, just numbers
        numeric = cols_region.to_numpy(dtype=np.float128, copy=True)
        filename = f"sub_matrix_{region}_{type_of_matrix_input}"
        
        if np.isnan(numeric).any() or np.isinf(numeric).any():
            raise ValueError("Sub-matrix contains NaN/Inf values.")
        
                
        return filename, numeric

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
        help="Matrix types to extract (e.g. Y-Z-x). Default: Z",
    )
    parser.add_argument(
        "-o", "--out-dir",
        dest="out_dir",
        required=True,
        help="Folder to save the Dataset of sub-matrix",
    )
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    out_dir = args.out_dir

    print("--- Start Load Matrix ---")
    instance = LoadInstance(args.file_path)
    print("--- Matrix Loaded ---")

    os.makedirs(out_dir, exist_ok=True)

    for region in args.regions:
        for mtype in args.types_of_matrix:
            filename, numeric = instance.construct_sub_matrix(region, mtype)
            saved_path = os.path.join(out_dir, filename)  
            header = str(numeric.shape[0])               
            np.savetxt(
                saved_path,
                numeric,
                fmt="%.15g",
                delimiter=" ",   
                header=header,
                comments=""     
            )

            print(f"Saved: {saved_path} (shape={numeric.shape}), name={filename}")