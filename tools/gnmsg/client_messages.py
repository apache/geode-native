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
import sys
import json

from read_values import (
    call_reader_function,
    parse_key_or_value,
    read_byte_value,
    read_cacheable,
    read_int_value,
    read_jmutf8_string_value,
    read_long_value,
    read_short_value,
    read_string_value,
    read_unsigned_byte_value,
)
from read_parts import read_object_header, read_int_part
from numeric_conversion import decimal_string_to_hex_string, int_to_hex_string

CHARS_IN_MESSAGE_HEADER = 34


def parse_region_part(message_bytes, offset):
    region_part = {}
    (region_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (region_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (region_part["Name"], offset) = read_string_value(
        message_bytes, region_part["Size"], offset
    )
    return (region_part, offset)


def parse_object_part(message_bytes, offset):
    object_part = {}
    (object_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (object_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    offset += 2 * object_part["Size"]
    return (object_part, offset)


def parse_event_id_part(message_bytes, offset):
    event_id_part = {}
    (event_id_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (event_id_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (event_id_part["LongCode1"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (event_id_part["EventIdThread"], offset) = call_reader_function(
        message_bytes, offset, read_long_value
    )
    (event_id_part["LongCode2"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (event_id_part["EventIdSequence"], offset) = call_reader_function(
        message_bytes, offset, read_long_value
    )
    return (event_id_part, offset)


def parse_operation_part(message_bytes, offset):
    operation_part = {}
    (operation_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (operation_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    if operation_part["Size"] != 1:
        raise Exception(
            "'Operation' part of PUT message should always be size 1 for native client"
        )
    operation_part["Data"], offset = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    return (operation_part, offset)


def parse_flags_part(message_bytes, offset):
    flags_part = {}
    (flags_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (flags_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    if flags_part["Size"] != 4:
        raise Exception(
            "'Flags' part of PUT message should always be size 4 for native client"
        )
    flags_part["Data"], offset = call_reader_function(
        message_bytes, offset, read_int_value
    )
    return (flags_part, offset)


def read_array_len(message_bytes, offset):
    (byte_value, offset) = call_reader_function(message_bytes, offset, read_byte_value)
    array_len = 0
    if byte_value == 255:
        raise Exception("Don't know how to handle len == -1 in serialized array!")
    elif byte_value == 254:
        (array_len, offset) = call_reader_function(
            message_bytes, offset, read_short_value
        )
    elif byte_value == 253:
        (array_len, offset) = call_reader_function(
            message_bytes, offset, read_int_value
        )
    else:
        array_len = byte_value
    return (array_len, offset)


def parse_credentials(message_bytes, offset):
    credentials = {}
    array_len = 0
    (credentials["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (credentials["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (array_len, offset) = read_array_len(message_bytes, offset)
    for i in range(0, array_len):
        (credentials["Key" + str(i)], offset) = read_cacheable(message_bytes, offset)
        (credentials["Value" + str(i)], offset) = read_cacheable(message_bytes, offset)
    return (credentials, offset)


def parse_raw_string_part(message_bytes, offset):
    string_part = {}
    (string_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (string_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (string_part["Value"], offset) = read_jmutf8_string_value(
        message_bytes, string_part["Size"], offset
    )
    return (string_part, offset)


def parse_raw_int_part(message_bytes, offset):
    int_part = {}
    (int_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (int_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (int_part["Value"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    return (int_part, offset)


def parse_raw_byte_part(message_bytes, offset):
    byte_part = {}
    (byte_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (byte_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (byte_part["Value"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    return (byte_part, offset)


def parse_raw_boolean_part(message_bytes, offset):
    bool_part = {}
    (bool_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (bool_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    bool_val = 0
    (bool_val, offset) = call_reader_function(message_bytes, offset, read_byte_value)
    bool_part["Value"] = "False" if bool_val == 0 else "True"
    return (bool_part, offset)


def parse_byte_and_timeout_part(message_bytes, offset):
    byte_and_timeout_part = {}
    (byte_and_timeout_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (byte_and_timeout_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    bool_val = 0
    (byte_and_timeout_part["Byte"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (byte_and_timeout_part["TimeoutMs"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    return (byte_and_timeout_part, offset)


def read_put_message(properties, message_bytes, offset):
    (properties["Region"], offset) = parse_region_part(message_bytes, offset)
    (properties["Operation"], offset) = parse_operation_part(message_bytes, offset)

    (properties["Flags"], offset) = parse_flags_part(message_bytes, offset)

    (properties["Key"], offset) = parse_key_or_value(message_bytes, offset)

    (properties["IsDelta"], offset) = parse_key_or_value(message_bytes, offset)

    (properties["Value"], offset) = parse_key_or_value(message_bytes, offset)

    (properties["EventId"], offset) = parse_event_id_part(message_bytes, offset)


def read_request_message(properties, message_bytes, offset):
    (properties["Region"], offset) = parse_region_part(message_bytes, offset)
    (properties["Key"], offset) = parse_key_or_value(message_bytes, offset)


def read_close_connection_message(properties, message_bytes, offset):
    object_part = {}
    (object_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    if object_part["Size"] != 1:
        raise Exception("CloseConnection message should be only one byte long!")

    (object_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (object_part["KeepAlive"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    properties["ObjectPart"] = object_part


def read_contains_key_message(properties, message_bytes, offset):
    (properties["RegionPart"], offset) = parse_region_part(message_bytes, offset)
    (properties["Key"], offset) = parse_key_or_value(message_bytes, offset)
    (request_type, offset) = parse_raw_int_part(message_bytes, offset)
    if request_type["Value"] == 1:
        properties["RequestType"] == "ContainsValueForKey"
    else:
        properties["RequestType"] = "ContainsKey"


def read_destroy_message(properties, message_bytes, offset):
    if properties["Parts"] > 5:
        raise Exception(
            "Parser can't handle callback argument in DESTROY message.  Send this log file to the dev team!"
        )
    (properties["Region"], offset) = parse_region_part(message_bytes, offset)
    (properties["Key"], offset) = parse_key_or_value(message_bytes, offset)
    (properties["ExpectedOldValue"], offset) = parse_key_or_value(message_bytes, offset)
    (properties["Operation"], offset) = parse_operation_part(message_bytes, offset)
    (properties["EventId"], offset) = parse_event_id_part(message_bytes, offset)


def read_get_client_partition_attributes_message(properties, message_bytes, offset):
    (properties["RegionPart"], offset) = parse_region_part(message_bytes, offset)
    for i in range(1, properties["Parts"]):
        (properties["ObjectPart" + str(i)], offset) = parse_object_part(
            message_bytes, offset
        )


def read_get_client_pr_metadata_message(properties, message_bytes, offset):
    (properties["RegionPart"], offset) = parse_region_part(message_bytes, offset)
    for i in range(1, properties["Parts"]):
        (properties["ObjectPart" + str(i)], offset) = parse_object_part(
            message_bytes, offset
        )


def read_query_message(properties, message_bytes, offset):
    (properties["Query"], offset) = parse_region_part(message_bytes, offset)
    (properties["EventId"], offset) = parse_event_id_part(message_bytes, offset)
    if properties["Parts"] == 3:
        timeout_part = {}
        (timeout_part["Size"], offset) = call_reader_function(
            message_bytes, offset, read_int_value
        )
        (timeout_part["IsObject"], offset) = call_reader_function(
            message_bytes, offset, read_byte_value
        )
        (timeout_part["TimeoutMs"], offset) = call_reader_function(
            message_bytes, offset, read_int_value
        )
        properties["Timeout"] = timeout_part


def read_user_credential_message(properties, message_bytes, offset):
    (properties["Credentials"], offset) = parse_credentials(message_bytes, offset)


def read_executecq_msg_type_message(properties, message_bytes, offset):
    (properties["CQName"], offset) = parse_raw_string_part(message_bytes, offset)
    (properties["QueryString"], offset) = parse_raw_string_part(message_bytes, offset)
    (properties["CqState"], offset) = parse_raw_int_part(message_bytes, offset)
    (properties["Durable"], offset) = parse_raw_boolean_part(message_bytes, offset)
    (properties["RegionDataPolicy"], offset) = parse_raw_byte_part(
        message_bytes, offset
    )


def read_executecq_with_ir_msg_type_message(properties, message_bytes, offset):
    if properties["Parts"] != 5:
        raise Exception(
            "Don't know how to parse a RESPONSE_CLIENT_PARTITION_ATTRIBUTES message with "
            + properties["Parts"]
            + " parts (should always have 5)."
        )
    (properties["CQName"], offset) = parse_raw_string_part(message_bytes, offset)
    (properties["QueryString"], offset) = parse_raw_string_part(message_bytes, offset)
    (properties["CqState"], offset) = parse_raw_int_part(message_bytes, offset)
    (properties["Durable"], offset) = parse_raw_boolean_part(message_bytes, offset)
    (properties["RegionDataPolicy"], offset) = parse_raw_byte_part(
        message_bytes, offset
    )


def read_stopcq_or_closecq_msg_type_message(properties, message_bytes, offset):
    (properties["Region"], offset) = parse_region_part(message_bytes, offset)
    (properties["EventId"], offset) = parse_event_id_part(message_bytes, offset)
    if properties["Parts"] == 3:
        timeout_part = {}
        (timeout_part["Size"], offset) = call_reader_function(
            message_bytes, offset, read_int_value
        )
        (timeout_part["IsObject"], offset) = call_reader_function(
            message_bytes, offset, read_byte_value
        )
        (timeout_part["TimeoutMs"], offset) = call_reader_function(
            message_bytes, offset, read_int_value
        )
        properties["Timeout"] = timeout_part


def read_get_pdx_id_for_type_message(properties, message_bytes, offset):
    (properties["PdxType"], offset) = parse_object_part(message_bytes, offset)


def read_get_function_attributes_message(properties, message_bytes, offset):
    (properties["FunctionName"], offset) = parse_region_part(message_bytes, offset)


def read_execute_function_message(properties, message_bytes, offset):
    (properties["ByteAndTimeout"], offset) = parse_byte_and_timeout_part(
        message_bytes, offset
    )
    (properties["FunctionName"], offset) = parse_region_part(message_bytes, offset)
    (properties["Arguments"], offset) = parse_object_part(message_bytes, offset)


def parse_getall_optional_callback_arguments(message_bytes, offset):
    (local_object, local_offset) = parse_object_part(message_bytes, offset)
    if local_object["IsObject"] == 0:
        (local_object, local_offset) = parse_raw_int_part(message_bytes, offset)
    return (local_object, local_offset)


def read_get_all_70_message(properties, message_bytes, offset):
    (properties["Region"], offset) = parse_region_part(message_bytes, offset)
    (properties["KeyList"], offset) = parse_key_or_value(message_bytes, offset)
    (
        properties["CallbackArguments"],
        offset,
    ) = parse_getall_optional_callback_arguments(message_bytes, offset)


def read_key_set(properties, message_bytes, offset):
    (properties["Region"], offset) = parse_region_part(message_bytes, offset)


def read_object_as_raw_bytes(message_bytes, offset):
    raw_bytes_part, offset = read_object_header(message_bytes, offset)

    bytes_string = ""
    for i in range(raw_bytes_part["Size"]):
        if i:
            bytes_string += " "
        byte_val, offset = call_reader_function(
            message_bytes, offset, read_unsigned_byte_value
        )
        bytes_string += decimal_string_to_hex_string(str(byte_val))
    raw_bytes_part["Bytes"] = bytes_string
    return raw_bytes_part, offset


def read_add_pdx_type_message(properties, message_bytes, offset):
    properties["PdxType"], offset = read_object_as_raw_bytes(message_bytes, offset)
    properties["TypeId"], offset = read_int_part(message_bytes, offset)


client_message_parsers = {
    "ADD_PDX_TYPE": read_add_pdx_type_message,
    "CLOSECQ_MSG_TYPE": read_stopcq_or_closecq_msg_type_message,
    "CLOSE_CONNECTION": read_close_connection_message,
    "CONTAINS_KEY": read_contains_key_message,
    "DESTROY": read_destroy_message,
    "EXECUTECQ_MSG_TYPE": read_executecq_msg_type_message,
    "EXECUTECQ_WITH_IR_MSG_TYPE": read_executecq_with_ir_msg_type_message,
    "EXECUTE_FUNCTION": read_execute_function_message,
    "GET_ALL_70": read_get_all_70_message,
    "GET_CLIENT_PARTITION_ATTRIBUTES": read_get_client_partition_attributes_message,
    "GET_CLIENT_PR_METADATA": read_get_client_pr_metadata_message,
    "GET_FUNCTION_ATTRIBUTES": read_get_function_attributes_message,
    "GET_PDX_ID_FOR_TYPE": read_get_pdx_id_for_type_message,
    "KEY_SET": read_key_set,
    "PUT": read_put_message,
    "QUERY": read_query_message,
    "REQUEST": read_request_message,
    "STOPCQ_MSG_TYPE": read_stopcq_or_closecq_msg_type_message,
    "USER_CREDENTIAL_MESSAGE": read_user_credential_message,
}


def parse_client_message(properties, message_bytes):
    offset = CHARS_IN_MESSAGE_HEADER
    if properties["Type"] in client_message_parsers.keys():
        try:
            client_message_parsers[properties["Type"]](
                properties, message_bytes, offset
            )
        except:
            properties["ERROR"] = "Exception reading message - probably incomplete"
            return
