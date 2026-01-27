import pymrio
import numpy as np
import sys
import pandas as pd
import os
import shutil
import json
import warnings


def build_lop_instance(A):
    #normalize A when it contains floats
    A -= np.minimum(A,A.T)
    #scale and round A to integer
    c = 100_000
    A = np.rint(A*c).astype(int)
    #normalize A now that contains ints (just to be sure)
    A -= np.minimum(A,A.T)
    #remove useless items from A
    useful_indices = np.sort( np.where( (A.sum(axis=1)>0) | (A.sum(axis=0)>0) )[0] ) #at least one of rowsum or colsum positive to make it useful
    A = A[ np.ix_(useful_indices,useful_indices) ]
    indices_reverse_map = { int(old): int(new) for new,old in enumerate(useful_indices) }
    #A is now the LOP instance matrix, but returns also indices_reverse_map which may be useful for future works
    return A, indices_reverse_map


def save_lop_matrix(filename: str, A: np.ndarray) -> None:
    with open(filename, "w") as f:
        f.write(f"{A.shape[0]}\n")
        np.savetxt(f, A, fmt="%d", delimiter=" ")


def get_back_full_matrix(Ared, n_full, indices_reverse_map):
    #possibly useful for future work, but never tested!!!!
    Afull = np.empty( (n_full,n_full), dtype=Ared.dtype )
    for i in range(n_full):
        for j in range(n_full):
            if i in indices_reverse_map and j in indices_reverse_map:
                ii = indices_reverse_map[i]
                jj = indices_reverse_map[j]
                Afull[i,j] = A[ii,jj]
            else:
                Afull[i,j] = 0
    return Afull



#ignore warnings
warnings.filterwarnings("ignore")

#read command line arguments
'''
if len(sys.argv)<3:
    sys.exit('USAGE: python build_benchmark_suite.py <REAL_DATA_DIR> <OUTPUT_DIR>')
indir = sys.argv[1]
outdir = sys.argv[2]
'''
indir = 'real_data'
outdir = 'exiobase'

#delete everything was existing in output directory
if os.path.exists(outdir):
    shutil.rmtree(outdir)
os.mkdir(outdir)
os.mkdir(f'{outdir}/revmap')

#initialize random number generator
np.random.seed(42)



#################################
##### ixi and pxp instances #####
#################################
year = 2022 #last year available
for typ in ['ixi', 'pxp']:
    os.mkdir(f'{outdir}/{typ}')
    in_filename = f'{indir}/IOT_{year}_{typ}.zip'
    ios = pymrio.parse_exiobase3(in_filename)
    for reg in ios.regions:
        A = ios.A.loc[reg,reg].to_numpy()
        A, indices_reverse_map = build_lop_instance(A)
        out_filename = f'{typ}_{reg.lower()}_{year}_n{A.shape[0]}'
        save_lop_matrix(f'{outdir}/{typ}/{out_filename}', A)
        with open(f'{outdir}/revmap/revmap_{out_filename}','w') as f:
            json.dump(indices_reverse_map,f)
        print(f'* {out_filename} DONE.')


#################################
##### rxr instances (49x49) #####
#################################
from_year, to_year = 1995, 2022
typ = 'rxr'
os.mkdir(f'{outdir}/{typ}')
for year in range(from_year,to_year+1):
    in_filename = f'{indir}/IOT_{year}_pxp.zip' #ixi or pxp are practically the same here
    ios = pymrio.parse_exiobase3(in_filename)
    ios.aggregate(sector_agg='total').calc_all()
    A = ios.A.to_numpy()
    ### The two lines above works like the lines commented below (from now on I will use pymrio functions)
    ### Z, Y = ios.Z, ios.Y
    ### #Z_rxr is 49x49 (region x region), it is the sum sectors within each source region, then sum sectors within each destination region
    ### Z_rxr = Z.groupby(level=0).sum().T.groupby(level=0).sum().T
    ### #Y_rxr is 49x7 (region x final demand categories), where each entry is total value of final demand (of each category) for goods produced in region r, summed over: all sectors in region r, and all destination (consuming) regions
    ### Y_rxr = Y.groupby(level=0).sum().T.groupby(level=1).sum().T
    ### x_rxr = pymrio.calc_x(Z_rxr, Y_rxr)
    ### A = pymrio.calc_A(Z_rxr, x_rxr).to_numpy()
    A, indices_reverse_map = build_lop_instance(A)
    out_filename = f'rxr_{year}_n{A.shape[0]}'
    save_lop_matrix(f'{outdir}/{typ}/{out_filename}', A)
    with open(f'{outdir}/revmap/revmap_{out_filename}','w') as f:
        json.dump(indices_reverse_map,f)
    print(f'* {out_filename} DONE.')



#################################
##### cxc instances (8x8) #######
#################################
year = 2022 #last year available
typ = 'cxc'
os.mkdir(f'{outdir}/{typ}')
in_filename = f'{indir}/IOT_{year}_pxp.zip'
ios = pymrio.parse_exiobase3(in_filename)
class_info = pymrio.get_classification('exio3_pxp') #aligned with filename two lines above
ios.rename_sectors( class_info.get_sector_dict(
                        orig = class_info.sectors.ExioName,
                        new  = class_info.sectors.ConsumptionCategories ) )
ios.aggregate_duplicates()
ios.calc_all()
for reg in ios.regions:
    A = ios.A.loc[reg,reg].to_numpy()
    A, indices_reverse_map = build_lop_instance(A)
    out_filename = f'{typ}_{reg.lower()}_{year}_n{A.shape[0]}'
    save_lop_matrix(f'{outdir}/{typ}/{out_filename}', A)
    with open(f'{outdir}/revmap/revmap_{out_filename}','w') as f:
        json.dump(indices_reverse_map,f)
    print(f'* {out_filename} DONE.')



#################################
##### isic instances (12x12) ####
#################################
year = 2022 #last year available
typ = 'isic'
os.mkdir(f'{outdir}/{typ}')
in_filename = f'{indir}/IOT_{year}_ixi.zip' #ISIC classification is only on ixi tables
ios = pymrio.parse_exiobase3(in_filename)
class_info = pymrio.get_classification('exio3_ixi') #aligned with filename two lines above
ios.rename_sectors( class_info.get_sector_dict(
                        orig = class_info.sectors.ExioName,
                        new  = class_info.sectors.ISICCode ) )
ios.aggregate_duplicates()
ios.calc_all()
for reg in ios.regions:
    A = ios.A.loc[reg,reg].to_numpy()
    A, indices_reverse_map = build_lop_instance(A)
    out_filename = f'{typ}_{reg.lower()}_{year}_n{A.shape[0]}'
    save_lop_matrix(f'{outdir}/{typ}/{out_filename}', A)
    with open(f'{outdir}/revmap/revmap_{out_filename}','w') as f:
        json.dump(indices_reverse_map,f)
    print(f'* {out_filename} DONE.')



##############################################
##### oversampled instances (300x300) ########
##############################################
year = 2022 #last year available
size = 300
typ = f'os{size}'
os.mkdir(f'{outdir}/{typ}')
in_filename = f'{indir}/IOT_{year}_pxp.zip' #I oversample pxp instances
ios = pymrio.parse_exiobase3(in_filename)
for reg in ios.regions:
    P = ios.A.loc[reg,reg].to_numpy()
    P, _ = build_lop_instance(P)
    pool = P[ P>0 ] #pool is an array with positive entries of P
    A = np.random.choice(pool, size=(size,size), replace=True)
    A, _ = build_lop_instance(A) #reverse map not required here, because instance are "real-life like" and not real-world
    out_filename = f'{typ}_{reg.lower()}_{year}_n{A.shape[0]}'
    save_lop_matrix(f'{outdir}/{typ}/{out_filename}', A)
    print(f'* {out_filename} DONE.')















