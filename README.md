# Linear Ordering Problem: Time for a Change

This repository accompanies the [PPSN 2026 (BEST PAPER AWARD)](https://arxiv.org/abs/2605.31051) by Fabrizio Fagiolo, Marco Baioletti, and Valentino Santucci. The paper introduces a benchmark suite for the Linear Ordering Problem (LOP) built from EXIOBASE economic input–output tables. It also studies how to find several high-quality orderings that differ from one another.

## Repository contents

| Resource | What it contains |
| --- | --- |
| EXIOBASE benchmark suite | LOP instances derived from economic data spanning 1995–2022. Browse the [instance directory](./exiobase/) or get the complete suite as a [ZIP archive](./exiobase.zip). |
| Supplementary material | A PDF with additional material for the paper. |
| Reproducibility code | Programs and scripts used to run the experiments reported in the paper. |

## Solver credits

We used the original CD-RVNS and MA-EDM implementations in our experiments. If you use either solver in your research, please cite the corresponding paper.

| Solver | Source code | Paper to cite |
| --- | --- | --- |
| **CD-RVNS** | [sgpceurj/Precedences_LOP](https://github.com/sgpceurj/Precedences_LOP) | [Using pairwise precedences for solving the linear ordering problem](https://www.sciencedirect.com/science/article/pii/S1568494619307793) |
| **MA-EDM** | [carlossegurag/LOP_MA-EDM](https://github.com/carlossegurag/LOP_MA-EDM) | [A diversity-aware memetic algorithm for the linear ordering problem](https://doi.org/10.1007/s12293-022-00378-5) |

## Citation

If you use the EXIOBASE benchmark suite or build on these experiments, please cite the conference paper.

> Fabrizio Fagiolo, Marco Baioletti, and Valentino Santucci (2026). “Linear Ordering Problem: Time for a Change.” In *Parallel Problem Solving from Nature – PPSN XIX*, Part I, Lecture Notes in Computer Science, vol. 16985, pp. 399–416. Springer. [Open-access version on arXiv](https://arxiv.org/abs/2605.31051).

```bibtex
@inproceedings{fagiolo2026linear,
  author    = {Fagiolo, Fabrizio and Baioletti, Marco and Santucci, Valentino},
  title     = {{Linear Ordering Problem: Time for a Change}},
  booktitle = {Parallel Problem Solving from Nature -- {PPSN XIX}, Part I},
  series    = {Lecture Notes in Computer Science},
  volume    = {16985},
  pages     = {399--416},
  publisher = {Springer},
  year      = {2026},
  eprint    = {2605.31051},
  archivePrefix = {arXiv}
}
```