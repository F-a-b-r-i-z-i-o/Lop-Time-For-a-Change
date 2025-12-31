import pymrio
import pandas as pd
import numpy as np

class LoadInstance:
    def __init__(self, file_path: str) -> None:
        self.file_path = file_path
        self.matrix = self._load_matrix()
    
    def _load_matrix(self):
        return np.loadtxt(file_path, skiprows=1)
        
    def _calculate_sum(self):
        return np.asarray(self.matrix).sum(dtype=np.float128)

    def _calculate_sum_normalize(self): 
        self.matrix = np.asarray(self.matrix, dtype=np.float64)
        norm = self.matrix - np.minimum(self.matrix, self.matrix.T)
        S_norm = norm.sum()
        return S_norm
        




if __name__ == "__main__":
    file_path = "Dataset/sub_matrix_IT_A"
    print("--LOAD MATRIX--")
    matrix = LoadInstance(file_path=file_path)
    print("---LOADED")
    matrix_sum = matrix._calculate_sum()
    print(f"SUM MATRIX = {matrix_sum}")
    matrix_sum = matrix._calculate_sum_normalize()
    print(f"SUM MATRIX NORMALIZE = {matrix_sum}")
    