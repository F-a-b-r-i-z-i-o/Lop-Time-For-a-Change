import csv
input_file = "results_rxr/results_MS_CDRVNS.csv"
output_file = "results_MS_CDRVNS.csv"

with open(input_file, newline="", encoding="utf-8") as f:
    reader = csv.reader(f, delimiter=";")
    rows = list(reader)

    # sort by seed
    rows.sort(key=lambda row: int(row[0]))

with open(output_file, "w", newline="", encoding="utf-8") as f:
    writer = csv.writer(f, delimiter=";")
    writer.writerows(rows)

print("File order and save in:", output_file)

