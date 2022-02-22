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
import json
import queue
import os
import sys
import traceback


import command_line

from client_message_decoder import ClientMessageDecoder
from server_message_decoder import ServerMessageDecoder
from handshake_decoder import HandshakeDecoder


def scan_opened_file(
    file,
    handshake_decoder,
    client_decoder,
    server_decoder,
    output_queue,
    dump_handshake,
    dump_messages,
    thread_id,
    start_string,
):
    separator = start_string
    if dump_handshake:
        handshake_decoder = HandshakeDecoder(output_queue)
        for line in file:
            handshake_decoder.process_line(line.decode("utf-8").rstrip())
            try:
                data = output_queue.get_nowait()
                for key, value in data.items():
                    if key == "handshake":
                        print(separator + json.dumps(value, indent=2, default=str))
                        separator = ","
            except queue.Empty:
                continue

    separator = start_string
    for line in file:
        linestr = line.decode("utf-8").rstrip()
        client_decoder.process_line(linestr)
        server_decoder.process_line(linestr)
        try:
            data = output_queue.get_nowait()
            for key, value in data.items():
                if key == "message" and dump_messages:
                    if thread_id:
                        if "tid" in value.keys() and value["tid"] == thread_id:
                            print(separator + json.dumps(value, indent=2, default=str))
                            separator = ","
                    else:
                        print(separator + json.dumps(value, indent=2, default=str))
                        separator = ","

        except queue.Empty:
            continue
        except:
            traceback.print_exc()
            continue

    while True:
        try:
            data = output_queue.get_nowait()
            for key, value in data.items():
                if key == "message" and dump_messages:
                    print(separator + json.dumps(data, indent=2, default=str))
                    separator = ","
        except queue.Empty:
            break


def scan_file(filename, dump_handshake, dump_messages, thread_id):
    print("[")

    output_queue = queue.Queue()

    f = open(filename, "rb")
    print("Scanning " + filename, file=sys.stderr)
    scan_opened_file(
        f,
        HandshakeDecoder(output_queue),
        ClientMessageDecoder(output_queue),
        ServerMessageDecoder(output_queue),
        output_queue,
        dump_handshake,
        dump_messages,
        thread_id,
        "",
    )
    f.close()

    print("]")


def scan_file_sequence(file, handshake, messages, thread_id):
    dirname = os.path.dirname(file)
    basename, ext = os.path.splitext(os.path.basename(file))

    base_parts = basename.split("-")
    if len(base_parts) == 2:
        print("[")

        root = base_parts[0]
        roll_index = int(base_parts[1])

        output_queue = queue.Queue()
        handshake_decoder = HandshakeDecoder(output_queue)
        client_decoder = ClientMessageDecoder(output_queue)
        server_decoder = ServerMessageDecoder(output_queue)
        start_string = ""
        last_chance = False

        while True:
            if last_chance:
                if len(dirname) > 0:
                    filename = dirname + os.sep + root + ext
                else:
                    filename = root + ext
            else:
                if len(dirname) > 0:
                    filename = dirname + os.sep + root + "-" + str(roll_index) + ext
                else:
                    filename = root + "-" + str(roll_index) + ext

            try:
                print("Scanning " + filename, file=sys.stderr)
                f = open(filename, "rb")
                scan_opened_file(
                    f,
                    handshake_decoder,
                    client_decoder,
                    server_decoder,
                    output_queue,
                    handshake,
                    messages,
                    thread_id,
                    start_string,
                )
                start_string = ","
                f.close()
                roll_index += 1
                if last_chance:
                    break
            except FileNotFoundError:
                if last_chance:
                    break
                last_chance = True
                continue

        print("]")

    else:
        raise ValueError(basename + " is not a valid rolled logfile name")


if __name__ == "__main__":
    (file, handshake, messages, thread_id, rolled) = command_line.parse_command_line()

    if rolled:
        scan_file_sequence(file, handshake, messages, thread_id)
    else:
        scan_file(file, handshake, messages, thread_id)
