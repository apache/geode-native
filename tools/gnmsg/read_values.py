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
from ds_codes import ds_codes


def read_number_from_hex_string(string, offset, size):
    value = int(string[offset : offset + size], 16)
    bits = size * 4
    if value & (1 << (bits - 1)):
        value -= 1 << bits
    return value, size


def read_unsigned_number_from_hex_string(string, offset, size):
    value = int(string[offset : offset + size], 16)
    bits = size * 4
    return value, size


def read_byte_value(string, offset):
    return read_number_from_hex_string(string, offset, 2)


def read_unsigned_byte_value(string, offset):
    return read_unsigned_number_from_hex_string(string, offset, 2)


def read_short_value(string, offset):
    return read_number_from_hex_string(string, offset, 4)


def read_int_value(string, offset):
    return read_number_from_hex_string(string, offset, 8)


def read_long_value(string, offset):
    return read_number_from_hex_string(string, offset, 16)


def read_unsigned_vl(string, offset):
    shift = 0
    result = 0
    cursor = offset

    while shift < 64:
        b, cursor = call_reader_function(string, cursor, read_byte_value)

        result |= (b & 0x7F) << shift
        if not (b & 0x80):
            break
        shift += 7

    if shift >= 64:
        raise ValueError("Malformed variable length integer")
    return result, cursor - offset


def read_string_value(string, length, offset):
    string_value = bytearray.fromhex(string[offset : offset + (length * 2)]).decode(
        "utf-8"
    )
    return (string_value, offset + (length * 2))


def read_jmutf8_string_value(string, length, offset):
    # TODO: Read Java Modified utf-8 string from bytes.  Cheating is okay for
    # now, cause it's super unlikely I'll hit a string where it makes a
    # difference
    return read_string_value(string, length, offset)


def call_reader_function(string, offset, fn):
    (value, read_count) = fn(string, offset)
    return (value, offset + read_count)


def read_cacheable(message_bytes, offset):
    value = {}
    dscode = ""
    (dscode, offset) = call_reader_function(message_bytes, offset, read_byte_value)
    value["DSCode"] = ds_codes[dscode]
    if (
        value["DSCode"] == "CacheableASCIIString"
        or value["DSCode"] == "CacheableASCIIStringHuge"
    ):
        (value["StringLength"], offset) = call_reader_function(
            message_bytes, offset, read_short_value
        )
        (value["Value"], offset) = read_string_value(
            message_bytes, value["StringLength"], offset
        )
    elif value["DSCode"] == "CacheableBoolean":
        (bool_val, offset) = call_reader_function(
            message_bytes, offset, read_byte_value
        )
        value["Value"] = "False" if bool_val == 0 else "True"
    elif value["DSCode"] == "CacheableInt32":
        (int_val, offset) = call_reader_function(message_bytes, offset, read_int_value)
        value["Value"] = int_val
    elif value["DSCode"] == "NullObj":
        # Gah!  Nasty little bug in the protocol here.  NC writes '1' in the
        # size field for a NullObj, but the payload is actually ZERO bytes,
        # and if you read 1 byte of payload like it says to you'll blow the
        # message parse.
        value["Value"] = "<<null>>"
    elif value["DSCode"] == "PDX":
        value["Value"] = "<<Unreadable - no type info available in gnmsg>>"
        # This is here for completion, but not actually necessary.
        offset = len(message_bytes)
    else:
        raise Exception("Unknown DSCode")

    return (value, offset)


def parse_key_or_value(message_bytes, offset):
    value = {}
    (value["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (value["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    try:
        (value["Data"], offset) = read_cacheable(message_bytes, offset)
    except:
        offset += value["Size"] * 2

    return (value, offset)
