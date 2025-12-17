import numpy as np

def read_space_matrix_10x10(path):
    with open(path, "r") as f:
        txt = f.read()
    flat = np.fromstring(txt, sep=" ")
    M = flat[:100].reshape((10, 10))
    return M

if __name__ == "__main__":
    path = "Create-instances/sub_matrix_IT_Z" 
    M = read_space_matrix_10x10(path)
    print("Shape:", M.shape)
    print(M)
    out_path = "Create-instances/sub_matrix_IT_Z_10x10.csv"
    np.savetxt(out_path, M, delimiter=" ", fmt="%.18e")
    print("Save CSV in:", out_path)
