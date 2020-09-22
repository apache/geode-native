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
from enum import Enum


class ConnectionTypes(int, Enum):
    CLIENT_TO_SERVER = 100
    PRIMARY_SERVER_TO_CLIENT = 101
    SECONDARY_SERVER_TO_CLIENT = 102


ConnectionTypeStrings = {
    ConnectionTypes.CLIENT_TO_SERVER: "Client to server",
    ConnectionTypes.PRIMARY_SERVER_TO_CLIENT: "Primary server to client",
    ConnectionTypes.SECONDARY_SERVER_TO_CLIENT: "Secondary server to client",
}
