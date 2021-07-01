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

from client_messages import parse_client_message
from decoder_base import DecoderBase
from message_types import message_types
from numeric_conversion import to_hex_digit
from gnmsg_globals import global_protocol_state


class ClientMessageDecoder(DecoderBase):
    def __init__(self, output_queue):
        super(ClientMessageDecoder, self).__init__(output_queue)
        self.STATE_NEUTRAL_ = 0
        self.STATE_FOUND_SECURITY_FOOTER_ = 1
        self.send_trace_parts_retriever_ = None
        self.send_trace_parser_ = None
        self.connection_states_ = {}
        self.get_send_trace_parts_functions = [
            self.get_send_trace_parts_base,
            self.get_send_trace_parts_v911,
        ]
        self.send_trace_parsers = [
            self.parse_request_fields_base,
            self.parse_request_fields_v911,
        ]

        #
        # Native client code believes this is the list of messages that require a security footer.
        # We will use this list to verify and report if a message is sent that needs one but doesn't
        # have it, since this has been the source of at least one difficult-to-diagnose bug in the
        # past.  To see the decision-making code that filters on this message list, look at
        # ThinClientBaseDM::beforeSendingRequest and TcrMessage::isUserInitiativeOps in geode-native
        # C++ code base.
        self.message_requires_security_part = [
            "ADD_PDX_ENUM",
            "ADD_PDX_TYPE",
            "CLIENT_READY",
            "CLOSE_CONNECTION",
            "COMMIT",
            "GETCQSTATS_MSG_TYPE",
            "GET_CLIENT_PARTITION_ATTRIBUTES",
            "GET_CLIENT_PR_METADATA",
            "GET_ENTRY",
            "GET_FUNCTION_ATTRIBUTES",
            "GET_PDX_ENUM_BY_ID",
            "GET_PDX_ID_FOR_ENUM",
            "GET_PDX_ID_FOR_TYPE",
            "GET_PDX_TYPE_BY_ID",
            "INVALID",
            "MAKE_PRIMARY",
            "MONITORCQ_MSG_TYPE",
            "PERIODIC_ACK",
            "PING",
            "REQUEST_EVENT_VALUE",
            "ROLLBACK",
            "SIZE",
            "TX_FAILOVER",
            "TX_SYNCHRONIZATION",
            "USER_CREDENTIAL_MESSAGE",
        ]

        self.security_trace_expression_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).*([\d|a-f|A-F|x|X]+)\]\s*TcrMessage::addSecurityPart\s*\[(0x[\d|a-f|A-F]*).*length\s*=\s*(\d+)\s*,\s*encrypted\s+ID\s*=\s*([\d|a-f|A-F]+)"
        )

        self.send_trace_expression_v911_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).*TcrConnection::send:\s*\[([\d|a-f|A-F|x|X]+).*sending request to endpoint.*bytes:\s*([\d| ]+)"
        )

        self.send_trace_expression_base_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).+:\d+\s+([\d|a-f|A-F|x|X]+)\]\s*TcrConnection::send:\s*\[([\d|a-f|A-F|x|X]+).*sending request to endpoint.*bytes:\s*([\d|a-f|A-F]+)"
        )

    def get_send_trace_parts_v911(self, line, parts):
        result = False
        match = self.send_trace_expression_v911_.search(line)
        if match:
            parts.append(parser.parse(match.group(1)))
            # TODO: Revisit parsing TID here if we ever see a v9 client log again
            parts.append("0")
            parts.append(match.group(2))
            parts.append(match.group(3))
            result = True

        return result

    def get_send_trace_parts_base(self, line, parts):
        result = False
        match = self.send_trace_expression_base_.search(line)
        if match:
            parts.append(parser.parse(match.group(1)))
            parts.append(match.group(2))
            parts.append(match.group(3))
            parts.append(match.group(4))
            result = True

        return result

    def get_send_trace_parts(self, line, parts):
        if self.send_trace_parts_retriever_ is not None:
            return self.send_trace_parts_retriever_(line, parts)
        else:
            for retriever in self.get_send_trace_parts_functions:
                if retriever(line, parts):
                    self.send_trace_parts_retriever_ = retriever
                    self.send_trace_parser_ = self.send_trace_parsers[
                        self.get_send_trace_parts_functions.index(retriever)
                    ]
                    return True
            else:
                return False

    def get_add_security_trace_parts(self, line, parts):
        result = False

        if "addSec" in line:
            match = self.security_trace_expression_.search(line)
            if match:
                parts.append(parser.parse(match.group(1)))
                parts.append(match.group(2))
                parts.append(match.group(3))
                parts.append(match.group(4))
                parts.append(match.group(5))
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

    def parse_request_fields_v911(self, message_bytes):
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

    def parse_request_fields_base(self, message_bytes):
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

    def parse_request_fields(self, message_bytes):
        if self.send_trace_parser_ is not None:
            return self.send_trace_parser_(message_bytes)

    def request_requires_security_footer(self, message_type):
        return message_type in self.message_requires_security_part

    def is_candidate_line(self, line):
        return "TcrMess" in line or "TcrConn" in line

    def process_line(self, line):
        connection = None
        is_send_trace = False
        is_add_security_trace = False
        send_trace = {}

        if not self.is_candidate_line(line):
            return

        parts = []
        if self.get_send_trace_parts(line, parts):
            (
                send_trace["Timestamp"],
                send_trace["tid"],
                send_trace["Connection"],
                message_bytes,
            ) = parts
            is_send_trace = True
        elif self.get_add_security_trace_parts(line, parts):
            timestamp, tid, connection, security_footer_length, message_bytes = parts
            is_add_security_trace = True
        else:
            return

        if connection not in self.connection_states_:
            self.connection_states_[connection] = self.STATE_NEUTRAL_

        if self.connection_states_[connection] == self.STATE_NEUTRAL_:
            if is_add_security_trace:
                self.connection_states_[connection] = self.STATE_FOUND_SECURITY_FOOTER_
            elif is_send_trace:
                send_trace["Direction"] = "--->"
                (
                    send_trace["Type"],
                    send_trace["Length"],
                    send_trace["Parts"],
                    send_trace["TransactionId"],
                    send_trace["SecurityFlag"],
                ) = self.parse_request_fields(message_bytes)
                if (send_trace["SecurityFlag"] == 1) and (
                    self.request_requires_security_footer(str(send_trace["Type"]))
                ):
                    print(
                        "ERROR: Security flag is set, but no footer was added for this message!"
                    )

                parse_client_message(send_trace, message_bytes)
                self.output_queue_.put({"message": send_trace})
                global_protocol_state.set_last_client_message(
                    send_trace["tid"], send_trace["Type"]
                )
        elif self.connection_states_[connection] == self.STATE_FOUND_SECURITY_FOOTER_:
            if is_send_trace:
                send_trace["Direction"] = "--->"
                (
                    send_trace["Type"],
                    send_trace["Length"],
                    send_trace["Parts"],
                    send_trace["TransactionId"],
                    send_trace["SecurityFlag"],
                ) = self.parse_request_fields(message_bytes)
                self.output_queue_.put({"message": send_trace})
                global_protocol_state.set_last_client_message(
                    send_trace["tid"], send_trace["Type"]
                )
