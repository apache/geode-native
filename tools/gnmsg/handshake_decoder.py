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
import re

from decoder_base import DecoderBase
from ds_codes import ds_codes
from modified_utf8 import utf8m_to_utf8s
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


class HandshakeDecoder(DecoderBase):
    def __init__(self, output_queue):
        super(HandshakeDecoder, self).__init__(output_queue)
        self.separator = ""
        self.credentials_types = {
            0: "SECURITY_CREDENTIALS_NONE",
            1: "SECURITY_CREDENTIALS_NORMAL",
            2: "SECURITY_CREDENTIALS_DHENCRYPT",
            3: "SECURITY_MULTIUSER_NOTIFICATIONCHANNEL",
        }

    def is_handshake_trace(self, line):
        expression = re.compile(r"Handshake bytes: \(\d+\):\s*([0-9|a-f|A-F]+)")
        match = expression.search(line)
        if match:
            return True
        else:
            return False

    def get_handshake_bytes(self, line):
        expression = re.compile(r"Handshake bytes: \(\d+\):\s*([0-9|a-f|A-F]+)")
        match = expression.search(line)
        if match:
            return match.group(1)
        else:
            exit(1)

    # TODO: Find a handshake that uses a list and implement this function to parse it
    def read_list_of_ports(string):
        return (1001, 1002, 1003)

    def get_client_proxy_address(self, string, offset):
        (address_size, offset) = call_reader_function(string, offset, read_byte_value)
        result = ""

        if int(address_size) == 4:
            (octet1, offset) = call_reader_function(string, offset, read_byte_value)
            (octet2, offset) = call_reader_function(string, offset, read_byte_value)
            (octet3, offset) = call_reader_function(string, offset, read_byte_value)
            (octet4, offset) = call_reader_function(string, offset, read_byte_value)
            result = (
                str(octet1) + "." + str(octet2) + "." + str(octet3) + "." + str(octet4)
            )
        return (result, offset)

    def convert_to_bytes(self, string, length):
        result = bytearray()
        for i in range(0, length, 2):
            byte = int(string[i : i + 2], 16)
            result.append(byte)
        return result

    def read_cacheable_string(self, string, offset):
        (dscode, offset) = call_reader_function(string, offset, read_byte_value)
        hostname = ""
        string_type = ds_codes[dscode]

        if string_type == "CacheableString":
            (length_high_byte, offset) = call_reader_function(
                string, offset, read_byte_value
            )
            (length_low_byte, offset) = call_reader_function(
                string, offset, read_byte_value
            )
            string_length = (length_high_byte << 8) + length_low_byte
            string_bytes = string[offset : offset + string_length * 2]
            hostname = utf8m_to_utf8s(
                self.convert_to_bytes(string_bytes, string_length * 2)
            ).decode("utf-8")
            offset += string_length * 2
        elif string_type == "CacheableStringHuge":
            (length_byte_3, offset) = call_reader_function(
                string, offset, read_byte_value
            )
            (length_byte_2, offset) = call_reader_function(
                string, offset, read_byte_value
            )
            (length_byte_1, offset) = call_reader_function(
                string, offset, read_byte_value
            )
            (length_byte_0, offset) = call_reader_function(
                string, offset, read_byte_value
            )
            string_length = (
                (length_byte_3 << 24)
                + (length_byte_2 << 16)
                + (length_byte_1 << 8)
                + length_byte_0
            )
            string_bytes = string[offset : offset + string_length * 2]
            hostname = utf8m_to_utf8s(
                self.convert_to_bytes(string_bytes, string_length * 2)
            ).decode("utf-8")
            offset += string_length * 2
        elif ds_codes[dscode] == "CacheableNullString":
            # CacheableNullString is a signal that there's actually nothing here, so just return empty
            hostname = ""
        else:
            hostname = ""
            offset -= 2

        return (hostname, offset)

    def get_client_proxy_role_array_length(self, string, offset):
        (maybe_length, bytes_read) = read_byte_value(string, offset)
        offset += bytes_read
        array_length = 0

        if maybe_length == 255:
            array_length = -1
        elif maybe_length == 254:
            array_length = read_short_value(string, offset)
            offset += 4
        elif maybe_length == 253:
            array_length = read_int_value(string, offset)
            offset += 8
        else:
            array_length = maybe_length

        return (array_length, offset)

    def get_client_proxy_version_ordinal(self, string, offset):
        (maybe_ordinal, offset) = call_reader_function(string, offset, read_byte_value)
        ordinal = 0
        if maybe_ordinal == 255:
            (ordinal, offset) = call_reader_function(string, offset, read_short_value)
        else:
            ordinal = maybe_ordinal

        return (ordinal, offset)

    def get_credentials_type(self, string, offset):
        (credential_type, offset) = call_reader_function(
            string, offset, read_byte_value
        )
        return (self.credentials_types[credential_type], offset)

    def get_handshake_info(self, line, handshake_info):
        handshake_bytes = self.get_handshake_bytes(line)
        (connection_type, offset) = call_reader_function(
            handshake_bytes, 0, read_byte_value
        )
        handshake_info["ConnectionType"] = ConnectionTypeStrings[connection_type]

        (handshake_info["VersionOrdinal"], offset) = call_reader_function(
            handshake_bytes, offset, read_byte_value
        )

        (handshake_info["ReplyOK"], offset) = call_reader_function(
            handshake_bytes, offset, read_byte_value
        )

        ports = ()
        if (connection_type == ConnectionTypes.PRIMARY_SERVER_TO_CLIENT) or (
            connection_type == ConnectionTypes.SECONDARY_SERVER_TO_CLIENT
        ):
            ports = self.read_list_of_ports(handshake_bytes)
            handshake_info["Ports"] = ports

        (handshake_info["ReadTimeout"], offset) = call_reader_function(
            handshake_bytes, offset, read_int_value
        )

        (handshake_info["FixedIDByte"], offset) = call_reader_function(
            handshake_bytes, offset, read_byte_value
        )

        client_proxy = {}
        (client_proxy["MembershipIDByte"], offset) = call_reader_function(
            handshake_bytes, offset, read_byte_value
        )

        # TODO: Solve this mystery!
        (client_proxy["MysteryByte"], offset) = call_reader_function(
            handshake_bytes, offset, read_byte_value
        )

        (client_proxy["FixedIDByte"], offset) = call_reader_function(
            handshake_bytes, offset, read_byte_value
        )

        (client_proxy["InternalDistributedMemberID"], offset) = call_reader_function(
            handshake_bytes, offset, read_byte_value
        )

        (client_proxy["Address"], offset) = self.get_client_proxy_address(
            handshake_bytes, offset
        )

        (client_proxy["SyncCounter"], offset) = call_reader_function(
            handshake_bytes, offset, read_int_value
        )

        (client_proxy["Hostname"], offset) = self.read_cacheable_string(
            handshake_bytes, offset
        )

        (client_proxy["SplitBrainFlag"], offset) = call_reader_function(
            handshake_bytes, offset, read_byte_value
        )

        (client_proxy["DCPort"], offset) = call_reader_function(
            handshake_bytes, offset, read_int_value
        )

        (client_proxy["VPID"], offset) = call_reader_function(
            handshake_bytes, offset, read_int_value
        )

        (client_proxy["VMKind"], offset) = call_reader_function(
            handshake_bytes, offset, read_byte_value
        )

        (
            client_proxy["RoleArrayLength"],
            offset,
        ) = self.get_client_proxy_role_array_length(handshake_bytes, offset)

        (client_proxy["DSName"], offset) = self.read_cacheable_string(
            handshake_bytes, offset
        )

        (client_proxy["UniqueTag"], offset) = self.read_cacheable_string(
            handshake_bytes, offset
        )

        (client_proxy["DurableClientID"], offset) = self.read_cacheable_string(
            handshake_bytes, offset
        )

        client_proxy_durable_client_timeout = 0
        if "DurableClientID" in client_proxy:
            (client_proxy_durable_client_timeout, offset) = call_reader_function(
                handshake_bytes, offset, read_int_value
            )

        (client_proxy["Version"], offset) = self.get_client_proxy_version_ordinal(
            handshake_bytes, offset
        )

        handshake_info["ClientProxy"] = client_proxy

        (handshake_info["ThisValueisAlways1"], offset) = call_reader_function(
            handshake_bytes, offset, read_int_value
        )

        (handshake_info["Overrides"], offset) = call_reader_function(
            handshake_bytes, offset, read_byte_value
        )

        (handshake_info["CredentialsTypeFlag"], offset) = self.get_credentials_type(
            handshake_bytes, offset
        )

    def process_line(self, line):
        handshake = {}
        if self.is_handshake_trace(line):
            self.get_handshake_info(line, handshake)
            self.output_queue_.put({"handshake": handshake})
