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

      template <class _T, class _D = std::default_delete<_T>>
      public ref class native_unique_ptr sealed {
      private:
        std::unique_ptr<_T, _D>* ptr;

      public:
        native_unique_ptr(std::unique_ptr<_T, _D>&& ptr) :
          ptr(new std::unique_ptr<_T, _D>(ptr.release(), std::forward<_D>(ptr.get_deleter()))) {}

        ~native_unique_ptr() {
          native_unique_ptr::!native_unique_ptr();
        }

        !native_unique_ptr() {
          delete ptr;
        }

        inline _T* get() {
          return ptr->get();
        }

      };

      
      template <class _T, class _D>
      public ref class native_unique_ptr<_T[], _D> sealed {
      private:
        std::unique_ptr<_T[], _D>* ptr;

      public:
        native_unique_ptr(std::unique_ptr<_T[], _D>&& ptr) :
          ptr(new std::unique_ptr<_T[], _D>(ptr.release(), std::forward<_D>(ptr.get_deleter()))) {}

        native_unique_ptr(_T* ptr) :
          ptr(new std::unique_ptr<_T[], _D>(ptr)) {}

        ~native_unique_ptr() {
          native_unique_ptr::!native_unique_ptr();
        }

        !native_unique_ptr() {
          delete ptr;
        }

        inline _T* get() {
          return ptr->get();
        }

      };

      template<class _T, class... _Args,
        std::enable_if_t<!std::is_array_v<_T>, int> = 0>
      inline native_unique_ptr<_T>^ make_unique(_Args&&... args) {
        return gcnew native_unique_ptr<_T>(std::make_unique<_T>(std::forward<_Args>(args)...));
      }

      template <class _T, std::enable_if_t<std::is_array_v<_T> && std::extent_v<_T> == 0, int> = 0>
      inline native_unique_ptr<_T>^ make_native_unique(std::size_t size) {
        return gcnew native_unique_ptr<_T>(std::make_unique<_T>(size));
      }
      
    }
  }
}
