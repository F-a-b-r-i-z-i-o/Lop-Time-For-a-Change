import os
import re
import pymrio
import numpy as np

YEAR_RE = re.compile(r"IOT_(199[5-9]|20(?:0\d|1\d|2[0-2]))_")

def extract_year_from_filename(file_path):
    name = os.path.basename(file_path)
    m = YEAR_RE.search(name)
    return m.group(1) if m else "unknown"

def export_matrices(files, out_dir, matrix_type_label="A"):
    os.makedirs(out_dir, exist_ok=True)

    for file_path in files:
        print(f"--- Start Load Matrix: {file_path} ---")
        exio = pymrio.parse_exiobase3(file_path)
        print("--- Matrix Loaded ---")

        numeric = exio.A.to_numpy(dtype=np.float64, copy=False)

        if numeric.shape[0] != numeric.shape[1]:
            raise ValueError(f"Matrix A is not square: {numeric.shape}")

        n = numeric.shape[0]
        year = extract_year_from_filename(file_path)
        filename = f"matrix_{matrix_type_label}_{year}"
        saved_path = os.path.join(out_dir, filename)

        with open(saved_path, "w", encoding="utf-8") as f:
            f.write(str(n) + "\n")
            np.savetxt(f, numeric, fmt="%.15g", delimiter=" ")

        print(f"Saved: {saved_path} (shape={numeric.shape}), name={filename}")


if __name__ == "__main__":
    
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


    out_dir = "../Calcolate-c/find_c_dataset"
    export_matrices(files=folders_pxp, out_dir=out_dir, matrix_type_label="A")
