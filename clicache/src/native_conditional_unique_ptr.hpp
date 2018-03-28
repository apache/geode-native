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
      public ref class native_conditional_unique_ptr sealed {
      private:
        std::unique_ptr<_T>* owned_ptr;
        _T* unowned_ptr;

      public:
        native_conditional_unique_ptr(std::unique_ptr<_T> ptr) :
          owned_ptr(new std::unique_ptr<_T>(std::move(ptr))), 
          unowned_ptr(__nullptr) {}

        native_conditional_unique_ptr(_T* ptr) :
          owned_ptr(__nullptr),
          unowned_ptr(ptr) {}

        ~native_conditional_unique_ptr() {
          native_conditional_unique_ptr::!native_conditional_unique_ptr();
        }

        !native_conditional_unique_ptr() {
          delete owned_ptr;
        }

        inline _T* get() {
          return __nullptr == owned_ptr ? unowned_ptr : owned_ptr->get();
        }

        static inline _T& operator*(native_conditional_unique_ptr<_T>^ t) {
          return *(t->get());
        }
      };
    }
  }
}
