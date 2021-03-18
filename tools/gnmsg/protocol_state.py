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

from message_types import message_types
from read_values import read_int_value, read_byte_value, call_reader_function


class ProtocolState:
    def __init__(self):
        self.last_client_message_ = {}

    def get_last_client_message(self, thread_id):
        result = ""
        if thread_id in self.last_client_message_.keys():
            result = self.last_client_message_[thread_id]
        return result

    def set_last_client_message(self, thread_id, client_message):
        self.last_client_message_[thread_id] = client_message
