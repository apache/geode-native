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