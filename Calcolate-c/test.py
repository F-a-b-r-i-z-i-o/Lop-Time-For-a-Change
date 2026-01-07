import os
import numpy as np


class LoadInstance:
    def __init__(self, file_path: str) -> None:
        self.file_path = file_path
        self.matrix = self._load_matrix()

    def _load_matrix(self):
        return np.loadtxt(self.file_path, skiprows=1)

    def calculate_sum_and_shape(self):
        arr = np.asarray(self.matrix)
        return arr.sum(dtype=np.float128), arr.shape


def iter_instance_files(folder: str):
    for name in sorted(os.listdir(folder)):
        full = os.path.join(folder, name)
        if not os.path.isfile(full):
            continue
        yield full


if __name__ == "__main__":
    folder_path = "../Dataset/pxp_n_200"
    U_long_max = 18446744073709551615
    

    print("--SCAN FOLDER--")
    print(f"Folder: {folder_path}")
    print(f"U_long_max: {U_long_max}")

    any_problem = False

    for file_path in iter_instance_files(folder_path): 
        file_name = os.path.basename(file_path)
        try:
            inst = LoadInstance(file_path=file_path)
            s, shape = inst.calculate_sum_and_shape()

            if np.isfinite(s):
                s_int = s
                ok = s_int < U_long_max
            else:
                s_int = None
                ok = False

            print(f"\nFILE: {file_name}")
            print(f"  SHAPE: {shape}")
            print(f"  SUM:   {s:.10f}")
            print(f"  CHECK sum < U_long_max: {ok}")
            print(f"  {U_long_max-s}")

            if not ok:
                any_problem = True
                if s_int is not None:
                    print(f"DIFF (sum - U_long_max): {s_int - U_long_max}")
                else:
                    print("(Infinite sum: NaN/Inf)")

        except Exception as e:
            any_problem = True
            print(f"\nFILE: {file_name}")
            print(f"  ERROR: {e}")

    print("\n--DONE--")
    print("Overall result:", "OK (all under U_long_max)" if not any_problem else "ATTENTION (error or overflow)")

    