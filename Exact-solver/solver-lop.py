import gurobipy as gp
from gurobipy import GRB
import os 

class GurobySolver:
    def __init__(self, file_name: str) -> None: 
        self.matrix = []
        self.filename = file_name
    
    def load_problem(self):
        with open(self.filename, "r") as f: 
            next(f)
            for line in f: 
                row = list(map(float, line.split()))
                self.matrix.append(row)
    
    def save_solution(self, perm, objval, out_dir=None, out_name="solution.txt"):
        """
        Save objvalue and best permutation 
        """
        if out_dir is None:
            out_dir = self.filename  

        os.makedirs(out_dir, exist_ok=True)
        out_path = os.path.join(out_dir, out_name)

        with open(out_path, "w") as f:
            f.write(f"obj={objval}\n")
            f.write("perm_0based=" + " ".join(map(str, perm)) + "\n")
            f.write("perm_1based=" + " ".join(str(i + 1) for i in perm) + "\n")

        return out_path
    
    def solve_lop(self, timelimit: int = None, threads:int = None, verbose:bool = True) -> list[int] | float | object:
        
        """
        obj_matrix: nxn (list of lists) with float values.
        returns: (perm, obj, model)
            perm: list of index sorted (0..n-1)
            obj: val objfun
            model: gurobi obj 
        """

        if not self.matrix: 
            self.load_problem()

        n = len(self.matrix)
        assert all(len(row) == n for row in self.matrix), "Matrix must be NxN"

        m = gp.Model("LOP")
        # Activate or deactivate log to verbose flag 
        m.Params.OutputFlag = 1 if verbose else 0

        if timelimit is not None:
            m.Params.TimeLimit = timelimit

        if threads is not None:
            m.Params.Threads = threads

        # y[i,j] = 1 if i before j 
        y = m.addVars(n, n, vtype=GRB.BINARY, name="y")
        # integer position: 0..n-1
        p = m.addVars(n, vtype=GRB.INTEGER, lb=0, ub=n-1, name="p")

        # exclude diagonal
        for i in range(n):
            m.addConstr(y[i, i] == 0, name=f"diag_{i}")

        # anti-sim: y_ij + y_ji = 1
        for i in range(n):
            for j in range(i + 1, n):
                m.addConstr(y[i, j] + y[j, i] == 1, name=f"anti_{i}_{j}")

        # Big m for avoid loop 
        # if y[i,j]=1 => p_i <= p_j - 1
        for i in range(n):
            for j in range(n):
                if i == j:
                    continue
                m.addConstr(p[i] - p[j] + n * y[i, j] <= n - 1, name=f"mtz_{i}_{j}")

        # objective (float/double ok)
        m.setObjective(gp.quicksum(float(self.matrix[i][j]) * y[i, j] for i in range(n) for j in range(n) if i != j), GRB.MAXIMIZE)

        m.optimize()

        if m.Status not in (GRB.OPTIMAL, GRB.TIME_LIMIT):
            raise RuntimeError(f"Optimization fail. Status={m.Status}")
        
        # find permutation sorted for p[i]
        p_val = {i: p[i].X for i in range(n)}
        perm = sorted(range(n), key=lambda i: p_val[i])

        return perm, m.ObjVal, m


if __name__ == "__main__":
    filename = "Dataset/sub_matrix_AT_Z"
    solver = GurobySolver(file_name=filename)
    perm, mobjval, m = solver.solve_lop()
    print(perm)
    print(mobjval)
    print(m)
    saved_path = solver.save_solution(perm, mobjval)
    print("Saved to:", saved_path)


