#pragma once

#include <memory>

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

      };
    }
  }
}
