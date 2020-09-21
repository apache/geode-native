import argparse
import sys

parser = None


def parse_command_line():
    global parser
    parser = argparse.ArgumentParser(
        description="Parse a Gemfire NativeClient log file."
    )
    parser.add_argument("--file", metavar="F", nargs="?", help="Data file path/name")

    parser.add_argument(
        "--handshake",
        action="store_true",
        help="(optionally) print out handshake message details",
    )
    parser.add_argument(
        "--messages",
        action="store_true",
        help="(optionally) print out regular message details",
    )

    args = parser.parse_args()

    if args.file is None:
        print("ERROR: Please provide a '--file' argument")
        parser.print_help()
        sys.exit(1)

    return (args.file, args.handshake, args.messages)
