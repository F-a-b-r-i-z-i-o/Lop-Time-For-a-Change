import pymrio
import pandas as pd
import numpy as np
import contextlib
import gc

class LoadInstance:
    def __init__(self, file_path: str) -> None:
        self.file_path = file_path
        self.file_max_abs_err = 0.0
        self.file_max_info = None

    def _load_matrix(self):
        return pymrio.parse_exiobase3(self.file_path)

    def check_all_countries_A_equals_Z_diag_x_exact(
        self,
        rtol: float = 1e-9,
        atol: float = 1e-12,
        topk_cells: int = 10,
    ) -> None:

        """
        Verify for country A == Z @ diag(1/x) and print mismatch
        + Track max absolute error over ALL checked blocks
        """
        matrix = pymrio.parse_exiobase3(self.file_path)
        try:
            A_ref = matrix.A

            # --- Z ---
            Z = matrix.Z

            # --- x ---
            x = matrix.x
            x_np = np.asarray(x, dtype=np.float64).reshape(-1)

            assert Z.shape[1] == x_np.shape[0], (
                f"Dimension mismatch: Z ha {Z.shape[1]} colonne, x ha lunghezza {x_np.shape[0]} "
                f"(serve x globale allineato alle colonne di Z)."
            )

            # recix = 1/x con x=0 -> 0
            recix = np.zeros_like(x_np)
            mask = x_np != 0
            recix[mask] = 1.0 / x_np[mask]

            # calculate A_calc on full matrix maintaining label of Z
            Z_np = Z.to_numpy(dtype=np.float64)
            A_calc_np = Z_np @ np.diag(recix)

            A_calc = pd.DataFrame(A_calc_np, index=Z.index, columns=Z.columns)

            regions = sorted(set(A_ref.index.get_level_values(0)))

            print(f"\n== Check A == Z @ diag(1/x) for file: {self.file_path} ==")
            print(f"rtol={rtol} atol={atol} topk_cells={topk_cells} method={'matmul_diag'}")
            print(f"regions={len(regions)}")

            bad_regions = []

            # TRACK MAX ABS ERROR PER FILE 
            file_max_abs_err = 0.0
            file_max_info = None

            for region in regions:
                idx_r = A_ref.index[A_ref.index.get_level_values(0) == region]
                if len(idx_r) == 0:
                    continue

                # block reference
                Ar_ref = A_ref.loc[idx_r, idx_r]

                # block calculate
                Ar_calc = A_calc.reindex(index=Ar_ref.index, columns=Ar_ref.columns)

                # shape check
                if Ar_calc.shape != Ar_ref.shape:
                    print(f"[{region}] SHAPE MISMATCH: calc={Ar_calc.shape} ref={Ar_ref.shape}")
                    bad_regions.append(region)
                    continue

        
                ref_np = Ar_ref.to_numpy(dtype=np.float64)
                cal_np = Ar_calc.to_numpy(dtype=np.float64)

                diff_np = np.abs(cal_np - ref_np)

                              # ABS MAX for block 
                if np.all(np.isnan(diff_np)):
                    region_max = 0.0
                    i = j = 0
                else:
                    region_max = float(np.nanmax(diff_np))
                    flat_i = int(np.nanargmax(diff_np))
                    i, j = np.unravel_index(flat_i, diff_np.shape)

                # Update max for file
                if region_max > file_max_abs_err:
                    file_max_abs_err = region_max
                    row_key = Ar_ref.index[i]
                    col_key = Ar_ref.columns[j]
                    file_max_info = {
                        "file": self.file_path,
                        "region": region,
                        "row": row_key,
                        "col": col_key,
                        "ref": float(ref_np[i, j]) if not np.isnan(ref_np[i, j]) else float("nan"),
                        "calc": float(cal_np[i, j]) if not np.isnan(cal_np[i, j]) else float("nan"),
                    }

                ok = np.allclose(
                    cal_np,
                    ref_np,
                    rtol=rtol,
                    atol=atol,
                    equal_nan=True,
                )

                if ok:
                    print(f"[{region}] OK")
                    continue

                bad_regions.append(region)

                # stampa mismatch e top-k
                row_key = Ar_ref.index[i]
                col_key = Ar_ref.columns[j]
                max_val = float(diff_np[i, j]) if not np.isnan(diff_np[i, j]) else float("nan")
                print(f"\n[{region}] MISMATCH: max diff={max_val:.6e} at row={row_key} col={col_key}")

                flat = diff_np.ravel()
                flat2 = np.where(np.isnan(flat), -np.inf, flat)
                top_idx = np.argsort(flat2)[-topk_cells:][::-1]

                print(f"[{region}] Top {topk_cells} mismatches:")
                for k in top_idx:
                    if flat2[k] == -np.inf:
                        continue
                    ii, jj = np.unravel_index(int(k), diff_np.shape)
                    dval = float(diff_np[ii, jj])
                    r_i = Ar_ref.index[ii]
                    c_j = Ar_ref.columns[jj]
                    v_ref = ref_np[ii, jj]
                    v_cal = cal_np[ii, jj]
                    print(f"  {r_i} -> {c_j} | ref={v_ref:.18e} calc={v_cal:.18e} diff={dval:.6e}")

            print(f"\nDone. Bad regions: {bad_regions}")

            self.file_max_abs_err = file_max_abs_err
            self.file_max_info = file_max_info

            if file_max_info is None:
                print(f"[FILE MAX ABS ERROR] {self.file_path}: 0.0")
            else:
                print(
                    f"[FILE MAX ABS ERROR] {self.file_path}: {file_max_abs_err:.6e} "
                    f"(region={file_max_info['region']}, row={file_max_info['row']}, col={file_max_info['col']}, "
                    f"ref={file_max_info['ref']}, calc={file_max_info['calc']})"
                )

            assert not bad_regions, f"Mismatch in regions: {bad_regions}"

        finally:
            del matrix
            gc.collect()


def compare_across_folders(folders, topk_cells=10):
    failures = []

    # ABS MAX for list (pxp or ixi)
    group_max_abs_err = 0.0
    group_max_info = None

    for f in folders:
        print(f"\n--- Loading+Check: {f} ---")
        inst = LoadInstance(file_path=f)

        try:
            inst.check_all_countries_A_equals_Z_diag_x_exact(topk_cells=topk_cells)
            if hasattr(inst, "file_max_abs_err") and inst.file_max_abs_err > group_max_abs_err:
                group_max_abs_err = inst.file_max_abs_err
                group_max_info = inst.file_max_info

            print(f"--- DONE OK: {f} ---")

        except AssertionError as e:
            if hasattr(inst, "file_max_abs_err") and inst.file_max_abs_err > group_max_abs_err:
                group_max_abs_err = inst.file_max_abs_err
                group_max_info = inst.file_max_info

            print(f"--- DONE FAIL: {f} ---")
            print(f"AssertionError: {e}")
            failures.append((f, str(e)))

        except Exception as e:
            print(f"--- DONE ERROR: {f} ---")
            print(f"{type(e).__name__}: {e}")
            failures.append((f, f"{type(e).__name__}: {e}"))

        finally:
            del inst
            gc.collect()

    print("\n=== SUMMARY ===")
    if not failures:
        print("ALL dataset OK.")
    else:
        for f, msg in failures:
            print(f"- {f}: {msg}")

    if group_max_info is None:
        print("\n[GROUP MAX ABS ERROR] 0.0")
    else:
        print(
            f"\n[GROUP MAX ABS ERROR] {group_max_abs_err:.6e} "
            f"(file={group_max_info['file']}, region={group_max_info['region']}, "
            f"row={group_max_info['row']}, col={group_max_info['col']}, "
            f"ref={group_max_info['ref']}, calc={group_max_info['calc']})"
        )

    return failures, group_max_abs_err, group_max_info


if __name__ == "__main__":

    folders_ixi = [
        "../Compact-data/IOT_1995_ixi.zip",
        "../Compact-data/IOT_1996_ixi.zip",
        "../Compact-data/IOT_1997_ixi.zip",
        "../Compact-data/IOT_1998_ixi.zip",
        "../Compact-data/IOT_1999_ixi.zip",
        "../Compact-data/IOT_2000_ixi.zip",
        "../Compact-data/IOT_2001_ixi.zip",
        "../Compact-data/IOT_2002_ixi.zip",
        "../Compact-data/IOT_2003_ixi.zip",
        "../Compact-data/IOT_2004_ixi.zip",
        "../Compact-data/IOT_2005_ixi.zip",
        "../Compact-data/IOT_2006_ixi.zip",
        "../Compact-data/IOT_2007_ixi.zip",
        "../Compact-data/IOT_2008_ixi.zip",
        "../Compact-data/IOT_2009_ixi.zip",
        "../Compact-data/IOT_2010_ixi.zip",
        "../Compact-data/IOT_2011_ixi.zip",
        "../Compact-data/IOT_2012_ixi.zip",
        "../Compact-data/IOT_2013_ixi.zip",
        "../Compact-data/IOT_2014_ixi.zip",
        "../Compact-data/IOT_2015_ixi.zip",
        "../Compact-data/IOT_2016_ixi.zip",
        "../Compact-data/IOT_2017_ixi.zip",
        "../Compact-data/IOT_2018_ixi.zip",
        "../Compact-data/IOT_2019_ixi.zip",
        "../Compact-data/IOT_2020_ixi.zip",
        "../Compact-data/IOT_2021_ixi.zip",
        "../Compact-data/IOT_2022_ixi.zip",
    ]

    folders_pxp = [
        "../Compact-data/IOT_1995_pxp.zip",
        "../Compact-data/IOT_1996_pxp.zip",
        "../Compact-data/IOT_1997_pxp.zip",
        "../Compact-data/IOT_1998_pxp.zip",
        "../Compact-data/IOT_1999_pxp.zip",
        "../Compact-data/IOT_2000_pxp.zip",
        "../Compact-data/IOT_2001_pxp.zip",
        "../Compact-data/IOT_2002_pxp.zip",
        "../Compact-data/IOT_2003_pxp.zip",
        "../Compact-data/IOT_2004_pxp.zip",
        "../Compact-data/IOT_2005_pxp.zip",
        "../Compact-data/IOT_2006_pxp.zip",
        "../Compact-data/IOT_2007_pxp.zip",
        "../Compact-data/IOT_2008_pxp.zip",
        "../Compact-data/IOT_2009_pxp.zip",
        "../Compact-data/IOT_2010_pxp.zip",
        "../Compact-data/IOT_2011_pxp.zip",
        "../Compact-data/IOT_2012_pxp.zip",
        "../Compact-data/IOT_2013_pxp.zip",
        "../Compact-data/IOT_2014_pxp.zip",
        "../Compact-data/IOT_2015_pxp.zip",
        "../Compact-data/IOT_2016_pxp.zip",
        "../Compact-data/IOT_2017_pxp.zip",
        "../Compact-data/IOT_2018_pxp.zip",
        "../Compact-data/IOT_2019_pxp.zip",
        "../Compact-data/IOT_2020_pxp.zip",
        "../Compact-data/IOT_2021_pxp.zip",
        "../Compact-data/IOT_2022_pxp.zip",
    ]

    log_file = "test_A_by_country.log"
    with open(log_file, "w", encoding="utf-8") as f, contextlib.redirect_stdout(f):
        fail_ixi, max_ixi, info_ixi = compare_across_folders(folders_ixi, topk_cells=10)
        fail_pxp, max_pxp, info_pxp = compare_across_folders(folders_pxp, topk_cells=10)
        
        if max_ixi >= max_pxp:
            overall_max = max_ixi
            overall_info = info_ixi
        else:
            overall_max = max_pxp
            overall_info = info_pxp

        print("\n=== OVERALL MAX ABS ERROR (ALL FILES IN LOG) ===")
        if overall_info is None:
            print("[OVERALL MAX ABS ERROR] 0.0")
        else:
            print(
                f"[OVERALL MAX ABS ERROR] {overall_max:.6e} "
                f"(file={overall_info['file']}, region={overall_info['region']}, "
                f"row={overall_info['row']}, col={overall_info['col']}, "
                f"ref={overall_info['ref']}, calc={overall_info['calc']})"
            )

    print(f"Log write in: {log_file}")
