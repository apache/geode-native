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
import re
import struct

from dateutil import parser

from server_messages import parse_server_message
from decoder_base import DecoderBase
from message_types import message_types
from numeric_conversion import to_hex_digit


class ServerMessageDecoder(DecoderBase):
    def __init__(self, output_queue):
        super(ServerMessageDecoder, self).__init__(output_queue)
        self.STATE_NEUTRAL_ = 0
        self.STATE_WAITING_FOR_MESSAGE_BODY_ = 1
        self.receive_trace_parts_retriever_ = None
        self.receive_trace_parser_ = None
        self.connection_states_ = {}
        self.last_header_ = {}
        self.nc_version_ = None
        self.get_receive_trace_parts_functions_ = {
            "0.0.42": self.get_receive_trace_header_base,
            "10.0.3": self.get_receive_trace_header_base,
            "10.1.1": self.get_receive_trace_header_base,
            "10.1.2": self.get_receive_trace_header_base,
            "10.1.3": self.get_receive_trace_header_base,
            "9.1.1": self.get_receive_trace_header_v911,
        }
        self.receive_trace_parsers_ = {
            "0.0.42": self.parse_response_fields_base,
            "10.0.3": self.parse_response_fields_base,
            "10.1.1": self.parse_response_fields_base,
            "10.1.2": self.parse_response_fields_base,
            "10.1.3": self.parse_response_fields_base,
            "9.1.1": self.parse_response_fields_v911,
        }

    def search_for_version(self, line):
        if self.nc_version_ == None:
            expression = re.compile(r"Product version:.*Native (\d+)\.(\d+)\.(\d+)-")
            match = expression.search(line)
            if match:
                major = match.group(1)
                minor = match.group(2)
                patch = match.group(3)
                self.nc_version_ = major + "." + minor + "." + patch
                self.receive_trace_parts_retriever_ = self.get_receive_trace_parts_functions_[
                    self.nc_version_
                ]
                self.receive_trace_parser_ = self.receive_trace_parsers_[
                    self.nc_version_
                ]

    def get_receive_trace_header_with_pointer(self, line, parts):
        result = False
        expression = re.compile(
            r"(\d\d:\d\d:\d\d\.\d+).*TcrConnection::readMessage:\s*\[([\d|a-f|A-F|x|X]+).*received header from endpoint.*bytes:\s*([\d|a-f|A-F]+)"
        )
        match = expression.search(line)
        if match:
            parts.append(match.group(1))
            parts.append(match.group(2))
            parts.append(match.group(3))
            result = True

        return result

    def get_receive_trace_header_without_pointer(self, line, parts):
        result = False
        expression = re.compile(
            r"(\d\d:\d\d:\d\d\.\d+).*TcrConnection::readMessage:\s*received header from endpoint.*bytes:\s*([\d|a-f|A-F]+)"
        )
        match = expression.search(line)
        if match:
            parts.append(match.group(1))
            parts.append("0")
            parts.append(match.group(2))
            result = True

        return result

    def get_receive_trace_header_base(self, line, parts):
        result = self.get_receive_trace_header_with_pointer(line, parts)
        if not result:
            result = self.get_receive_trace_header_without_pointer(line, parts)

        return result

    def get_receive_trace_header_v911(self, line, parts):
        result = False
        expression = re.compile(
            r"(\d\d:\d\d:\d\d\.\d+).*TcrConnection::readMessage: received header from endpoint.*bytes:\s*([\d| ]+)"
        )
        match = expression.search(line)
        if match:
            parts.append(parser.parse(match.group(1)))
            parts.append("0")
            parts.append(match.group(2))
            result = True

        return result

    def get_receive_trace_body_parts(self, line, parts):
        result = False
        expression = re.compile(
            "received message body from endpoint.*bytes:\s*([\d|a-f|A-F]+)"
        )
        match = expression.search(line)
        if match:
            message = match.group(1)
            parts.append(message)
            result = True

        return result

    def get_receive_trace_parts(self, line, parts):
        if self.receive_trace_parts_retriever_ is not None:
            return self.receive_trace_parts_retriever_(line, parts)

    def get_add_security_trace_parts(self, line, parts):
        result = False
        expression = re.compile(
            r"(\d\d:\d\d:\d\d\.\d+).*TcrMessage::addSecurityPart\s*\[(0x[\d|a-f|A-F]*).*length\s*=\s*(\d+)\s*,\s*encrypted\s+ID\s*=\s*([\d|a-f|A-F]+)"
        )
        match = expression.search(line)
        if match:
            parts.append(parser.parse(match.group(1)))
            parts.append(match.group(2))
            parts.append(match.group(3))
            parts.append(match.group(4))
            result = True

        return result

    def decimal_string_to_hex_string(self, byte):
        high_nibble = int(int(byte) / 16)
        low_nibble = int(byte) % 16
        return to_hex_digit[high_nibble] + to_hex_digit[low_nibble]

    def format_bytes_as_hex_v911(self, message_bytes):
        byte_list = message_bytes.split(" ")
        hex_string = ""
        for byte in byte_list:
            if byte:
                hex_string += self.decimal_string_to_hex_string(byte)
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

    def process_line(self, line):
        connection = None
        message_bytes = None
        message_body = None

        self.search_for_version(line)

        parts = []
        if self.get_receive_trace_parts(line, parts):
            (
                self.last_header_["Timestamp"],
                self.last_header_["Connection"],
                message_bytes,
            ) = parts
        elif self.get_receive_trace_body_parts(line, parts):
            message_body = parts[0]
        elif self.get_add_security_trace_parts(line, parts):
            connection = parts[1]
        else:
            return

        if connection not in self.connection_states_:
            self.connection_states_[connection] = self.STATE_NEUTRAL_

        if self.connection_states_[connection] == self.STATE_NEUTRAL_:
            if message_bytes:
                self.last_header_["Direction"] = "<---"
                (
                    self.last_header_["Type"],
                    self.last_header_["Length"],
                    self.last_header_["Parts"],
                    self.last_header_["TransactionId"],
                    self.last_header_["SecurityFlag"],
                ) = self.parse_response_fields(message_bytes)
                self.connection_states_[
                    connection
                ] = self.STATE_WAITING_FOR_MESSAGE_BODY_
        elif (
            self.connection_states_[connection] == self.STATE_WAITING_FOR_MESSAGE_BODY_
        ):
            if message_body:
                receive_trace = self.last_header_
                self.last_header_ = {}
                parse_server_message(receive_trace, message_body)
                self.connection_states_[connection] = self.STATE_NEUTRAL_
                self.output_queue_.put({"message": receive_trace})
