import os
import glob
import csv
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed

MAEDM_BIN = "./MAEDM"
CDRVNS_BIN = "./CDRVNS"

DIR = "exiobase"

N_SEEDS = 30
ARCHIVE_SIZES = [5, 10, 15]

RES_FOLDER = "results"
TMP_FOLDER = os.path.join(RES_FOLDER, "tmp")
FINAL_CSV = os.path.join(RES_FOLDER, "results.csv")


def iter_all_files(root_dir: str) -> list[str]:
    """Return all regular files under root_dir recursively."""
    paths = glob.glob(os.path.join(root_dir, "**", "*"), recursive=True)
    return sorted([p for p in paths if os.path.isfile(p)])


def tmp_output_path(algo: str, file_path: str, seed: int, m: int) -> str:
    """
    Build a unique tmp CSV name for each run.
    This avoids multiple parallel processes writing into the same CSV file.
    """
    rel = os.path.relpath(file_path, DIR).replace(os.sep, "__").replace(" ", "_")
    return os.path.join(TMP_FOLDER, f"{algo}__{rel}__seed{seed}__m{m}.csv")


def append_csv(tmp_files: list[str], final_csv: str) -> None:
    """
    Append tmp CSVs into final_csv, keeping only ONE header row.
    - First written file defines the header.
    - Subsequent files: if first row equals the header, it's skipped.
    """
    final_exists = os.path.isfile(final_csv) and os.path.getsize(final_csv) > 0
    header = None

    if final_exists:
        with open(final_csv, "r", newline="", encoding="utf-8") as f:
            r = csv.reader(f)
            try:
                header = next(r)
            except StopIteration:
                header = None

    with open(final_csv, "a", newline="", encoding="utf-8") as fout:
        w = csv.writer(fout)

        for path in tmp_files:
            if not (os.path.isfile(path) and os.path.getsize(path) > 0):
                continue

            with open(path, "r", newline="", encoding="utf-8") as fin:
                r = csv.reader(fin)
                try:
                    first = next(r)
                except StopIteration:
                    continue

                if not final_exists:
                    # First append ever: write header
                    w.writerow(first)
                    header = first
                    final_exists = True
                else:
                    # Skip duplicated header if present
                    if header is None or first != header:
                        w.writerow(first)

                # Write remaining rows
                for row in r:
                    if row:
                        w.writerow(row)


def run_cmd(algo: str, file_path: str, seed: int, m: int, out_tmp: str) -> int:
    """Run one algorithm; write output to a unique tmp CSV file."""
    print(f"Processing: {algo} file={file_path} seed={seed} m={m}", flush=True)

    bin_path = CDRVNS_BIN if algo == "CDRVNS" else MAEDM_BIN
    cmd = [bin_path, "-i", file_path, "-o", out_tmp, "-s", str(seed), "-m", str(m)]

    completed = subprocess.run(cmd, check=True)
    return completed.returncode


def main() -> None:
    os.makedirs(RES_FOLDER, exist_ok=True)
    os.makedirs(TMP_FOLDER, exist_ok=True)

    files = iter_all_files(DIR)

    max_workers = os.cpu_count() 
    print(f"Found {len(files)} files under '{DIR}'", flush=True)
    print(f"Using {max_workers} workers", flush=True)

    # Process one input file at a time
    # but run all seeds/m in parallel for that file.
    for file_path in files:
        futures = []
        tmp_outputs = []

        with ThreadPoolExecutor(max_workers=max_workers) as pool:
            for m in ARCHIVE_SIZES:
                for seed in range(1, N_SEEDS + 1):
                    out_cdrvns = tmp_output_path("CDRVNS", file_path, seed, m)
                    out_maedm = tmp_output_path("MAEDM", file_path, seed, m)

                    tmp_outputs.extend([out_cdrvns, out_maedm])

                    futures.append(pool.submit(run_cmd, "CDRVNS", file_path, seed, m, out_cdrvns))
                    futures.append(pool.submit(run_cmd, "MAEDM", file_path, seed, m, out_maedm))

            for fut in as_completed(futures):
                try:
                    fut.result()
                except subprocess.CalledProcessError as e:
                    print(f"ERROR command failed (code {e.returncode}): {e.cmd}", flush=True)

        # After finishing this input file, append its tmp outputs to the final CSV
        append_csv(sorted(set(tmp_outputs)), FINAL_CSV)
        print(f"Updated final CSV after: {file_path}", flush=True)

        # delete tmp files for this input to save disk space
    for p in set(tmp_outputs):
        try:
            os.remove(p)
        except OSError:
            pass

    print(f"Done. Final CSV: {FINAL_CSV}", flush=True)


if __name__ == "__main__":
    main()
