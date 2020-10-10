import subprocess
import argparse
from tabulate import tabulate
import itertools
from types import SimpleNamespace

def main():
    parser = argparse.ArgumentParser(description="Run test")

    parser.add_argument("-p", "--program", type=str, required=True, metavar="EXECUTABLE", help="Program/executable to run")
    parser.add_argument("-a", "--arguments", type=str, metavar="ARGS", help="Arguments passed to program")
    parser.add_argument("-d", "--detailed", action="store_true", help="Show detailed info")
    parser.add_argument("-v", "--verbose", action="store_true", help="Toggle verbosity")

    explicit_group = parser.add_argument_group(title="Explicit", description="Manually specify an input file and a comparison file")
    explicit_group.add_argument("-i", "--input-file", type=str, metavar="INPUT_FILE", help="File containing input")
    explicit_group.add_argument("-vs", "--compare-file", type=str, metavar="COMPARE_FILE", help="File containing expected output")

    auto_group = parser.add_argument_group(title="Auto", description="Run tests from test file")
    auto_group.add_argument("-t", "--test-file", type=str, metavar="TEST_FILE", help="File containing test description")
    auto_group.add_argument("-n", "--test-name", type=str, metavar="TEST_NAME", help="Run specific test")

    args = parser.parse_args()

    test_option = SimpleNamespace()
    test_option.detailed = args.detailed
    test_option.verbose = args.verbose

    test_cases = []
    if args.test_file is not None:
        for test_case in get_tests(args.test_file, args.test_name):
            run_test(args.program, args.arguments, test_case, test_option)
    else:
        if args.input_file and args.compare_file:
            run_test(args.program, args.arguments, (None, args.input_file, args.compare_file), test_option)
        else:
            print("An input file and a compare file are required")



def get_tests(test_file_path, test_name=None):
    test_cases = []

    with open(test_file_path) as f:

        if test_name is None:
            for line in f.readlines():
                test_case = line.strip().split(":")
                test_cases.append(test_case)
        else:
            for line in f.readlines():
                test_case = line.strip().split(":")
                if test_case[0] == test_name:
                    test_cases.append(test_case)
                    break

    return test_cases


def run_test(program_path, arguments, test_case, test_option):
    
    [test_name, test_input, test_compare] = test_case
    
    # Default test name is Unknown if not specified
    if test_name is None:
        test_name = "Unknown"

    # Prepare arguments passed to program
    full_args = f"{test_input} {arguments}" if arguments else test_input

    process = subprocess.Popen([program_path, full_args],
                                stdout=subprocess.PIPE,
                                universal_newlines=True)

    if test_option.verbose:
        print(f"Executing {process.args}...")

    results = [line.strip() for line in process.stdout.readlines()]

    with open(test_compare) as f:
        expected_result = [line.strip() for line in f.readlines()]

        status = "\u2705 Pass" if results == expected_result else "\u274C Failed"

        # Print test summary
        print(f"Test {test_name}: {status}")

        # Optionally print detailed information
        if test_option.detailed:
            table = [(r1, r2, r1 == r2) for (r1, r2) in itertools.zip_longest(results, expected_result)]

            # Print test result as a table
            print(tabulate(table, 
                           headers=["No.", "Program output", "Ref file", "Match"],
                           showindex=True))
        
            # End with a newline
            print()

main()