#pragma once

#include <memory>

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      template <class _T>
      public ref class native_unique_ptr sealed {
      private:
        std::unique_ptr<_T>* ptr;

      public:
        native_unique_ptr(std::unique_ptr<_T> ptr) : ptr(new std::unique_ptr<_T>(std::move(ptr))) {}

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
    }
  }
}
