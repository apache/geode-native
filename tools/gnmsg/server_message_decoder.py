# get_send_trace_parts_functionsrse
# !/usr/local/bin/python3

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
import re
import struct
import sys

from dateutil import parser

from server_messages import parse_server_message
from decoder_base import DecoderBase
from message_types import message_types
from numeric_conversion import to_hex_digit, decimal_string_to_hex_string
from chunked_message_decoder import ChunkedResponseDecoder
from read_values import read_number_from_hex_string
from gnmsg_globals import global_protocol_state


class ServerMessageDecoder(DecoderBase):
    def __init__(self, output_queue):
        super(ServerMessageDecoder, self).__init__(output_queue)
        self.STATE_NEUTRAL_ = 0
        self.STATE_WAITING_FOR_MESSAGE_BODY_ = 1
        self.receive_trace_parts_retriever_ = None
        self.receive_trace_parser_ = None
        self.connection_states_ = {}
        self.headers_ = {}
        self.nc_version_ = None
        self.get_receive_trace_parts_functions_ = [
            self.get_receive_trace_header_base,
            self.get_receive_trace_header_v911,
        ]
        self.receive_trace_parsers_ = [
            self.parse_response_fields_base,
            self.parse_response_fields_v911,
        ]
        self.chunk_decoders_ = {}
        self.threads_connections_ = {}

        self.connection_to_tid_expression_ = re.compile(
            r"(\d\d:\d\d:\d\d\.\d+).+:\d+\s+([\d|a-f|A-F|x|X]+)\]\s*TcrConnection::send:\s*\[([\d|a-f|A-F|x|X]+).*sending request to endpoint.*bytes:\s*([\d|a-f|A-F]+)"
        )

        self.trace_header_with_pointer_expression_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).+:\d+\s+([\d|a-f|A-F|x|X]+)\]\s*TcrConnection::readMessage\(([\d|a-f|A-F|x|X]+)\):.*received header from endpoint.*bytes:\s*([\d|a-f|A-F]+)"
        )

        self.trace_header_without_pointer_expression_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).*:\d+\s+([\d|a-f|A-F|x|X]+)\]\s*TcrConnection::readMessage:\s*received header from endpoint.*bytes:\s*([\d|a-f|A-F]+)"
        )

        self.trace_header_v911_expression_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).*:\d+\s+(\d+)\]\s*TcrConnection::readMessage: received header from endpoint.*bytes:\s*([\d| ]+)"
        )

        self.receive_trace_body_expression_ = re.compile(
            ":\d+\s+([\d|a-f|A-F|x|X]+)\]\s*TcrConnection::readMessage: received message body from endpoint.*bytes:\s*([\d|a-f|A-F]+)"
        )

        self.security_trace_expression_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).*:\d+\s+(\d+)\]\s*TcrMessage::addSecurityPart\s*\[(0x[\d|a-f|A-F]*).*length\s*=\s*(\d+)\s*,\s*encrypted\s+ID\s*=\s*([\d|a-f|A-F]+)"
        )

        self.response_header_with_pointer_expression_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).*:\d+\s+(\d+)\]\s*TcrConnection::readResponseHeader\(([0-9|a-f|A-F|x]+)\):\s*received header from endpoint\s*([\w|:|\d|\.|-]+);\s*bytes:\s*([\d|a-f|A-F]+)"
        )

        self.response_header_without_pointer_expression_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).*:\d+\s+(\d+)\]\s*TcrConnection::readResponseHeader:\s*received header from endpoint\s*([\w|:|\d|\.|-]+);\s*bytes:\s*([\d|a-f|A-F]+)"
        )

        self.chunk_header_with_pointer_expression_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).*:\d+\s+(\d+)\]\s*TcrConnection::readChunkHeader\(([0-9|a-f|A-F|x]+)\):\s*.*, chunkLen=(\d+), lastChunkAndSecurityFlags=([0-9|a-f|A-F|x]+)"
        )

        self.chunk_header_without_pointer_expression_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).*:\d+\s+(\d+)\]\s*TcrConnection::readChunkHeader:\s*.*, chunkLen=(\d+), lastChunkAndSecurityFlags=([0-9|a-f|A-F|x]+)"
        )

        self.chunk_bytes_with_pointer_expression_ = re.compile(
            r"(\d\d:\d\d:\d\d\.\d+).*:\d+\s+(\d+)\]\s*TcrConnection::readChunkBody\(([0-9|a-f|A-F|x]+)\): \s*received chunk body from endpoint\s*([\w|:|\d|\.|-]+);\s*bytes:\s*([\d|a-f|A-F]+)"
        )
        self.chunk_bytes_without_pointer_expression_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).*:\d+\s+(\d+)\]\s*TcrConnection::readChunkBody: \s*received chunk body from endpoint\s*([\w|:|\d|\.|-]+);\s*bytes:\s*([\d|a-f|A-F]+)"
        )

    def associate_connection_to_tid(self, line):
        result = False
        match = self.connection_to_tid_expression_.search(line)
        if match:
            tid = match.group(2)
            connection = match.group(3)
            self.threads_connections_[tid] = connection
            result = True

        return result

    def get_receive_trace_header_with_pointer(self, line, parts):
        result = False
        match = self.trace_header_with_pointer_expression_.search(line)
        if match:
            date_time = parser.parse(match.group(1))
            tid = match.group(2)
            connection = match.group(3)
            bytes = match.group(4)
            parts.append(date_time)
            parts.append(tid)
            parts.append(connection)
            parts.append(bytes)
            self.threads_connections_[tid] = connection
            result = True

        return result

    def get_receive_trace_header_without_pointer(self, line, parts):
        result = False
        match = self.trace_header_without_pointer_expression_.search(line)
        if match:
            tid = match.group(2)
            parts.append(parser.parse(match.group(1)))
            parts.append(tid)
            if tid in self.threads_connections_.keys():
                parts.append(self.threads_connections_[tid])
            else:
                parts.append("0")
            parts.append(match.group(3))
            result = True

        return result

    def get_receive_trace_header_base(self, line, parts):
        result = self.get_receive_trace_header_with_pointer(line, parts)
        if not result:
            result = self.get_receive_trace_header_without_pointer(line, parts)

        return result

    def get_receive_trace_header_v911(self, line, parts):
        result = False
        match = self.trace_header_v911_expression_.search(line)
        if match:
            tid = match.group(2)
            parts.append(parser.parse(match.group(1)))
            parts.append(parser.parse(tid))
            if tid in self.threads_connections_.keys():
                parts.append(self.threads_connections_[tid])
            else:
                parts.append("0")
            parts.append(match.group(3))
            result = True

        return result

    def get_receive_trace_body_parts(self, line, parts):
        result = False
        match = self.receive_trace_body_expression_.search(line)
        if match:
            tid = match.group(1)
            parts.append(tid)
            if tid in self.threads_connections_.keys():
                parts.append(self.threads_connections_[tid])
            else:
                parts.append("0")
            parts.append(match.group(2))
            result = True

        return result

    def get_receive_trace_parts(self, line, parts):
        if self.receive_trace_parts_retriever_ is not None:
            return self.receive_trace_parts_retriever_(line, parts)
        else:
            for retriever in self.get_receive_trace_parts_functions_:
                if retriever(line, parts):
                    self.receive_trace_parts_retriever_ = retriever
                    self.receive_trace_parser_ = self.receive_trace_parsers_[
                        self.get_receive_trace_parts_functions_.index(retriever)
                    ]
                    return True
            else:
                return False

    def get_add_security_trace_parts(self, line, parts):
        result = False
        match = self.security_trace_expression_.search(line)
        if match:
            parts.append(parser.parse(match.group(1)))
            parts.append(match.group(2))
            parts.append(match.group(3))
            parts.append(match.group(4))
            parts.append(match.group(5))
            result = True

        return result

    def get_response_header(self, line, parts):
        # Check if this is a header for a chunked message
        result = False

        match = self.response_header_with_pointer_expression_.search(line)
        if match:
            tid = match.group(2)
            connection = match.group(3)
            self.threads_connections_[tid] = connection
            parts.append(parser.parse(match.group(1)))
            parts.append(tid)
            parts.append(connection)
            parts.append(match.group(4))
            parts.append(match.group(5))
            result = True

        if not result:
            match = self.response_header_without_pointer_expression_.search(line)
            if match:
                parts.append(parser.parse(match.group(1)))
                parts.append(match.group(2))
                parts.append("")
                parts.append(match.group(3))
                parts.append(match.group(4))
                result = True

        return result

    def get_chunk_header(self, line, parts):
        # Check if this is a header for a chunked message
        result = False
        match = self.chunk_header_with_pointer_expression_.search(line)
        if match:
            tid = match.group(2)
            connection = match.group(3)
            self.threads_connections_[tid] = connection
            parts.append(parser.parse(match.group(1)))
            parts.append(tid)
            parts.append(connection)
            parts.append(match.group(4))
            parts.append(match.group(5))
            result = True

        if not result:
            match = self.chunk_header_without_pointer_expression_.search(line)
            if match:
                parts.append(parser.parse(match.group(1)))
                tid = match.group(2)
                parts.append(tid)
                if tid in self.threads_connections_.keys():
                    parts.append(self.threads_connections_[tid])
                else:
                    parts.append("0")
                parts.append(match.group(3))
                parts.append(match.group(4))
                result = True

        return result

    def get_chunk_bytes(self, line, parts):
        # Check if this is a message chunk.
        # If it is, add it to the chunked decoder
        result = False
        match = self.chunk_bytes_with_pointer_expression_.search(line)
        if match:
            parts.append(parser.parse(match.group(1)))
            parts.append(match.group(2))
            parts.append(match.group(3))
            parts.append(match.group(4))
            parts.append(match.group(5))
            result = True

        if not result:
            match = self.chunk_bytes_without_pointer_expression_.search(line)
            if match:
                parts.append(parser.parse(match.group(1)))
                tid = match.group(2)
                parts.append(tid)
                if tid in self.threads_connections_.keys():
                    parts.append(self.threads_connections_[tid])
                else:
                    parts.append("0")
                parts.append(match.group(3))
                parts.append(match.group(4))
                result = True

        return result

    def format_bytes_as_hex_v911(self, message_bytes):
        byte_list = message_bytes.split(" ")
        hex_string = ""
        for byte in byte_list:
            if byte:
                hex_string += decimal_string_to_hex_string(byte)
        return hex_string

    def parse_response_fields_base(self, message_bytes):
        message_type = message_types[int(message_bytes[0:8], 16)]
        message_length = int(message_bytes[8:16], 16)
        message_number_of_parts = int(message_bytes[16:24], 16)
        message_transaction_id = struct.unpack(
            ">i", bytes.fromhex(message_bytes[24:32])
        )[0]
        message_security_flag = (int(message_bytes[32:34], 16) & 0x02) >> 1
        return (
            message_type,
            message_length,
            message_number_of_parts,
            message_transaction_id,
            message_security_flag,
        )

    def parse_response_fields_v911(self, message_bytes):
        hex_message_bytes = self.format_bytes_as_hex_v911(message_bytes)
        message_type = message_types[int(hex_message_bytes[0:8], 16)]
        message_length = int(hex_message_bytes[8:16], 16)
        message_number_of_parts = int(hex_message_bytes[16:24], 16)
        message_transaction_id = struct.unpack(
            ">i", bytes.fromhex(hex_message_bytes[24:32])
        )[0]
        message_security_flag = (int(hex_message_bytes[32:34], 16) & 0x02) >> 1
        return (
            message_type,
            message_length,
            message_number_of_parts,
            message_transaction_id,
            message_security_flag,
        )

    def parse_response_fields(self, message_bytes):
        if self.receive_trace_parser_ is not None:
            return self.receive_trace_parser_(message_bytes)

    def is_candidate_line(self, line):
        return "TcrMess" in line or "TcrConn" in line

    def process_line(self, line):
        connection = None
        message_bytes = None
        message_body = None
        chunk_bytes = None
        tid = None

        if not self.is_candidate_line(line):
            return

        parts = []
        if self.associate_connection_to_tid(line):
            pass
        elif self.get_receive_trace_parts(line, parts):
            tid = parts[1]
            last_header = {"Timestamp": parts[0], "tid": tid, "Connection": parts[2]}
            message_bytes = parts[3]
            self.headers_[tid] = last_header
            if (
                tid in self.connection_states_.keys()
                and self.connection_states_[tid] != self.STATE_NEUTRAL_
            ):
                print("WARNING: Multiple headers rec'd without a message body.")
            self.connection_states_[tid] = self.STATE_NEUTRAL_
        elif self.get_receive_trace_body_parts(line, parts):
            tid = parts[0]
            connection = parts[1]
            message_body = parts[2]
        elif self.get_add_security_trace_parts(line, parts):
            tid = parts[1]
            connection = parts[2]
        elif self.get_response_header(line, parts):
            tid = parts[1]
            connection = parts[2]
            if tid in self.chunk_decoders_.keys():
                self.chunk_decoders_[tid].add_header(parts[2], parts[4])
                if self.chunk_decoders_[tid].is_complete_message():
                    receive_trace = self.chunk_decoders_[tid].get_decoded_message(
                        parts[0]
                    )
                    receive_trace["tid"] = str(tid)
                    self.output_queue_.put({"message": receive_trace})
                    self.chunk_decoders_[tid].reset()
            else:
                self.chunk_decoders_[tid] = ChunkedResponseDecoder()
                self.chunk_decoders_[tid].add_header(parts[2], parts[4])
        elif self.get_chunk_header(line, parts):
            flags = 0xFF
            size = 0
            tid = parts[1]
            (flags, size) = read_number_from_hex_string(parts[4], 2, len(parts[4]) - 2)
            if tid in self.chunk_decoders_.keys():
                self.chunk_decoders_[tid].add_chunk_header(int(parts[3]), flags)
                if self.chunk_decoders_[tid].is_complete_message():
                    receive_trace = self.chunk_decoders_[tid].get_decoded_message(
                        parts[0]
                    )
                    receive_trace["tid"] = str(tid)
                    self.output_queue_.put({"message": receive_trace})
                    self.chunk_decoders_[tid].reset()
        else:
            return

        if tid not in self.connection_states_:
            self.connection_states_[tid] = self.STATE_NEUTRAL_

        if self.connection_states_[tid] == self.STATE_NEUTRAL_:
            if message_bytes:

                last_header = self.headers_[tid]
                last_header["Direction"] = "<---"
                (
                    last_header["Type"],
                    last_header["Length"],
                    last_header["Parts"],
                    last_header["TransactionId"],
                    last_header["SecurityFlag"],
                ) = self.parse_response_fields(message_bytes)
                self.headers_[tid] = last_header

                self.connection_states_[tid] = self.STATE_WAITING_FOR_MESSAGE_BODY_
        elif self.connection_states_[tid] == self.STATE_WAITING_FOR_MESSAGE_BODY_:
            if message_body:
                receive_trace = self.headers_[tid]
                self.headers_[tid] = None
                parse_server_message(receive_trace, message_body)
                self.connection_states_[tid] = self.STATE_NEUTRAL_
                self.output_queue_.put({"message": receive_trace})
