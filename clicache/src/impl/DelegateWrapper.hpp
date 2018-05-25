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


#include "../begin_native.hpp"
#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "../end_native.hpp"

#include "../geode_defs.hpp"
#include "../Serializable.hpp"
#include "ManagedCacheableKey.hpp"
#include "SafeConvert.hpp"
#include "../Log.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;
      /// <summary>
      /// Template class to wrap a managed <see cref="TypeFactoryMethod" />
      /// delegate that returns an <see cref="ISerializable" /> object. It contains
      /// a method that converts the managed object gotten by invoking the
      /// delegate to the native <c>apache::geode::client::Serializable</c> object
      /// (using the provided wrapper class constructor).
      /// </summary>
      /// <remarks>
      /// This class is to enable interopibility between the managed and unmanaged
      /// worlds when registering types.
      /// In the managed world a user would register a managed type by providing
      /// a factory delegate returning an object of that type. However, the
      /// native implementation requires a factory function that returns an
      /// object implementing <c>apache::geode::client::Serializable</c>. Normally this would not
      /// be possible since we require to dynamically generate a new function
      /// for a given delegate.
      ///
      /// Fortunately in the managed world the delegates contain an implicit
      /// 'this' pointer. Thus we can have a universal delegate that contains
      /// the given managed delegate (in the 'this' pointer) and returns the
      /// native <c>apache::geode::client::Serializable</c> object. Additionally marshalling
      /// services provide <c>Marshal.GetFunctionPointerForDelegate</c> which gives
      /// a function pointer for a delegate which completes the conversion.
      /// </remarks>
      ref class DelegateWrapperGeneric
      {
      public:

        /// <summary>
        /// Constructor to wrap the given managed delegate.
        /// </summary>
        inline DelegateWrapperGeneric( TypeFactoryMethod^ typeDelegate )
          : m_delegate( typeDelegate ) { }

        /// <summary>
        /// Returns the native <c>apache::geode::client::Serializable</c> object by invoking the
        /// managed delegate provided in the constructor.
        /// </summary>
        /// <returns>
        /// Native <c>apache::geode::client::Serializable</c> object after invoking the managed
        /// delegate and wrapping inside a <c>ManagedCacheableKey</c> object.
        /// </returns>
        std::shared_ptr<apache::geode::client::Serializable> NativeDelegateGeneric( )
        {
          auto tempObj = m_delegate( );
          if(auto tempDelta = dynamic_cast<IDelta^>(tempObj))
          {
            return std::shared_ptr<apache::geode::client::ManagedCacheableDeltaGeneric>(
              new apache::geode::client::ManagedCacheableDeltaGeneric(tempDelta));
          }
          else if (auto dataSerializable = dynamic_cast<IDataSerializable^>(tempObj))
          {
            return std::shared_ptr<apache::geode::client::ManagedCacheableKeyGeneric>(
              new apache::geode::client::ManagedCacheableKeyGeneric(dataSerializable));
          }
          else if (auto dataSerializablePrimitive = dynamic_cast<IDataSerializablePrimitive^>(tempObj))
          {
            return std::shared_ptr<apache::geode::client::ManagedDataSerializablePrimitive>(
              new apache::geode::client::ManagedDataSerializablePrimitive(dataSerializablePrimitive));
          }
          else if (auto dataSerializableFixedId = dynamic_cast<IDataSerializableFixedId^>(tempObj))
          {
            return std::shared_ptr<apache::geode::client::ManagedDataSerializableFixedId>(
              new apache::geode::client::ManagedDataSerializableFixedId(dataSerializableFixedId));
          }
          else if (auto dataSerializableInternal = dynamic_cast<IDataSerializableInternal^>(tempObj))
          {
            return std::shared_ptr<apache::geode::client::ManagedDataSerializableInternal>(
              new apache::geode::client::ManagedDataSerializableInternal(dataSerializableInternal));
          }

          throw native::IllegalStateException("Unknown serialization type.");
        }


      private:

        TypeFactoryMethod^ m_delegate;

      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
