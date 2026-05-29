import pandas as pd
import numpy as np
import scipy.stats as ss
import os


def read_lop_instance(filename):
    with open(filename, 'r') as f:
        n = int(f.readline())
        A = np.array([list(map(int, f.readline().split())) for _ in range(n)])
    return A


def calculate_instance_statistics(A):
    #Normalize (because xLOLIB2 is not normalized)
    B = A - np.minimum(A,A.T)
    #Take the meaningful \binom{n}{2} = n*(n-1)/2 values
    n = B.shape[0]
    values = []
    for i in range(n-1):
        for j in range(i+1,n):
            values.append( B[i,j] if B[i,j]>0 else B[j,i] )
    values = np.array(values)
    #Sparsity (percentage of zeros)
    sp = np.sum(values==0) / values.size
    #Variation coefficient (dev.std. / mean)
    vc = values.std() / values.mean()
    #Skewness
    sk = ss.skew(values)
    #Return the statistics of this instance
    return sp,vc,sk


def calculate_set_statistics(As):
    #Calculate median, 10-percentile, 90-percentile of all the instances in As
    sps,vcs,sks = [],[],[]
    for A in As:
        sp,vc,sk = calculate_instance_statistics(A)
        sps.append(sp)
        vcs.append(vc)
        sks.append(sk)
    sps,vcs,sks = np.array(sps),np.array(vcs),np.array(sks)
    return {
            'Sp. Median': f'${np.median(sps):.2f}$',
            'Sp. [Min,Max]':  f'$[{sps.min():.2f}, {sps.max():.2f}]$',
            'VC Median':  f'${np.median(vcs):.2f}$',
            'VC [Min,Max]':   f'$[{vcs.min():.2f}, {vcs.max():.2f}]$',
            'Sk. Median': f'${np.median(sks):.2f}$',
            'Sk. [Min,Max]':  f'$[{sks.min():.2f}, {sks.max():.2f}]$',
        }, sps, vcs, sks


#Directories
exiobase_dir = 'exiobase'
lolib_dir = 'classic_benchmarks'
exiobase_sets = [ 'isic', 'rxr', 'pxp', 'os300' ]
lolib_sets = [ 'IO', 'SGB', 'RandA1', 'RandA2', 'RandB', 'MB', 'xLOLIB', 'xLOLIB2' ] #without Spec because instances are too simple


#Calculate statistics
df = pd.DataFrame()
all_As = { exiobase_dir:[], lolib_dir:[] }
for benchmark in [ exiobase_dir, lolib_dir ]:
    sets = exiobase_sets if benchmark==exiobase_dir else lolib_sets
    for iset in sets:
        dir_name = f'{benchmark}/{iset}'
        As = []
        for filename in os.listdir(dir_name):
            full_filename = os.path.join(dir_name,filename)
            A = read_lop_instance(full_filename)
            As.append(A)
            all_As[benchmark].append(A)
        stats,_,__,___ = calculate_set_statistics(As)
        stats['Suite'] = r'\texttt{EXIOBASE}' if benchmark==exiobase_dir else r'\texttt{LOLIB}'
        stats['Set'] = iset
        df = pd.concat([df, pd.DataFrame([stats])], ignore_index=True)


#Reorder columns
df = df[ [ 'Suite',
           'Set',
           'Sp. Median',
           'Sp. [Min,Max]',
           'VC Median',
           'VC [Min,Max]',
           'Sk. Median',
           'Sk. [Min,Max]' ]
        ]
#Add \texttt to sets
df['Set'] = df['Set'].apply(lambda s: f'\\texttt{{{s}}}')
#Set index (for multirow)
df.set_index(['Suite','Set'], inplace=True)


#Create Latex
latex_filename = 'tables/tab_instance_stats.tex'
caption = r'Structural statistics for the \texttt{EXIOBASE} and \texttt{LOLIB} benchmark suites.'
df.to_latex( latex_filename,
             #index         = False,
             caption       = caption,
             #float_format  = '%.2f',
             header        = True,
             multirow      = True,
             column_format = 'llcccccc',
             label         = 'tab:bench_stats',
            )


#Adjust Latex
with open(latex_filename, 'r') as f:
    latex = f.read()
latex = latex.replace(
                r'\begin{table}',
                r'\begin{table}[t]' + '\n' + r'\setlength{\tabcolsep}{0.2cm}' + '\n' + r'\centering'
            ).replace(
                r'\begin{tabular}',
                r'\resizebox{\textwidth}{!}{' + '\n' + r'\begin{tabular}'
            ).replace(
                r'\end{tabular}',
                r'\end{tabular}' + '\n' + r'}'
            ).replace(
                r' &  & Sp. Median & Sp. [Min,Max] & VC Median & VC [Min,Max] & Sk. Median & Sk. [Min,Max] \\',
                r' &  & \multicolumn{2}{c}{\textbf{Sparsity}} & \multicolumn{2}{c}{\textbf{Variation Coef.}} & \multicolumn{2}{c}{\textbf{Skewness}} \\'
            ).replace(
                r'Suite & Set &  &  &  &  &  &  \\',
                r'\textbf{Suite} & \textbf{Set} & Median & [Min,Max] & Median & [Min,Max] & Median & [Min,Max] \\'
            )
with open(latex_filename, 'w') as f:
    f.write(latex)


#Calculate and add overall statistics
exiobase_stats,exiobase_sps,exiobase_vcs,exiobase_sks = calculate_set_statistics(all_As[exiobase_dir])
lolib_stats,lolib_sps,lolib_vcs,lolib_sks = calculate_set_statistics(all_As[lolib_dir])
with open(latex_filename, 'r') as f:
    latex = f.read()
latex = latex.replace(
                r'\cline{1-8}' + '\n' + r'\multirow',
                r'\cline{2-8}' +
                    '\n' +
                    f' & \\textbf{{Overall}} & {exiobase_stats["Sp. Median"]} & {exiobase_stats["Sp. [Min,Max]"]} & {exiobase_stats["VC Median"]} & {exiobase_stats["VC [Min,Max]"]} & {exiobase_stats["Sk. Median"]} & {exiobase_stats["Sk. [Min,Max]"]} \\\\' +
                    '\n' +
                    r'\cline{1-8}' + '\n' + r'\multirow'
            ).replace(
                r'\cline{1-8}' + '\n' + r'\bottomrule',
                r'\cline{2-8}' +
                    '\n' +
                    f' & \\textbf{{Overall}} & {lolib_stats["Sp. Median"]} & {lolib_stats["Sp. [Min,Max]"]} & {lolib_stats["VC Median"]} & {lolib_stats["VC [Min,Max]"]} & {lolib_stats["Sk. Median"]} & {lolib_stats["Sk. [Min,Max]"]} \\\\' +
                    '\n' +
                    r'\bottomrule'
            )
with open(latex_filename, 'w') as f:
    f.write(latex)


#Print Mann Whitney U of exiobase vs. lolib
print(f'MannWhitneyU on Sp. = {ss.mannwhitneyu(exiobase_sps,lolib_sps)}')
print(f'MannWhitneyU on VC  = {ss.mannwhitneyu(exiobase_vcs,lolib_vcs)}')
print(f'MannWhitneyU on Sk. = {ss.mannwhitneyu(exiobase_sks,lolib_sks)}')
