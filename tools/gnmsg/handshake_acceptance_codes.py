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
server_handshake_reply_codes = {
    21: "REPLY_SSL_ENABLED",
    59: "REPLY_OK",
    60: "REPLY_REFUSED",
    61: "REPLY_INVALID",
    62: "REPLY_AUTHENTICATION_REQUIRED",
    63: "REPLY_AUTHENTICATION_FAILED",
    64: "REPLY_DUPLICATE_DURABLE_CLIENT",
    105: "SUCCESSFUL_SERVER_TO_CLIENT",
    106: "UNSUCCESSFUL_SERVER_TO_CLIENT",
}
