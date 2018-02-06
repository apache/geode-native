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
       public ref class native_conditional_shared_ptr sealed {
       private:
         std::shared_ptr<_T>* owned_ptr;
         _T* unowned_ptr;

       public:
         native_conditional_shared_ptr(const std::shared_ptr<_T>& ptr) : owned_ptr(new std::shared_ptr<_T>(ptr)), unowned_ptr(__nullptr) {}
         native_conditional_shared_ptr(_T* ptr) : owned_ptr(__nullptr), unowned_ptr(ptr) {}

         ~native_conditional_shared_ptr() {
           native_conditional_shared_ptr::!native_conditional_shared_ptr();
         }

         !native_conditional_shared_ptr() {
           delete owned_ptr;
         }

         inline _T* get() {
           return __nullptr == owned_ptr ? unowned_ptr : owned_ptr->get();
         }

         inline std::shared_ptr<_T> get_conditional_shared_ptr() {
           return owned_ptr ? *owned_ptr : nullptr;
         }

       };
     }
   }
 }