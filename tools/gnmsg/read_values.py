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
from modified_utf8 import utf8m_to_utf8s


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


def read_array_length(message_bytes, offset):
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
    return array_len, offset


def read_byte_array(string, offset):
    (array_length, offset) = read_array_length(string, offset)
    byte_string = ""
    for i in range(offset, offset + (array_length * 2), 2):
        byte_string += string[i : i + 2]
        byte_string += " "
    byte_string = byte_string[:-1]
    return byte_string, offset + (array_length * 2)


def read_boolean_value(message_bytes, offset):
    (bool_val, offset) = call_reader_function(message_bytes, offset, read_byte_value)
    return "True" if bool_val == 1 else "False", offset


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


def read_fixed_id_byte_value(string, offset):
    (ds_code, offset) = call_reader_function(string, offset, read_byte_value)
    if ds_codes[ds_code] == "FixedIDByte":
        (byte_value, offset) = call_reader_function(string, offset, read_byte_value)
    else:
        raise TypeError("Expected DSCode 'FixedIDByte'")

    return (byte_value, offset)


def read_cacheable_string_value(string, offset):
    (dscode, offset) = call_reader_function(string, offset, read_byte_value)
    string_type = ds_codes[dscode]
    if string_type == "CacheableString":
        (string_length, offset) = call_reader_function(string, offset, read_short_value)
        return read_geode_jmutf8_string_value(string, offset, string_length)
    elif string_type == "CacheableStringHuge":
        (string_length, offset) = call_reader_function(string, offset, read_int_value)
        offset += string_length
    else:
        raise TypeError("Expected CacheableString or CacheableStringHuge")


def read_cacheable_ascii_string_value(string, offset):
    (ds_code, offset) = call_reader_function(string, offset, read_byte_value)
    string_value = []
    if ds_codes[ds_code] == "CacheableASCIIString":
        (size, offset) = call_reader_function(string, offset, read_short_value)
        for i in range(size):
            (ascii_char, offset) = call_reader_function(string, offset, read_byte_value)
            string_value.append(ascii_char)
    else:
        raise TypeError("Attempt to decode another type as CacheableASCIIString")

    return (bytes(string_value).decode("ascii"), offset)


# Decodes a hex string to JM utf-8 bytes, returns plain utf-8 string
def read_geode_jmutf8_string_value(buffer, offset, string_length):
    cursor = offset
    string = []
    bad_length = IndexError("Insufficient length for JM utf-8 string")

    while cursor < string_length:
        code_point, cursor = call_reader_function(buffer, cursor, read_byte_value)
        if code_point == 0:
            raise TypeError("Should not encounter a 0 byte in JM utf-8")
        elif code_point < 0x7F:  # one-byte encoding
            string.append(code_point)
        elif (code_point & 0xE0) == 0xC0:  # two-byte encoding
            if cursor < string_length - 1:
                (byte2, cursor) = call_reader_function(buffer, cursor, read_byte_value)
                string.append(code_point)
                string.append(byte2)
            else:
                raise bad_length
        # 3-byte or 6-byte encoding.  We don't care which here, because we'll
        # just pick up the next 3-byte encoding in the loop, and the conversion
        # at the end will raise an exception if there's a problem.
        elif (code_point & 0xF0) == 0xE0:
            if cursor < string_length - 3:
                (byte2, cursor) = call_reader_function(buffer, cursor, read_byte_value)
                (byte3, cursor) = call_reader_function(buffer, cursor, read_byte_value)
                string.append(code_point)
                string.append(byte2)
                string.append(byte3)
            else:
                raise bad_length

    return (utf8m_to_utf8s(string), cursor)


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
