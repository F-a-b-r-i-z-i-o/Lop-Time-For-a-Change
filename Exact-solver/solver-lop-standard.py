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
    
    def save_solution(self, objval, out_file="output/solution.txt"):
        """
        Save objvalue and best permutation in a single output file.
        """
        out_dir = os.path.dirname(out_file) or "."
        os.makedirs(out_dir, exist_ok=True)

        with open(out_file, "w", encoding="utf-8") as f:
            f.write(f"obj={objval}\n")
            f.write("perm_=" + " ".join(map(str, perm)) + "\n")
        return out_file

    
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
		for k in range(n):
			if k==i or k==j:
				continue
	                m.addConstr(y[i, j] + y[j, k] + y[k, i] <= 2, name=f"mtz_{i}_{j}")

        # objective (float/double ok)
        m.setObjective(gp.quicksum(float(self.matrix[i][j]) * y[i, j] for i in range(n) for j in range(n) if i != j), GRB.MAXIMIZE)

        m.optimize()

        if m.Status not in (GRB.OPTIMAL, GRB.TIME_LIMIT):
            raise RuntimeError(f"Optimization fail. Status={m.Status}")
        
        # find permutation sorted for p[i]
        return m.ObjVal, m


if __name__ == "__main__":
    filename = "Dataset/sub_matrix_AT_Z"
    solver = GurobySolver(file_name=filename)
    mobjval, m = solver.solve_lop()
    print(mobjval)
    print(m)
    saved_path = solver.save_solution(mobjval, out_file="output/solution_standard.txt")
    print("Saved to:", saved_path)


