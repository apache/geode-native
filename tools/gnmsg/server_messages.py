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
    read_byte_value,
    read_cacheable,
    parse_key_or_value,
)


def read_bucket_count(message_bytes, offset):
    object_part = {}
    (object_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (object_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (object_part["Data"], offset) = read_cacheable(message_bytes, offset)

    return (object_part, offset)


def read_partition_attributes(properties, message_bytes, offset):
    (properties["BucketCount"], offset) = read_bucket_count(message_bytes, offset)
    (properties["ColocatedWith"], offset) = parse_key_or_value(message_bytes, offset)
    if properties["Parts"] == 4:
        (properties["PartitionResolverName"], offset) = parse_key_or_value(message_bytes, offset)
        # TODO: parse part 4 (list of partition attributes)
    elif properties["Parts"] == 3:
        try:
            (properties["PartitionResolverName"], offset) = parse_key_or_value(message_bytes, offset)
        except:
            raise Exception(
                "Don't know how to parse a RESPONSE_CLIENT_PARTITION_ATTRIBUTES message with "
                + "3 parts and fpa attribute."
            )
        # TODO: parse part 3 if it is not partition resolver but list of partition attributes


server_message_parsers = {
    "RESPONSE_CLIENT_PARTITION_ATTRIBUTES": read_partition_attributes,
}


def parse_server_message(properties, message_bytes):
    offset = 0
    if properties["Type"] in server_message_parsers.keys():
        server_message_parsers[properties["Type"]](properties, message_bytes, offset)
