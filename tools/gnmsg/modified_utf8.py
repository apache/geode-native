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
# translated from: http://hg.openjdk.java.net/jdk8/jdk8/jdk/file/94cc251d0c45/src/share/npt/utf.c
# Source:  https://gist.github.com/BarelyAliveMau5/000e7e453b6d4ebd0cb06f39bc2e7aec


def utf8s_to_utf8m(string):
    """
    :param string: utf8 encoded string
    :return: modified utf8 encoded string
    """
    new_str = []
    i = 0
    while i < len(string):
        byte1 = string[i]
        # NULL bytes and bytes starting with 11110xxx are special
        if (byte1 & 0x80) == 0:
            if byte1 == 0:
                new_str.append(0xC0)
                new_str.append(0x80)
            else:
                # Single byte
                new_str.append(byte1)

        elif (byte1 & 0xE0) == 0xC0:  # 2byte encoding
            new_str.append(byte1)
            i += 1
            new_str.append(string[i])

        elif (byte1 & 0xF0) == 0xE0:  # 3byte encoding
            new_str.append(byte1)
            i += 1
            new_str.append(string[i])
            i += 1
            new_str.append(string[i])

        elif (byte1 & 0xF8) == 0xF0:  # 4byte encoding
            # Beginning of 4byte encoding, turn into 2 3byte encodings
            # Bits in: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            i += 1
            byte2 = string[i]
            i += 1
            byte3 = string[i]
            i += 1
            byte4 = string[i]

            # Reconstruct full 21bit value
            u21 = (byte1 & 0x07) << 18
            u21 += (byte2 & 0x3F) << 12
            u21 += (byte3 & 0x3F) << 6
            u21 += byte4 & 0x3F

            # Bits out: 11101101 1010xxxx 10xxxxxx
            new_str.append(0xED)
            new_str.append((0xA0 + (((u21 >> 16) - 1) & 0x0F)))
            new_str.append((0x80 + ((u21 >> 10) & 0x3F)))

            # Bits out: 11101101 1011xxxx 10xxxxxx
            new_str.append(0xED)
            new_str.append((0xB0 + ((u21 >> 6) & 0x0F)))
            new_str.append(byte4)
        i += 1
    return bytes(new_str)


def utf8m_to_utf8s(string):
    """
    :param string: modified utf8 encoded string
    :return: utf8 encoded string
    """
    new_string = []
    length = len(string)
    i = 0
    while i < length:
        byte1 = string[i]
        if (byte1 & 0x80) == 0:  # 1byte encoding
            new_string.append(byte1)
        elif (byte1 & 0xE0) == 0xC0:  # 2byte encoding
            i += 1
            byte2 = string[i]
            if byte1 != 0xC0 or byte2 != 0x80:
                new_string.append(byte1)
                new_string.append(byte2)
            else:
                new_string.append(0)
        elif (byte1 & 0xF0) == 0xE0:  # 3byte encoding
            i += 1
            byte2 = string[i]
            i += 1
            byte3 = string[i]
            if i + 3 < length and byte1 == 0xED and (byte2 & 0xF0) == 0xA0:
                # See if this is a pair of 3byte encodings
                byte4 = string[i + 1]
                byte5 = string[i + 2]
                byte6 = string[i + 3]
                if byte4 == 0xED and (byte5 & 0xF0) == 0xB0:
                    # Bits in: 11101101 1010xxxx 10xxxxxx
                    # Bits in: 11101101 1011xxxx 10xxxxxx
                    i += 3

                    # Reconstruct 21 bit code
                    u21 = ((byte2 & 0x0F) + 1) << 16
                    u21 += (byte3 & 0x3F) << 10
                    u21 += (byte5 & 0x0F) << 6
                    u21 += byte6 & 0x3F

                    # Bits out: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

                    # Convert to 4byte encoding
                    new_string.append(0xF0 + ((u21 >> 18) & 0x07))
                    new_string.append(0x80 + ((u21 >> 12) & 0x3F))
                    new_string.append(0x80 + ((u21 >> 6) & 0x3F))
                    new_string.append(0x80 + (u21 & 0x3F))
                    continue
            new_string.append(byte1)
            new_string.append(byte2)
            new_string.append(byte3)
        i += 1
    return bytes(new_string)
