#!/usr/local/bin/python3

# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
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

    parser.add_argument(
        "--thread_id", metavar="T", nargs="?", help="Show only messages on this thread"
    )

    parser.add_argument(
        "--rolled",
        action="store_true",
        help="(optionally) treat file as first in a sequence of rolled log files, and scan all sequential files.",
    )
    args = parser.parse_args()

    if args.file is None:
        print("ERROR: Please provide a '--file' argument")
        parser.print_help()
        sys.exit(1)

    return (args.file, args.handshake, args.messages, args.thread_id, args.rolled)
