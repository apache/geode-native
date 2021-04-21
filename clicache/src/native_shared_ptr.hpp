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

#pragma once


#include "begin_native.hpp"
#include <memory>
#include "end_native.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      template <class _T>
      public ref class native_shared_ptr sealed {
      private:
        std::shared_ptr<_T>* ptr;

      public:
        native_shared_ptr(const std::shared_ptr<_T>& ptr) : ptr(new std::shared_ptr<_T>(ptr)) {}

        ~native_shared_ptr() {
          native_shared_ptr::!native_shared_ptr();
        }

        !native_shared_ptr() {
          delete ptr;
        }

        inline _T* get() {
          return ptr->get();
        }

        inline std::shared_ptr<_T> get_shared_ptr() {
          return *ptr;
        }

      };
    }
  }
}
