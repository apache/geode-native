#!/usr/local/bin/python3


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


def scan_file(filename, dump_handshake, dump_messages):
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
                            print(separator + json.dumps(data, indent=2, default=str))
                            separator = ","
                except queue.Empty:
                    continue

    separator = ""
    client_decoder = ClientMessageDecoder(output_queue)
    server_decoder = ServerMessageDecoder(output_queue)
    with open(filename, "rb") as f:
        for line in f:
            linestr = line.decode("utf-8")
            client_decoder.process_line(linestr)
            server_decoder.process_line(linestr)
            try:
                data = output_queue.get_nowait()
                for key, value in data.items():
                    if key == "message" and dump_messages:
                        print(separator + json.dumps(data, indent=2, default=str))
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


if __name__ == "__main__":
    (file, handshake, messages) = command_line.parse_command_line()
    scan_file(file, handshake, messages)
