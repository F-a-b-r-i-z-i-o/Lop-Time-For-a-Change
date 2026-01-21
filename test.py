from scipy.stats import kendalltau
m = 5
set_permutations = []
set_fx = []
set_fitness_distances = [] 

def update_set(permutations:list, fitness:int):
 
    if permutations in set_permutations: 
        print("duplicate")
        return True

    if len(set_fitness_distances) < m:
        set_permutations.append(permutations)
        set_fx.append(fitness)

        row = [0] * m
        row[0] = fitness
        set_fitness_distances.append(row)
    else:
        #print("OK")
        worst_idx = set_fx.index(min(set_fx))
        if fitness <= set_fx[worst_idx]:
            return False
        else:
            set_permutations.append(permutations)
            set_fx.append(fitness)

            row = [0] * m
            row[0] = fitness
            set_fitness_distances.append(row)
            if set_fx.count(fitness) > 1:
                k = len(set_fitness_distances)
                for i in range(k):
                    row = set_fitness_distances[i]
                    row[0] = float(set_fx[i])

                    col = 1
                    for j in range(k):
                        if j == i:
                            continue
                        tau = float(kendalltau(set_permutations[i], set_permutations[j]).statistic)
                        if col < m:          
                            row[col] = tau
                            col += 1
        
                idx_min = 0
                for i in range(1, k):
                    if set_fitness_distances[i] < set_fitness_distances[idx_min]:
                        idx_min = i

                #print("before remove: ",set_fitness_distances)
                del set_fitness_distances[idx_min]
                del set_permutations[idx_min]
                del set_fx[idx_min]
                #print("after remove: ",set_fitness_distances)

                k = len(set_fitness_distances)  
                for i in range(k):
                    rowi = set_fitness_distances[i]
                    rowi[0] = float(set_fx[i])

                    col = 1
                    for j in range(k):
                        if j == i:
                            continue
                        tau = float(kendalltau(set_permutations[i], set_permutations[j]).statistic)
                        if tau != tau:
                            tau = 0.0
                        if col < m:
                            rowi[col] = tau
                            col += 1

                #print("recalculate: ", set_fitness_distances)
    
    print("\n\n")
    print(set_fitness_distances)



def update_set2(permutations: list, fitness: int):

    if permutations in set_permutations:
        print("duplicate")
       

    # 1) aggiungo sempre (divento anche 6)
    set_permutations.append(permutations)
    set_fx.append(fitness)

    # 2) se sono > m, elimino l'elemento peggiore (min fitness) per tornare a 5
    if len(set_fx) > m:
        worst_idx = set_fx.index(min(set_fx))
        del set_fx[worst_idx]
        del set_permutations[worst_idx]

    # 3) ricalcolo TUTTO da capo: set_fitness_distances deve riflettere ESATTAMENTE i 5 rimasti
    set_fitness_distances.clear()

    k = len(set_fx)  # <= m (quando pieno sarà = 5)
    for i in range(k):
        row = [0.0] * m
        row[0] = float(set_fx[i])

        col = 1
        for j in range(k):
            if j == i:
                continue
            tau = float(kendalltau(set_permutations[i], set_permutations[j]).statistic)
            if tau != tau:  # NaN -> 0
                tau = 0.0
            if col < m:
                row[col] = tau
                col += 1

        set_fitness_distances.append(row)

    print(set_fitness_distances)
        


        

if __name__ == "__main__": 
   permutations = [
    [1, 2, 3, 4, 5],
    [1, 2, 4, 6, 5],
    [1, 3, 2, 4, 5],
    [2, 1, 3, 4, 5],
    [2, 3, 1, 4, 5],
    [3, 1, 2, 4, 5],
    [3, 2, 1, 4, 5],
    [4, 1, 2, 3, 5],
    [5, 4, 3, 2, 1],
    [2, 5, 1, 4, 3],
]

fitness = [
    150,
    178,
    198,
    283,
    435,
    345,
    354,
    435,
    439,
    2859,
]

for i, (p, f) in enumerate(zip(permutations, fitness)):
    update_set2(p, f)
