import os
import glob
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed

MAEDM_BIN = "./MAEDM"
CDRVNS_BIN = "./CDRVNS"
DIR = "exiobase/pxp/"
N_RUNS = 30
ARCHIVE_SIZES = [5, 10, 15]   
OUTFILE = "results"
RES_FOLDER = "results"

def run_cmd(algo: str, file_path: str, run: int, seed: int, m: int, cmd: list[str]) -> int:
    print(f"Processing: {algo} file={file_path} run={run} seed={seed} m={m}", flush=True)
    completed = subprocess.run(cmd, check=True)
    return completed.returncode

def main() -> None:
    os.makedirs(RES_FOLDER, exist_ok=True)
    files = [f for f in glob.glob(os.path.join(DIR, "*")) if os.path.isfile(f)]

    max_workers = os.cpu_count()
    print(f"Using {max_workers} workers", flush=True)

    out_cdrvns = os.path.join(RES_FOLDER, f"{OUTFILE}_MS_CDRVNS.csv")
    out_maedm  = os.path.join(RES_FOLDER, f"{OUTFILE}_MS_MAEDM.csv")

    futures = []
    with ThreadPoolExecutor(max_workers=max_workers) as pool:
        for file_path in files:
            for m in ARCHIVE_SIZES:                 
                for run in range(1, N_RUNS + 1):
                    for seed in range(1, N_RUNS + 1):
                        cmd_cdrvns = [
                            CDRVNS_BIN, "-i", file_path, "-o", out_cdrvns, "-s", str(seed), "-m", str(m)
                        ]
                        cmd_maedm = [
                            MAEDM_BIN, "-i", file_path, "-o", out_maedm, "-s", str(seed), "-m", str(m)
                        ]
                        futures.append(pool.submit(run_cmd, "CDRVNS", file_path, run, seed, m, cmd_cdrvns))
                        futures.append(pool.submit(run_cmd, "MAEDM",  file_path, run, seed, m, cmd_maedm))

        for fut in as_completed(futures):
            try:
                fut.result()
            except subprocess.CalledProcessError as e:
                print(f"ERROR command failed (code {e.returncode}): {e.cmd}", flush=True)

if __name__ == "__main__":
    main()
