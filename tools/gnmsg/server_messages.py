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
from read_values import (
    call_reader_function,
    read_int_value,
    read_unsigned_byte_value,
    read_byte_value,
    read_cacheable,
    parse_key_or_value,
)
from numeric_conversion import decimal_string_to_hex_string
from gnmsg_globals import global_protocol_state


def read_bucket_count(message_bytes, offset):
    object_part = {}
    (object_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (object_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (object_part["Data"], offset) = read_cacheable(message_bytes, offset)

    return object_part, offset


def read_partition_attributes(properties, message_bytes, offset):
    (properties["BucketCount"], offset) = read_bucket_count(message_bytes, offset)
    (properties["ColocatedWith"], offset) = parse_key_or_value(message_bytes, offset)
    if properties["Parts"] == 4:
        (properties["PartitionResolverName"], offset) = parse_key_or_value(
            message_bytes, offset
        )
        # TODO: parse part 4 (list of partition attributes)
    elif properties["Parts"] == 3:
        try:
            (properties["PartitionResolverName"], offset) = parse_key_or_value(
                message_bytes, offset
            )
        except:
            raise Exception(
                "Don't know how to parse a RESPONSE_CLIENT_PARTITION_ATTRIBUTES message with "
                + "3 parts and fpa attribute."
            )
        # TODO: parse part 3 if it is not partition resolver but list of partition attributes


def read_object_header(message_bytes, offset):
    object_base = {}
    (object_base["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (object_base["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    return object_base, offset


def read_bytes_part(message_bytes, offset):
    bytes_part, offset = read_object_header(message_bytes, offset)

    bytes_string = ""
    for i in range(bytes_part["Size"]):
        if i:
            bytes_string += " "
        byte_val, offset = call_reader_function(message_bytes, offset, read_byte_value)
        bytes_string += decimal_string_to_hex_string(str(byte_val))
    bytes_part["Bytes"] = bytes_string
    return bytes_part, offset


def read_int_part(message_bytes, offset):
    int_part, offset = read_object_header(message_bytes, offset)

    int_value, offset = call_reader_function(message_bytes, offset, read_int_value)
    int_part["Value"] = decimal_string_to_hex_string(str(int_value))
    return int_part, offset


def read_old_value_part(message_bytes, offset):
    old_value_part, offset = read_object_header(message_bytes, offset)

    bytes_string = ""
    for i in range(old_value_part["Size"]):
        if i:
            bytes_string += " "
        byte_val, offset = call_reader_function(
            message_bytes, offset, read_unsigned_byte_value
        )
        bytes_string += decimal_string_to_hex_string(str(byte_val))
    old_value_part["Bytes"] = bytes_string
    return old_value_part, offset


def read_object_part(message_bytes, offset):
    object_part, offset = read_object_header(message_bytes, offset)

    offset += 2 * object_part["Size"]
    return object_part, offset


def read_put_reply(properties, message_bytes, offset):
    (properties["Bytes"], offset) = read_bytes_part(message_bytes, offset)
    (properties["Flags"], offset) = read_int_part(message_bytes, offset)
    if properties["Parts"] >= 3:
        (properties["OldValue"], offset) = read_old_value_part(message_bytes, offset)
        if properties["Parts"] == 4:
            (properties["VersionTag"], offset) = read_object_part(message_bytes, offset)


def read_contains_key_response(properties, message_bytes, offset):
    properties["Response"] = parse_key_or_value(message_bytes, offset)
    return properties, offset


server_message_parsers = {
    "RESPONSE_CLIENT_PARTITION_ATTRIBUTES": read_partition_attributes,
    "PUT_REPLY": read_put_reply,
    "CONTAINS_KEY_RESPONSE": read_contains_key_response,
}


def parse_server_message(properties, message_bytes):
    offset = 0
    if properties["Type"] in server_message_parsers.keys():
        server_message_parsers[properties["Type"]](properties, message_bytes, offset)
    else:
        key = (
            global_protocol_state.get_last_client_message(properties["tid"])
            + "_"
            + properties["Type"]
        )
        if key in server_message_parsers.keys():
            server_message_parsers[key](properties, message_bytes, offset)
