import subprocess
import argparse
from tabulate import tabulate
import itertools

def main():
    parser = argparse.ArgumentParser(description='Run test')

    parser.add_argument("program", type=str, help="Program to run")
    parser.add_argument("-a", "--arguments", type=str, help="Arguments passed to program")
    parser.add_argument("-vs", "--compare", type=str, help="Output file to compare")

    args = parser.parse_args()

    process = subprocess.Popen([args.program, args.arguments],
                                stdout=subprocess.PIPE,
                                universal_newlines=True)

    results = [line.strip() for line in process.stdout.readlines()]

    with open(args.compare) as f:
        compare_result = [line.strip() for line in f.readlines()]

        table = [(i, r1, r2, r1 == r2) for i, (r1, r2) in enumerate(itertools.zip_longest(results, compare_result))]
        print(tabulate(table, headers=["No.", "Program output", "Ref file", "Match"]))


main()