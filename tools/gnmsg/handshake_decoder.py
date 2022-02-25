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

from dateutil import parser

from decoder_base import DecoderBase
from ds_codes import ds_codes
from ds_fids import ds_fids
from modified_utf8 import utf8m_to_utf8s
from connection_types import ConnectionTypes, ConnectionTypeStrings

from read_values import (
    call_reader_function,
    read_boolean_value,
    read_byte_array,
    read_byte_value,
    read_cacheable_ascii_string_value,
    read_fixed_id_byte_value,
    read_geode_jmutf8_string_value,
    read_int_value,
    read_short_value,
    read_cacheable_string_value,
    read_unsigned_byte_value,
)

# TODO: Find a more reasonable place for this and other REPLY_* constants
REPLY_SSL_ENABLED = 21


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
        self.client_connection_request_expression_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).*([\d|a-f|A-F|x|X]+) .*\]\s*ThinClientLocatorHelper::sendRequest\([0-9|a-f|A-F|x|X]+\): sending \d+ bytes to locator:\s*([0-9|a-f|A-F]+)"
        )
        self.client_connection_response_expression_ = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).*([\d|a-f|A-F|x|X]+) .*\]\s*ThinClientLocatorHelper::sendRequest\([0-9|a-f|A-F|x|X]+\): received \d+ bytes from locator:\s*([0-9|a-f|A-F]+)"
        )
        self.server_handshake_expression_ = expression = re.compile(
            r"(\d\d\d\d\/\d\d\/\d\d \d\d:\d\d:\d\d\.\d+).* ([\d]+) .*\]\s*Handshake bytes: \(\d+\):\s*([0-9|a-f|A-F]+)"
        )

    def is_candidate_line(self, line):
        return "Helper::sendR" in line or "ake bytes:" in line

    def is_locator_request(self, line):
        match = self.client_connection_request_expression_.search(line)
        if match:
            return True
        else:
            return False

    def get_locator_request_parts(self, line, parts):
        result = False
        match = self.client_connection_request_expression_.search(line)
        if match:
            parts.append(parser.parse(match.group(1)))
            parts.append(match.group(2))
            parts.append(match.group(3))
            result = True

        return result

    def is_locator_response(self, line):
        match = self.client_connection_response_expression_.search(line)
        if match:
            return True
        else:
            return False

    def get_locator_response_parts(self, line, parts):
        result = False
        match = self.client_connection_response_expression_.search(line)
        if match:
            parts.append(parser.parse(match.group(1)))
            parts.append(match.group(2))
            parts.append(match.group(3))
            result = True

        return result

    def is_server_handshake_trace(self, line):
        match = self.server_handshake_expression_.search(line)
        if match:
            return True
        else:
            return False

    def get_server_handshake_parts(self, line, parts):
        match = self.server_handshake_expression_.search(line)
        if match:
            parts.append(parser.parse(match.group(1)))
            parts.append(match.group(2))
            parts.append(match.group(3))
        else:
            exit(1)

    # TODO: Find a handshake that uses a list and implement this function to parse it
    def read_list_of_ports(self, string, offset):
        (number_of_ports, offset) = call_reader_function(string, offset, read_int_value)
        ports = []
        for i in range(number_of_ports):
            (port, offset) = call_reader_function(string, offset, read_int_value)
            ports.append(port)

        return (ports, offset)

    def get_client_proxy_address(self, string, offset):
        (address_size, offset) = call_reader_function(string, offset, read_byte_value)
        result = ""

        if int(address_size) == 4:
            (octet1, offset) = call_reader_function(
                string, offset, read_unsigned_byte_value
            )
            (octet2, offset) = call_reader_function(
                string, offset, read_unsigned_byte_value
            )
            (octet3, offset) = call_reader_function(
                string, offset, read_unsigned_byte_value
            )
            (octet4, offset) = call_reader_function(
                string, offset, read_unsigned_byte_value
            )
            result = (
                str(octet1) + "." + str(octet2) + "." + str(octet3) + "." + str(octet4)
            )
        else:
            # don't really know what these bytes represent, so we'll just do a hex dump.
            for i in range(offset, offset + (address_size * 2), 2):
                result += string[i : i + 2]
                result += " "
            result = result[:-1]
            offset += address_size * 2

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
            )
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

    def get_server_handshake_info(self, line, handshake_info):
        parts = []
        self.get_server_handshake_parts(line, parts)
        handshake_info["Timestamp"] = parts[0]
        handshake_info["tid"] = parts[1]
        handshake_info["Direction"] = "--->"
        handshake_info["Type"] = "ServerHandshake"
        handshake_bytes = parts[2]

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
        if connection_type == ConnectionTypes.CLIENT_TO_SERVER:
            (handshake_info["ReadTimeout"], offset) = call_reader_function(
                handshake_bytes, offset, read_int_value
            )
        else:
            (ports, offset) = self.read_list_of_ports(handshake_bytes, offset)
            handshake_info["Ports"] = ports

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

    def read_client_connection_request(self, request_bytes, offset):
        client_connection_request = {}
        server_group = {}
        (server_group["name"], offset) = read_cacheable_string_value(
            request_bytes, offset
        )
        client_connection_request["ServerGroup"] = server_group
        (server_location_count, offset) = call_reader_function(
            request_bytes, offset, read_int_value
        )
        client_connection_request["ServerLocationCount"] = server_location_count
        server_locations = {}
        for i in range(client_connection_request["ServerLocationCount"]):
            (server_location, offset) = self.read_server_location(request_bytes, offset)
            server_locations["Server" + str(i)] = server_location
        client_connection_request["ServerLocations"] = server_locations
        return client_connection_request, offset

    def read_queue_connection_request(self, request_bytes, offset):
        queue_connection_request = {}
        server_group = {}
        (server_group["name"], offset) = read_cacheable_string_value(
            request_bytes, offset
        )
        queue_connection_request["servergroup"] = server_group
        (ds_fid, offset) = read_fixed_id_byte_value(request_bytes, offset)
        if ds_fids[ds_fid] != "ClientProxyMembershipId":
            raise TypeError("Expected type ClientProxyMembershipId")
        (
            queue_connection_request["ClientProxyMembershipId"],
            offset,
        ) = read_byte_array(request_bytes, offset)
        (
            queue_connection_request["ThisValueIsAlways1"],
            offset,
        ) = call_reader_function(request_bytes, offset, read_int_value)
        (
            queue_connection_request["RedundantCopies"],
            offset,
        ) = call_reader_function(request_bytes, offset, read_int_value)
        (
            queue_connection_request["ServerLocationCount"],
            offset,
        ) = call_reader_function(request_bytes, offset, read_int_value)
        server_locations = {}
        for i in range(queue_connection_request["ServerLocationCount"]):
            (server_location, offset) = self.read_server_location(request_bytes, offset)
            server_locations["Server" + str(i)] = server_location
        queue_connection_request["ServerLocations"] = server_locations
        (
            queue_connection_request["FindDurable"],
            offset,
        ) = call_reader_function(request_bytes, offset, read_boolean_value)
        return queue_connection_request, offset

    def read_locator_list_request(self, request_bytes, offset):
        locator_list_request = {}
        server_group = {}
        (server_group["name"], offset) = read_cacheable_string_value(
            request_bytes, offset
        )
        locator_list_request["servergroup"] = server_group
        return locator_list_request, offset

    def read_server_location(self, line, offset):
        server_location = {}
        (server_location["hostname"], offset) = read_cacheable_ascii_string_value(
            line, offset
        )
        (server_location["port"], offset) = call_reader_function(
            line, offset, read_int_value
        )

        return server_location, offset

    def read_locator_location(self, line, offset):
        return self.read_server_location(line, offset)

    def read_client_connection_response(self, response_bytes, offset):
        client_connection_response = {}
        (server_found, offset) = call_reader_function(
            response_bytes, offset, read_byte_value
        )
        client_connection_response["ServerFound"] = (
            "True" if server_found == 1 else "False"
        )

        if server_found == 1:
            (
                client_connection_response["ServerLocation"],
                offset,
            ) = self.read_server_location(response_bytes, offset)
        return client_connection_response, offset

    def read_queue_connection_response(self, response_bytes, offset):
        queue_connection_response = {}

        (
            queue_connection_response["DurableQueueFound"],
            offset,
        ) = read_boolean_value(response_bytes, offset)

        (
            queue_connection_response["ServerLocationCount"],
            offset,
        ) = call_reader_function(response_bytes, offset, read_int_value)

        server_locations = {}
        for i in range(queue_connection_response["ServerLocationCount"]):
            (server_location, offset) = self.read_server_location(
                response_bytes, offset
            )
            server_locations["Server" + str(i)] = server_location
        queue_connection_response["ServerLocations"] = server_locations
        return queue_connection_response, offset

    def read_locator_list_response(self, response_bytes, offset):
        locator_list_response = {}
        (
            locator_list_response["LocatorLocationCount"],
            offset,
        ) = call_reader_function(response_bytes, offset, read_int_value)
        locator_locations = {}
        for i in range(locator_list_response["LocatorLocationCount"]):
            (locator_location, offset) = self.read_locator_location(
                response_bytes, offset
            )
            locator_locations["Locator" + str(i)] = locator_location
        locator_list_response["LocatorLocations"] = locator_locations
        return locator_list_response, offset

    def decode_locator_request(self, line, handshake_request):
        parts = []
        if self.get_locator_request_parts(line, parts):
            offset = 0
            handshake_request["Timestamp"] = parts[0]
            handshake_request["tid"] = parts[1]
            handshake_request["Direction"] = "--->"
            request_bytes = parts[2]

            (handshake_request["GossipVersion"], offset) = call_reader_function(
                request_bytes, offset, read_int_value
            )
            (handshake_request["ProtocolOrdinal"], offset) = call_reader_function(
                request_bytes, offset, read_short_value
            )

            (ds_code, offset) = call_reader_function(
                request_bytes, offset, read_byte_value
            )

            (dsfid, offset) = call_reader_function(
                request_bytes, offset, read_byte_value
            )
            request_type = ds_fids[dsfid]
            handshake_request["Type"] = request_type
            if request_type == "ClientConnectionRequest":
                (
                    handshake_request["ClientConnectionRequest"],
                    offset,
                ) = self.read_client_connection_request(request_bytes, offset)

            elif request_type == "QueueConnectionRequest":
                (
                    handshake_request["QueueConnectionRequest"],
                    offset,
                ) = self.read_queue_connection_request(request_bytes, offset)

            elif request_type == "LocatorListRequest":
                (
                    handshake_request["LocatorListRequest"],
                    offset,
                ) = self.read_locator_list_request(request_bytes, offset)
            else:
                pass
                # TODO: decode other request types (locator list, server list, ...)

    def decode_locator_response(self, line, handshake_response):
        parts = []
        if self.get_locator_response_parts(line, parts):
            handshake_response["Timestamp"] = parts[0]
            handshake_response["tid"] = parts[1]
            handshake_response["Direction"] = "<---"
            response_bytes = parts[2]
            offset = 0

            (ssl_enabled, offset) = call_reader_function(
                response_bytes, offset, read_byte_value
            )
            if ssl_enabled == REPLY_SSL_ENABLED:
                handshake_response["SSLEnabled"] = "True"
            else:
                handshake_response["SSLEnabled"] = "False"
                offset = 0

            (fixed_id, offset) = read_fixed_id_byte_value(response_bytes, offset)
            response_type = ds_fids[fixed_id]
            handshake_response["Type"] = response_type
            if response_type == "ClientConnectionResponse":
                (
                    handshake_response["ClientConnectionResponse"],
                    offset,
                ) = self.read_client_connection_response(response_bytes, offset)
            elif response_type == "QueueConnectionResponse":
                (
                    handshake_response["QueueConnectionResponse"],
                    offset,
                ) = self.read_queue_connection_response(response_bytes, offset)
            elif response_type == "LocatorListResponse":
                (
                    handshake_response["LocatorListResponse"],
                    offset,
                ) = self.read_locator_list_response(response_bytes, offset)
            else:
                pass
                # TODO: decode other response types

    def process_line(self, line):
        if not self.is_candidate_line(line):
            return

        handshake = {}
        if self.is_locator_request(line):
            self.decode_locator_request(line, handshake)
            self.output_queue_.put({"handshake": handshake})
        elif self.is_locator_response(line):
            self.decode_locator_response(line, handshake)
            self.output_queue_.put({"handshake": handshake})
        elif self.is_server_handshake_trace(line):
            self.get_server_handshake_info(line, handshake)
            self.output_queue_.put({"handshake": handshake})
