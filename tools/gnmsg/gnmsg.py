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
import re
import sys
import threading
import traceback


from modified_utf8 import utf8m_to_utf8s
from numeric_conversion import to_hex_digit
import command_line

from ds_codes import ds_codes
from connection_types import ConnectionTypes, ConnectionTypeStrings
from read_values import (
    read_number_from_hex_string,
    read_byte_value,
    read_number_from_hex_string,
    read_short_value,
    read_number_from_hex_string,
    read_int_value,
    read_long_value,
    read_string_value,
    read_jmutf8_string_value,
    read_number_from_hex_string,
    call_reader_function,
)
from client_message_decoder import ClientMessageDecoder
from server_message_decoder import ServerMessageDecoder
from handshake_decoder import HandshakeDecoder


def scan_file(filename, dump_handshake, dump_messages, thread_id):
    output_queue = queue.Queue()
    separator = ""
    if dump_handshake:
        handshake_decoder = HandshakeDecoder(output_queue)
        with open(filename, "rb") as f:
            for line in f:
                handshake_decoder.process_line(line.decode("utf-8"))
                try:
                    data = output_queue.get_nowait()
                    for key, value in data.items():
                        if key == "handshake":
                            print(separator + json.dumps(value, indent=2, default=str))
                            separator = ","
                except queue.Empty:
                    continue

    separator = ""
    client_decoder = ClientMessageDecoder(output_queue)
    server_decoder = ServerMessageDecoder(output_queue)
    print("[")
    with open(filename, "rb") as f:
        for line in f:
            linestr = line.decode("utf-8")
            client_decoder.process_line(linestr)
            server_decoder.process_line(linestr)
            try:
                data = output_queue.get_nowait()
                for key, value in data.items():
                    if key == "message" and dump_messages:
                        if thread_id:
                            if "tid" in value.keys() and value["tid"] == thread_id:
                                print(
                                    separator + json.dumps(value, indent=2, default=str)
                                )
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

    print("]")


if __name__ == "__main__":
    (file, handshake, messages, thread_id) = command_line.parse_command_line()
    scan_file(file, handshake, messages, thread_id)
