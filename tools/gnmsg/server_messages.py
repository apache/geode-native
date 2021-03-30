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
    parse_key_or_value,
    read_byte_value,
    read_cacheable,
    read_int_value,
    read_short_value,
    read_unsigned_byte_value,
    read_unsigned_vl,
    read_string_value,
)
from read_parts import read_object_header, read_object_part, read_int_part
from numeric_conversion import decimal_string_to_hex_string, int_to_hex_string
from gnmsg_globals import global_protocol_state
from enum import IntFlag


class VersionTagFlags(IntFlag):
    HAS_MEMBER_ID = 1
    HAS_PREVIOUS_MEMBER_ID = 2
    VERSION_TWO_BYTES = 4
    DUPLICATE_MEMBER_IDS = 8
    HAS_RVV_HIGH_BYTE = 16


class DestroyReplyFlags(IntFlag):
    HAS_VERSION_TAG = 1
    HAS_ENTRY_NOT_FOUND_PART = 2


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


def read_flags_part(message_bytes, offset):
    flags_part, offset = read_object_header(message_bytes, offset)
    int_value, offset = call_reader_function(message_bytes, offset, read_int_value)

    flags_part["Flags"] = int_to_hex_string(int_value) + " ("
    if int_value & DestroyReplyFlags.HAS_VERSION_TAG:
        flags_part["Flags"] += "HAS_VERSION_TAG"
    if int_value & DestroyReplyFlags.HAS_ENTRY_NOT_FOUND_PART:
        flags_part["Flags"] += " | HAS_ENTRY_NOT_FOUND_PART"
    flags_part["Flags"] += ")"

    return flags_part, offset, int_value


def version_flags_to_string(version_flags):
    version_flags_string = "("
    if version_flags & VersionTagFlags.HAS_MEMBER_ID:
        version_flags_string += "HAS_MEMBER_ID"
    if version_flags & VersionTagFlags.HAS_PREVIOUS_MEMBER_ID:
        version_flags_string += "| " if version_flags_string == ")" else ""
        version_flags_string += "HAS_PREVIOUS_MEMBER_ID"
    if version_flags & VersionTagFlags.VERSION_TWO_BYTES:
        version_flags_string += "| " if version_flags_string == ")" else ""
        version_flags_string += "VERSION_TWO_BYTES"
    if version_flags & VersionTagFlags.DUPLICATE_MEMBER_IDS:
        version_flags_string += "| " if version_flags_string == ")" else ""
        version_flags_string += "DUPLICATE_MEMBER_IDS"
    if version_flags & VersionTagFlags.HAS_RVV_HIGH_BYTE:
        version_flags_string += "| " if version_flags_string == ")" else ""
        version_flags_string += "HAS_RVV_HIGH_BYTE"
    version_flags_string += ")"
    return version_flags_string


def read_version_tag_part(message_bytes, offset):
    version_tag_part, offset = read_object_header(message_bytes, offset)
    cursor = offset
    version_flags, cursor = call_reader_function(
        message_bytes, cursor, read_short_value
    )

    version_tag_part["Flags"] = version_flags_to_string(version_flags)

    entry_version = 0
    if version_flags & VersionTagFlags.VERSION_TWO_BYTES:
        entry_version, cursor = call_reader_function(
            message_bytes, cursor, read_short_value
        )
    else:
        entry_version, cursor = call_reader_function(
            message_bytes, cursor, read_int_value
        )
    version_tag_part["EntryVersion"] = int_to_hex_string(entry_version)

    if version_flags & VersionTagFlags.HAS_RVV_HIGH_BYTE:
        region_version_high_bytes, cursor = call_reader_function(
            message_bytes, cursor, read_short_value
        )
        version_tag_part["RegionVersionHighBytes"] = int_to_hex_string(
            region_version_high_bytes
        )

    region_version_low_bytes, cursor = call_reader_function(
        message_bytes, cursor, read_int_value
    )

    timestamp, cursor = call_reader_function(message_bytes, cursor, read_unsigned_vl)
    version_tag_part["Timestamp"] = int_to_hex_string(timestamp)

    version_tag_part["RegionVersionLowBytes"] = int_to_hex_string(
        region_version_low_bytes
    )

    bytes_string = ""
    for i in range(0, offset + 2 * version_tag_part["Size"] - cursor, 2):
        if i:
            bytes_string += " "
        byte_val, cursor = call_reader_function(
            message_bytes, cursor, read_unsigned_byte_value
        )
        bytes_string += decimal_string_to_hex_string(str(byte_val))
    version_tag_part[
        "Un-decodable part (member id, [previous member id])"
    ] = bytes_string

    return version_tag_part, offset + 2 * version_tag_part["Size"]


def read_bytes_part(message_bytes, offset):
    bytes_part, offset = read_object_header(message_bytes, offset)
    cursor = offset
    bytes_string = ""
    for i in range(0, offset + 2 * bytes_part["Size"] - cursor, 2):
        if i:
            bytes_string += " "
        byte_val, cursor = call_reader_function(
            message_bytes, cursor, read_unsigned_byte_value
        )
        bytes_string += decimal_string_to_hex_string(str(byte_val))
    bytes_part["Bytes"] = bytes_string
    return bytes_part, cursor


def read_entry_not_found_part(message_bytes, offset):
    entry_not_found_part, offset = read_object_header(message_bytes, offset)
    int_value, offset = call_reader_function(message_bytes, offset, read_int_value)
    entry_not_found_part["EntryNotFound"] = "true" if int_value == 1 else "false"
    return entry_not_found_part, offset


def read_destroy_reply(properties, message_bytes, offset):
    (
        properties["Flags"],
        offset,
        flags,
    ) = read_flags_part(message_bytes, offset)

    if flags & DestroyReplyFlags.HAS_VERSION_TAG:
        properties["VersionTag"], offset = read_version_tag_part(message_bytes, offset)

    properties["OkBytes"], offset = read_bytes_part(message_bytes, offset)

    if flags & DestroyReplyFlags.HAS_ENTRY_NOT_FOUND_PART:
        properties["EntryNotFoundPart"], offset = read_entry_not_found_part(
            message_bytes, offset
        )


def read_exception_msg(properties, message_bytes, offset):
    (properties["SerializedJavaObjectPart"], offset) = read_object_part(
        message_bytes, offset
    )
    object_part, offset = read_object_header(message_bytes, offset)
    (object_part["ExceptionMessageAndCallstack"], offset) = read_string_value(
        message_bytes, object_part["Size"], offset
    )
    properties["StringRepresentationPart"] = object_part


server_message_parsers = {
    "CONTAINS_KEY_RESPONSE": read_contains_key_response,
    "DESTROY_REPLY": read_destroy_reply,
    "EXCEPTION": read_exception_msg,
    "PUT_REPLY": read_put_reply,
    "RESPONSE_CLIENT_PARTITION_ATTRIBUTES": read_partition_attributes,
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
