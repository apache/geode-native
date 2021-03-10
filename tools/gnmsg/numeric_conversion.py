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
to_hex_digit = {
    0: "0",
    1: "1",
    2: "2",
    3: "3",
    4: "4",
    5: "5",
    6: "6",
    7: "7",
    8: "8",
    9: "9",
    10: "a",
    11: "b",
    12: "c",
    13: "d",
    14: "e",
    15: "f",
}


def int_to_hex_string(intval):
    byte3 = (intval & 0xFF000000) >> 24
    byte2 = (intval & 0xFF0000) >> 16
    byte1 = (intval & 0xFF00) >> 8
    byte0 = intval & 0xFF
    return (
        decimal_string_to_hex_string(byte3)
        + decimal_string_to_hex_string(byte2)
        + decimal_string_to_hex_string(byte1)
        + decimal_string_to_hex_string(byte0)
    )


def decimal_string_to_hex_string(byte):
    high_nibble = int(int(byte) / 16)
    low_nibble = int(byte) % 16
    return to_hex_digit[high_nibble] + to_hex_digit[low_nibble]
