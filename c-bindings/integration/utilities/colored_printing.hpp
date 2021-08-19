/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "utilities/internal/colored_printing.hpp"

namespace util {
/* Include this header.

   These instances will print that little colored box on the left column, just
   like GoogleTest does, followed by your formatted text in the color specified.
   The instances are extern'd to reduce object bloat and compile time. You can
   find their definition and instantiation in the internal directory below.
*/
extern template void print_message<::testing::internal::COLOR_DEFAULT>(const char* fmt...);
extern template void print_message<::testing::internal::COLOR_GREEN>(const char* fmt...);
extern template void print_message<::testing::internal::COLOR_YELLOW>(const char* fmt...);
extern template void print_message<::testing::internal::COLOR_RED>(const char* fmt...);
}  // namespace util
