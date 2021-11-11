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

from message_types import message_types
from read_values import read_int_value, read_byte_value, call_reader_function

CHUNKED_MESSAGE_HEADER_LENGTH = 17


class ChunkedResponseDecoder:
    def __init__(self):
        self.reset()

    def add_header(self, connection, header):
        if len(self.chunked_message) > 0:
            raise Exception(
                "Previous chunked message is not completed, can't process another header"
            )

        if len(header) == 2 * CHUNKED_MESSAGE_HEADER_LENGTH:
            offset = 0
            message_type = ""
            (message_type, offset) = call_reader_function(
                header, offset, read_int_value
            )
            self.chunked_message["Type"] = message_types[message_type]
            # TODO: pass connection value in as a parameter
            self.chunked_message["Connection"] = connection
            self.chunked_message["Direction"] = "<---"
            (self.chunked_message["Parts"], offset) = call_reader_function(
                header, offset, read_int_value
            )
            (self.chunked_message["TransactionId"], offset) = call_reader_function(
                header, offset, read_int_value
            )
            chunk_size = 0
            flags = 0
            (chunk_size, offset) = call_reader_function(header, offset, read_int_value)
            (flags, offset) = call_reader_function(header, offset, read_byte_value)
            self.chunked_message["ChunkInfo"] = []
            self.add_chunk_header(chunk_size, flags)
        else:
            raise IndexError(
                "Chunked message header should be "
                + str(CHUNKED_MESSAGE_HEADER_LENGTH)
                + " bytes"
            )

    def add_chunk_header(self, chunk_size, flags):
        self.chunk_index += 1
        if len(self.chunked_message) == 0:
            raise Exception("Can't add chunk header before message header")

        key = "Chunk" + str(self.chunk_index)
        inner_item = dict(ChunkLength=int(chunk_size), Flags=flags)
        outer_item = {}
        outer_item[key] = inner_item
        self.chunked_message["ChunkInfo"].append(outer_item)
        self.chunk_flags = flags

    def add_chunk(self, chunk):
        if len(self.chunked_message) == 0:
            raise Exception("Can't add chunks before message header")

        self.message_body += chunk

    def is_complete_message(self):
        return self.chunk_flags & 0x1

    def get_decoded_message(self, time_stamp):
        # Return a re-ordered dictionary, with Timestamp at the front.  This
        # makes output consistent with other messages
        decoded_message = {"Timestamp": time_stamp}
        decoded_message.update(self.chunked_message)
        return decoded_message

    def reset(self):
        self.header = ""
        self.message_body = ""
        self.chunked_message = {}
        self.complete = False
        self.chunk_flags = 0xFF
        self.chunk_index = -1
