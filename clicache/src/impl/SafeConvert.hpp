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

#include "begin_native.hpp"
#include "CacheImpl.hpp"
#include "end_native.hpp"

#include "ManagedCacheableKey.hpp"
#include "ManagedCacheableDelta.hpp"
#include "ManagedCacheableKeyBytes.hpp"
#include "ManagedCacheableDeltaBytes.hpp"
#include "../Serializable.hpp"
#include "../Log.hpp"
#include "../CacheableKey.hpp"
#include "../CqEvent.hpp"
#include "PdxManagedCacheableKey.hpp"
#include "PdxManagedCacheableKeyBytes.hpp"
#include "PdxWrapper.hpp"
//TODO::split
#include "../CqEvent.hpp"
#include "../UserFunctionExecutionException.hpp"
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
      public:
        static bool isAppDomainEnabled = false;
  
        static void SetAppDomainEnabled(bool isAppDomainEnable)
        {
          Apache::Geode::Client::Log::Fine("AppDomain support enabled: " + isAppDomainEnable);
          isAppDomainEnabled = isAppDomainEnable;
        }
      };

      /// <summary>
      /// Helper function to convert native <c>apache::geode::client::Serializable</c> object
      /// to managed <see cref="IGeodeSerializable" /> object.
      /// </summary>
      inline static Apache::Geode::Client::IGeodeSerializable^
        SafeUMSerializableConvertGeneric(native::SerializablePtr obj)
      {
        if (obj == nullptr) return nullptr;

        if (SafeConvertClassGeneric::isAppDomainEnabled)
        {
          if (auto mg_bytesObj = std::dynamic_pointer_cast<native::ManagedCacheableKeyBytesGeneric>(obj))
          {
            return mg_bytesObj->ptr();
          }
          if (auto mg_bytesObj_delta = std::dynamic_pointer_cast<native::ManagedCacheableDeltaBytesGeneric>(obj))
          {
            return dynamic_cast<Apache::Geode::Client::IGeodeSerializable^>(mg_bytesObj_delta->ptr());
          }
        }
        else {
          if (auto mg_obj = std::dynamic_pointer_cast<native::ManagedCacheableKeyGeneric>(obj))
          {
            return mg_obj->ptr();
          }
          if (auto mg_obj_delta = std::dynamic_pointer_cast<native::ManagedCacheableDeltaGeneric>(obj))
          {
            return dynamic_cast<Apache::Geode::Client::IGeodeSerializable^>(mg_obj_delta->ptr());
          }
        }

        if (obj->typeId() == 0)
        {
          if (auto mg_UFEEobj = std::dynamic_pointer_cast<native::UserFunctionExecutionException>(obj))
          {
            return gcnew UserFunctionExecutionException(mg_UFEEobj);
          }
        }

        auto wrapperMethod = Apache::Geode::Client::Serializable::GetWrapperGeneric( obj->typeId( ) );             
        if (wrapperMethod != nullptr)
        {
          return wrapperMethod( obj );
        }

        return gcnew Apache::Geode::Client::Serializable( obj );
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
        /*
        *return SafeM2UMConvertGeneric<IGeodeSerializable, native::ManagedCacheableKey,
          native::Serializable, Serializable>( mg_obj );
        */
        //TODO: need to look this further for all types
        if (mg_obj == nullptr) return NULL;
        
        NativeWrapper^ obj = dynamic_cast<NativeWrapper^>( mg_obj );
        
        //if (obj != nullptr) {
        //  // this should not be 
        //  throw gcnew Exception("Something is worng");
        //  return obj->_NativePtr;
        //}
        //else 
        {
          Apache::Geode::Client::IGeodeDelta^ sDelta =
            dynamic_cast<Apache::Geode::Client::IGeodeDelta^> (mg_obj);
          if(sDelta != nullptr){
            if(!SafeConvertClassGeneric::isAppDomainEnabled)
              return new native::ManagedCacheableDeltaGeneric( sDelta);
            else
              return new native::ManagedCacheableDeltaBytesGeneric( sDelta, true);
          }
          else{
            if(!SafeConvertClassGeneric::isAppDomainEnabled)
              return new ManagedWrapper(mg_obj, mg_obj->GetHashCode(), mg_obj->ClassId, nullptr);
            else
              return new native::ManagedCacheableKeyBytesGeneric(mg_obj, true);
          }
        }
         //if (mg_obj == nullptr) return NULL;
         //return new ManagedWrapperGeneric(mg_obj, mg_obj->GetHashCode(), mg_obj->ClassId);
      }

      template<typename NativeType, typename ManagedType>
      inline static NativeType* GetNativePtr( ManagedType^ mg_obj )
      {
        return (mg_obj != nullptr ? mg_obj->_NativePtr : NULL);
      }

      generic<class TValue>
      inline static TValue SafeGenericUMSerializableConvert( native::SerializablePtr obj )
      {

        if (obj == nullptr) return TValue();
        
        if (SafeConvertClassGeneric::isAppDomainEnabled)
        {
          if (auto mg_bytesObj = std::dynamic_pointer_cast<native::ManagedCacheableKeyBytesGeneric>(obj))
          {
            return (TValue)mg_bytesObj->ptr();
          }
          if (auto mg_bytesObj_delta = std::dynamic_pointer_cast<native::ManagedCacheableDeltaBytesGeneric>(obj))
          {
            return safe_cast<TValue>(mg_bytesObj_delta->ptr());
          }
        } else {
          if (auto mg_obj = std::dynamic_pointer_cast<native::ManagedCacheableKeyGeneric>(obj))
          {
            return (TValue)mg_obj->ptr();
          }

          if (auto mg_obj_delta = std::dynamic_pointer_cast<native::ManagedCacheableDeltaGeneric>(obj))
          {
            return safe_cast<TValue>(mg_obj_delta->ptr());
          }
        }

        if (obj->typeId() == 0)
        {
          if (auto mg_UFEEobj = std::dynamic_pointer_cast<native::UserFunctionExecutionException>(obj))
          {
            return safe_cast<TValue> (gcnew UserFunctionExecutionException(mg_UFEEobj));
          }
        }

        auto wrapperMethod = Apache::Geode::Client::Serializable::GetWrapperGeneric( obj->typeId( ) );             
        if (wrapperMethod != nullptr)
        {
          return safe_cast<TValue>(wrapperMethod( obj ));
        }

        return safe_cast<TValue>(gcnew Apache::Geode::Client::Serializable( obj ));
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
        return SafeM2UMConvertGeneric<Apache::Geode::Client::IGeodeSerializable,
          native::ManagedCacheableKeyGeneric, native::Serializable,
          Apache::Geode::Client::Serializable>( mg_obj );
      }

      generic<class TValue>
      inline static native::Cacheable* SafeGenericM2UMConvert( TValue mg_val, native::Cache* cache )
      {
        if (mg_val == nullptr) return NULL;

				Object^ mg_obj = (Object^)mg_val;

				/*ICacheableKey^ iKey = dynamic_cast<ICacheableKey^>(obj);

        if(iKey != nullptr)
        {
          if(!SafeConvertClass::isAppDomainEnabled)
          return new vmware::ManagedCacheableKey(iKey);
        else
          return new vmware::ManagedCacheableKeyBytes( iKey, true);
        }*/

        IPdxSerializable^ pdxType = dynamic_cast<IPdxSerializable^>(mg_obj);

        if(pdxType != nullptr)
        {
          //TODO:: probably need to do for appdomain
					if(!SafeConvertClassGeneric::isAppDomainEnabled)
						return new native::PdxManagedCacheableKey(pdxType);
					else
						return new native::PdxManagedCacheableKeyBytes(pdxType, true, cache);
        }
      
				Apache::Geode::Client::IGeodeDelta^ sDelta =
            dynamic_cast<Apache::Geode::Client::IGeodeDelta^> (mg_obj);
          if(sDelta != nullptr)
					{
            if(!SafeConvertClassGeneric::isAppDomainEnabled)
              return new native::ManagedCacheableDeltaGeneric( sDelta);
            else
              return new native::ManagedCacheableDeltaBytesGeneric( sDelta, true);
          }
          else
					{
						Apache::Geode::Client::IGeodeSerializable^ tmpIGFS = 
							dynamic_cast<Apache::Geode::Client::IGeodeSerializable^>(mg_obj);
						if(tmpIGFS != nullptr)
						{
							if(!SafeConvertClassGeneric::isAppDomainEnabled)
							{
									return new native::ManagedCacheableKeyGeneric( tmpIGFS, nullptr);
							}
							else
							{
								return new native::ManagedCacheableKeyBytesGeneric( tmpIGFS, true);
							}
						}
            
            if(Serializable::IsObjectAndPdxSerializerRegistered(mg_obj->GetType()->FullName))
            {
              //TODO:: probably need to do for appdomain
					    if(!SafeConvertClassGeneric::isAppDomainEnabled)
					    	return new native::PdxManagedCacheableKey(gcnew PdxWrapper(mg_obj));
					    else
						    return new native::PdxManagedCacheableKeyBytes(gcnew PdxWrapper(mg_obj), true, nullptr);
            }
            throw gcnew Apache::Geode::Client::IllegalStateException(String::Format("Unable to map object type {0}. Possible Object type may not be registered or PdxSerializer is not registered. ", mg_obj->GetType()));
          }	
      }

      generic<class TValue>
      inline static native::Cacheable* SafeGenericMSerializableConvert( TValue mg_obj, native::Cache* cache )
      {
        return SafeGenericM2UMConvert<TValue>( mg_obj, cache );
      }

			inline static IPdxSerializable^ SafeUMSerializablePDXConvert( native::SerializablePtr obj )
      {
         if(auto mg_obj = std::dynamic_pointer_cast<native::PdxManagedCacheableKey>( obj ))
           return mg_obj->ptr();

				 if(auto mg_bytes = std::dynamic_pointer_cast<native::PdxManagedCacheableKeyBytes>( obj ))
           return mg_bytes->ptr();

         throw gcnew IllegalStateException("Not be able to deserialize managed type");
      }

      /// <summary>
      /// Helper function to convert native <c>native::CacheableKey</c> object
      /// to managed <see cref="ICacheableKey" /> object.
      /// </summary>
      generic<class TKey>
      inline static Client::ICacheableKey^ SafeGenericUMKeyConvert( native::CacheableKeyPtr obj )
      {
        //All cacheables will be ManagedCacheableKey only
        if (obj == nullptr) return nullptr;
 
        if (SafeConvertClassGeneric::isAppDomainEnabled)
        {
          if (auto mg_bytesObj = std::dynamic_pointer_cast<native::ManagedCacheableKeyBytesGeneric>(obj))
          {
            return (Client::ICacheableKey^)mg_bytesObj->ptr( );
          }
        }
        if (auto mg_obj = std::dynamic_pointer_cast<native::ManagedCacheableKeyGeneric>(obj))
        {
            return (Client::ICacheableKey^)mg_obj->ptr( );
        }

        auto wrapperMethod = Apache::Geode::Client::Serializable::GetWrapperGeneric( obj->typeId( ) );
        if (wrapperMethod != nullptr)
        {
          return (Client::ICacheableKey^)wrapperMethod( obj );
        }
        return gcnew Client::CacheableKey( obj );
      }

      generic <class TKey>
      inline static native::CacheableKey* SafeGenericMKeyConvert( TKey mg_obj )
      {
        if (mg_obj == nullptr) return NULL;
        auto obj = Apache::Geode::Client::Serializable::GetUnmanagedValueGeneric<TKey>( mg_obj, nullptr );
        if (obj.get() != nullptr)
        {
          return obj.get();
        }
        else
        {
          if(!SafeConvertClassGeneric::isAppDomainEnabled)
            return new native::ManagedCacheableKeyGeneric(SafeUMSerializableConvertGeneric(obj), nullptr);
          else
            return new native::ManagedCacheableKeyBytesGeneric(SafeUMSerializableConvertGeneric(obj), true);
        }
      }

      template<typename NativeType, typename ManagedType>
      inline static NativeType* GetNativePtr2( ManagedType^ mg_obj )
      {
        if (mg_obj == nullptr) return NULL;
        //for cacheables types
        //return new native::ManagedCacheableKey(mg_obj, mg_obj->GetHashCode(), mg_obj->ClassId);
        {
          if(!SafeConvertClassGeneric::isAppDomainEnabled)
            return new native::ManagedCacheableKeyGeneric( mg_obj, mg_obj->GetHashCode(), mg_obj->ClassId );
          else
            return new native::ManagedCacheableKeyBytesGeneric( mg_obj, true, nullptr);
        }
      }

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

