import os
import re
import glob
import argparse
from pdfminer.high_level import extract_text

# χ²/n pattern (handles: χ 2 /n = 0.50, χ2/n=0.50, chi^2/n = 0.50 etc.)
CHI2_PATTERN = re.compile(
    r'(χ\s*2\s*/\s*n|chi\s*\^?2\s*/\s*n)\s*=\s*([0-9]*\.?[0-9]+)',
    re.IGNORECASE
)

# ------------------------------------
# Extract ALL chi2 values from a PDF
# ------------------------------------
def extract_chi2_from_pdf(pdf_path):
    try:
        text = extract_text(pdf_path)
        matches = CHI2_PATTERN.findall(text)

        if matches:
            return [m[1] for m in matches]  # list of values
        else:
            return []

    except Exception as e:
        print(f"[ERROR] reading {pdf_path}: {e}")
        return []


# ------------------------------------
# Collect PDF inputs
# ------------------------------------
def collect_input_pdfs(user_input):
    if os.path.isfile(user_input) and user_input.endswith(".pdf"):
        return [user_input]
    elif os.path.isdir(user_input):
        return sorted(glob.glob(os.path.join(user_input, "*.pdf")))
    else:
        return sorted(glob.glob(user_input))


# ------------------------------------
# MAIN
# ------------------------------------
def main():

    parser = argparse.ArgumentParser(
        description="Extract chi2 values from PDF plots"
    )

    parser.add_argument(
        "--input",
        required=True,
        help="Input PDF file, directory, or wildcard pattern"
    )

    parser.add_argument(
        "--output",
        default=None,
        help="Output directory name (optional). If not given, auto-generated."
    )

    args = parser.parse_args()

    input_arg = args.input
    pdf_list = collect_input_pdfs(input_arg)

    if len(pdf_list) == 0:
        print("No PDF files found!")
        return

    print(f"\nFound {len(pdf_list)} PDF file(s)\n")

    results = []
    values_only_lines = []

    # -------- process PDFs --------
    for pdf in pdf_list:
        chi2_list = extract_chi2_from_pdf(pdf)

        if len(chi2_list) > 0:
            values_str = " ".join(chi2_list)
            line = f"{os.path.basename(pdf)}   {values_str}"
            values_only_lines.append(values_str)
        else:
            line = f"{os.path.basename(pdf)}   NOT_FOUND"
            values_only_lines.append("")

        print(line)
        results.append(line)

    # -------- determine chi2 count --------
    chi2_per_pdf = 0
    for v in values_only_lines:
        if v.strip() != "":
            chi2_per_pdf = len(v.split())
            break

    print(f"\nEach PDF contains {chi2_per_pdf} chi-squared value(s)\n")

    # -------- define output directory --------
    if args.output is not None:
        out_dir = args.output
    else:
        if os.path.isfile(input_arg):
            base = os.path.splitext(os.path.basename(input_arg))[0]
        elif os.path.isdir(input_arg):
            base = os.path.basename(os.path.normpath(input_arg))
        else:
            base = "pdf_collection"

        out_dir = f"{base}_output"

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    # -------- output files --------
    histo_file = os.path.join(out_dir, "chi2_histo_values.txt")
    values_file = os.path.join(out_dir, "chi2_values.txt")

    nfiles = len(pdf_list)

    # -------- write histo file --------
    with open(histo_file, "w") as f:
        f.write(f"Found {nfiles} PDF file(s)\n")
        f.write(f"Found {chi2_per_pdf} chi-squared values per plot\n")
        for line in results:
            f.write(line + "\n")

    print(f"Saved name+χ² → {histo_file}")

    # -------- write values-only file --------
    with open(values_file, "w") as f:
        f.write(f"{nfiles}\n")
        f.write(f"{chi2_per_pdf}\n")
        for line in values_only_lines:
            f.write(line + "\n")

    print(f"Saved χ² only → {values_file}\n")


# ------------------------------------
if __name__ == "__main__":
    main()
