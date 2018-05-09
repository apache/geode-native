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


#include "../geode_defs.hpp"

#include "../begin_native.hpp"
#include "CacheImpl.hpp"
#include "../end_native.hpp"

#include "ManagedCacheableKey.hpp"
#include "ManagedCacheableDelta.hpp"
#include "../Serializable.hpp"
#include "../Log.hpp"
#include "../CacheableKey.hpp"
#include "../CqEvent.hpp"
#include "PdxManagedCacheableKey.hpp"
#include "PdxWrapper.hpp"
//TODO::split
#include "../CqEvent.hpp"
#include "../UserFunctionExecutionException.hpp"
#include "../Cache.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

			interface class IPdxSerializable;
      public ref class SafeConvertClassGeneric
      {
      };

      /// <summary>
      /// Helper function to convert native <c>apache::geode::client::Serializable</c> object
      /// to managed <see cref="IGeodeSerializable" /> object.
      /// </summary>
      inline static Apache::Geode::Client::IGeodeSerializable^
        SafeUMSerializableConvertGeneric(std::shared_ptr<native::Serializable> serializableObject)
      {
        if (serializableObject == nullptr) return nullptr;

        if (auto mg_obj = std::dynamic_pointer_cast<native::ManagedCacheableKeyGeneric>(serializableObject))
        {
          return mg_obj->ptr();
        }

        if (auto mg_obj_delta = std::dynamic_pointer_cast<native::ManagedCacheableKeyGeneric>(serializableObject))
        {
          return dynamic_cast<Apache::Geode::Client::IGeodeSerializable^>(mg_obj_delta->ptr());
        }

        if (auto mg_UFEEobj = std::dynamic_pointer_cast<native::UserFunctionExecutionException>(serializableObject))
        {
          return gcnew UserFunctionExecutionException(mg_UFEEobj);
        }

        if (auto primitive = std::dynamic_pointer_cast<native::DataSerializablePrimitive>(serializableObject)) {
          auto wrapperMethod = TypeRegistry::GetWrapperGeneric(primitive->getDsCode());             
          if (wrapperMethod != nullptr)
          {
            return wrapperMethod(primitive);
          }
        }

        return gcnew Apache::Geode::Client::Serializable( serializableObject );
      }

      /// <summary>
      /// This function is to safely cast objects from managed class to native class.
      /// </summary>
      /// <remarks>
      /// <para>
      /// Consider the scenario that we have both native objects of class
      /// <c>native::Serializable</c> and managed objects of class
      /// <see cref="IGeodeSerializable" /> in a Region.
      /// </para><para>
      /// The former would be passed wrapped inside the
      /// <see cref="Serializable" /> class.
      /// When this object is passed to native methods, it would be wrapped
      /// inside <c>ManagedSerializable</c> class. However, for the
      /// former case it will result in double wrapping and loss of information
      /// (since the <c>ManagedSerializable</c> would not be as rich as the
      /// original native class). So for the former case we will directly
      /// get the native object, while we need to wrap only for the latter case.
      /// </para><para>
      /// This template function does a dynamic_cast to check if the object is of
      /// the given <c>NativeWrapper</c> type and if so, then simply return the
      /// native object else create a new object that wraps the managed object.
      /// </para>
      /// </remarks>
      template<typename ManagedType, typename ManagedWrapper,
        typename NativeType, typename NativeWrapper>
      inline static NativeType* SafeM2UMConvertGeneric( ManagedType^ mg_obj )
      {
        if (mg_obj == nullptr) return __nullptr;
        
        NativeWrapper^ obj = dynamic_cast<NativeWrapper^>( mg_obj );
        
        if(auto sDelta = dynamic_cast<Apache::Geode::Client::IGeodeDelta^> (mg_obj))
        {
          return new native::ManagedCacheableDeltaGeneric(sDelta);
        }
        else
        {
          return new ManagedWrapper(mg_obj, mg_obj->GetHashCode(), mg_obj->ClassId);
        }
      }

      template<typename NativeType, typename ManagedType>
      inline static NativeType* GetNativePtr( ManagedType^ mg_obj )
      {
        return (mg_obj != nullptr ? mg_obj->_NativePtr : NULL);
      }

      generic<class TValue>
      inline static TValue SafeGenericUMSerializableConvert( std::shared_ptr<native::Serializable> obj)
      {
        auto converted = SafeUMSerializableConvertGeneric(obj);

        if (converted == nullptr) return TValue();
        
        return safe_cast<TValue>(converted);
      }

      /// <summary>
      /// Helper function to convert managed <see cref="IGeodeSerializable" />
      /// object to native <c>native::Serializable</c> object using
      /// <c>SafeM2UMConvert</c>.
      /// </summary>
      inline static native::Serializable* SafeMSerializableConvertGeneric(
        Apache::Geode::Client::IGeodeSerializable^ mg_obj )
      {
        //it is called for cacheables types  only
        if (auto dataSerializable = dynamic_cast<IDataSerializable^>(mg_obj))
        {
          return SafeM2UMConvertGeneric<IDataSerializable,
            native::ManagedCacheableKeyGeneric, native::Serializable,
            Serializable>(dataSerializable);
        }
        // TODO serializable other serializable types here
        return nullptr;
      }

      generic<class TValue>
      inline static native::Cacheable* SafeGenericM2UMConvert( TValue mg_val)
      {
        if (mg_val == nullptr) return NULL;

				Object^ mg_obj = (Object^)mg_val;

        if(auto pdxType = dynamic_cast<IPdxSerializable^>(mg_obj))
        {
					return new native::PdxManagedCacheableKey(pdxType);
        }
      
        if(auto sDelta = dynamic_cast<IGeodeDelta^> (mg_obj))
				{
          return new native::ManagedCacheableDeltaGeneric(sDelta);
        }

        if(auto dataSerializable = dynamic_cast<IDataSerializable^>(mg_obj))
				{
					return new native::ManagedCacheableKeyGeneric(dataSerializable);
				}

        return new native::PdxManagedCacheableKey(gcnew PdxWrapper(mg_obj));
      }

      generic<class TValue>
      inline static native::Cacheable* SafeGenericMSerializableConvert( TValue mg_obj)
      {
        return SafeGenericM2UMConvert<TValue>( mg_obj );
      }

      inline static IPdxSerializable^ SafeUMSerializablePDXConvert( std::shared_ptr<native::Serializable> obj )
      {
         if(auto mg_obj = std::dynamic_pointer_cast<native::PdxManagedCacheableKey>( obj ))
           return mg_obj->ptr();

         throw gcnew IllegalStateException("Not be able to deserialize managed type");
      }

      /// <summary>
      /// Helper function to convert native <c>native::CacheableKey</c> object
      /// to managed <see cref="ICacheableKey" /> object.
      /// </summary>
      generic<class TKey>
      inline static Client::ICacheableKey^ SafeGenericUMKeyConvert( std::shared_ptr<native::CacheableKey> obj )
      {
        //All cacheables will be ManagedCacheableKey only
        if (obj == nullptr) return nullptr;
 
        if (auto mg_obj = std::dynamic_pointer_cast<native::ManagedCacheableKeyGeneric>(obj))
        {
            return (Client::ICacheableKey^)mg_obj->ptr( );
        }

        if (auto primitive = std::dynamic_pointer_cast<native::DataSerializablePrimitive>(obj)) {
          auto wrapperMethod = TypeRegistry::GetWrapperGeneric( primitive->getDsCode( ) );
          if (wrapperMethod != nullptr)
          {
            return (Client::ICacheableKey^)wrapperMethod(primitive);
          }
        }

        return gcnew Client::CacheableKey( obj );
      }

      template<typename NativeType, typename ManagedType>
      inline static NativeType* GetNativePtr2( ManagedType^ mg_obj )
      {
        if (mg_obj == nullptr) return NULL;
        //for cacheables types
        //return new native::ManagedCacheableKey(mg_obj, mg_obj->GetHashCode(), mg_obj->ClassId);
        {
          return new native::ManagedCacheableKeyGeneric( mg_obj, mg_obj->GetHashCode(), mg_obj->ClassId );
        }
      }

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

