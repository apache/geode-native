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



#include "ExceptionTypes.hpp"
#include <cstdlib>

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace msclr::interop;

      Dictionary<String^, CreateException2^>^ GeodeException::Init( )
      {
        if (Native2ManagedExMap != nullptr)
        {
          return Native2ManagedExMap;
        }
        array<NameDelegatePair>^ exNamesDelegates = gcnew array<NameDelegatePair> {
          { "apache::geode::client::" "AssertionException", gcnew CreateException2( AssertionException::Create ) },
          { "apache::geode::client::" "IllegalArgumentException", gcnew CreateException2( IllegalArgumentException::Create ) },
          { "apache::geode::client::" "IllegalStateException", gcnew CreateException2( IllegalStateException::Create ) },
          { "apache::geode::client::" "CacheExistsException", gcnew CreateException2( CacheExistsException::Create ) },
          { "apache::geode::client::" "CacheXmlException", gcnew CreateException2( CacheXmlException::Create ) },
          { "apache::geode::client::" "TimeoutException", gcnew CreateException2( TimeoutException::Create ) },
          { "apache::geode::client::" "CacheWriterException", gcnew CreateException2( CacheWriterException::Create ) },
          { "apache::geode::client::" "CacheListenerException", gcnew CreateException2( CacheListenerException::Create ) },
          { "apache::geode::client::" "RegionExistsException", gcnew CreateException2( RegionExistsException::Create ) },
          { "apache::geode::client::" "CacheClosedException", gcnew CreateException2( CacheClosedException::Create ) },
          { "apache::geode::client::" "LeaseExpiredException", gcnew CreateException2( LeaseExpiredException::Create ) },
          { "apache::geode::client::" "CacheLoaderException", gcnew CreateException2( CacheLoaderException::Create ) },
          { "apache::geode::client::" "RegionDestroyedException", gcnew CreateException2( RegionDestroyedException::Create ) },
          { "apache::geode::client::" "EntryDestroyedException", gcnew CreateException2( EntryDestroyedException::Create ) },
          { "apache::geode::client::" "NoSystemException", gcnew CreateException2( NoSystemException::Create ) },
          { "apache::geode::client::" "AlreadyConnectedException", gcnew CreateException2( AlreadyConnectedException::Create ) },
          { "apache::geode::client::" "FileNotFoundException", gcnew CreateException2( FileNotFoundException::Create ) },          
          { "apache::geode::client::" "InterruptedException", gcnew CreateException2( InterruptedException::Create ) },
          { "apache::geode::client::" "UnsupportedOperationException", gcnew CreateException2( UnsupportedOperationException::Create ) },
          { "apache::geode::client::" "StatisticsDisabledException", gcnew CreateException2( StatisticsDisabledException::Create ) },
          { "apache::geode::client::" "ConcurrentModificationException", gcnew CreateException2( ConcurrentModificationException::Create ) },
          { "apache::geode::client::" "UnknownException", gcnew CreateException2( UnknownException::Create ) },
          { "apache::geode::client::" "ClassCastException", gcnew CreateException2( ClassCastException::Create ) },
          { "apache::geode::client::" "EntryNotFoundException", gcnew CreateException2( EntryNotFoundException::Create ) },
          { "apache::geode::client::" "GeodeIOException", gcnew CreateException2( GeodeIOException::Create ) },
          { "apache::geode::client::" "GeodeConfigException", gcnew CreateException2( GeodeConfigException::Create ) },
          { "apache::geode::client::" "NullPointerException", gcnew CreateException2( NullPointerException::Create ) },
          { "apache::geode::client::" "EntryExistsException", gcnew CreateException2( EntryExistsException::Create ) },
          { "apache::geode::client::" "NotConnectedException", gcnew CreateException2( NotConnectedException::Create ) },
          { "apache::geode::client::" "CacheProxyException", gcnew CreateException2( CacheProxyException::Create ) },
          { "apache::geode::client::" "OutOfMemoryException", gcnew CreateException2( OutOfMemoryException::Create ) },
          { "apache::geode::client::" "NotOwnerException", gcnew CreateException2( NotOwnerException::Create ) },
          { "apache::geode::client::" "WrongRegionScopeException", gcnew CreateException2( WrongRegionScopeException::Create ) },
          { "apache::geode::client::" "BufferSizeExceededException", gcnew CreateException2( BufferSizeExceededException::Create ) },
          { "apache::geode::client::" "RegionCreationFailedException", gcnew CreateException2( RegionCreationFailedException::Create ) },
          { "apache::geode::client::" "FatalInternalException", gcnew CreateException2( FatalInternalException::Create ) },
          { "apache::geode::client::" "DiskFailureException", gcnew CreateException2( DiskFailureException::Create ) },
          { "apache::geode::client::" "DiskCorruptException", gcnew CreateException2( DiskCorruptException::Create ) },
          { "apache::geode::client::" "InitFailedException", gcnew CreateException2( InitFailedException::Create ) },
          { "apache::geode::client::" "ShutdownFailedException", gcnew CreateException2( ShutdownFailedException::Create ) },
          { "apache::geode::client::" "CacheServerException", gcnew CreateException2( CacheServerException::Create ) },
          { "apache::geode::client::" "OutOfRangeException", gcnew CreateException2( OutOfRangeException::Create ) },
          { "apache::geode::client::" "QueryException", gcnew CreateException2( QueryException::Create ) },
          { "apache::geode::client::" "MessageException", gcnew CreateException2( MessageException::Create ) },
          { "apache::geode::client::" "NotAuthorizedException", gcnew CreateException2( NotAuthorizedException::Create ) },
          { "apache::geode::client::" "AuthenticationFailedException", gcnew CreateException2( AuthenticationFailedException::Create ) },
          { "apache::geode::client::" "AuthenticationRequiredException", gcnew CreateException2( AuthenticationRequiredException::Create ) },
          { "apache::geode::client::" "DuplicateDurableClientException", gcnew CreateException2( DuplicateDurableClientException::Create ) },
          { "apache::geode::client::" "NoAvailableLocatorsException", gcnew CreateException2( NoAvailableLocatorsException::Create ) },
          { "apache::geode::client::" "FunctionExecutionException", gcnew CreateException2( FunctionExecutionException::Create ) },
          { "apache::geode::client::" "CqInvalidException", gcnew CreateException2( CqInvalidException::Create ) },
          { "apache::geode::client::" "CqExistsException", gcnew CreateException2( CqExistsException::Create ) },
          { "apache::geode::client::" "CqQueryException", gcnew CreateException2( CqQueryException::Create ) },
          { "apache::geode::client::" "CqClosedException", gcnew CreateException2( CqClosedException::Create ) },
          { "apache::geode::client::" "CqException", gcnew CreateException2( CqException::Create ) },
          { "apache::geode::client::" "AllConnectionsInUseException", gcnew CreateException2( AllConnectionsInUseException::Create ) },
          { "apache::geode::client::" "InvalidDeltaException", gcnew CreateException2( InvalidDeltaException::Create ) },
          { "apache::geode::client::" "KeyNotFoundException", gcnew CreateException2( KeyNotFoundException::Create ) },
          { "apache::geode::client::" "CommitConflictException", gcnew CreateException2( CommitConflictException::Create ) },
          { "apache::geode::client::" "TransactionDataNodeHasDepartedException", gcnew CreateException2( TransactionDataNodeHasDepartedException::Create ) },
          { "apache::geode::client::" "TransactionDataRebalancedException", gcnew CreateException2( TransactionDataRebalancedException::Create ) }
        };

        Native2ManagedExMap = gcnew Dictionary<String^, CreateException2^>( );
        for (System::Int32 index = 0; index < exNamesDelegates->Length; index++)
        {
          Native2ManagedExMap[ exNamesDelegates[ index ].m_name ] =
            exNamesDelegates[ index ].m_delegate;
        }
        return Native2ManagedExMap;
      }

      System::Exception^ GeodeException::Get(const apache::geode::client::Exception& nativeEx)
      {
        Exception^ innerException = nullptr;
        try
        {
          std::rethrow_if_nested(nativeEx);
        } catch (const apache::geode::client::Exception& ex) {
          innerException = GeodeException::Get(ex);
        }
        String^ exName = marshal_as<String^>(nativeEx.getName());
        CreateException2^ exDelegate;
        if (Native2ManagedExMap->TryGetValue(exName, exDelegate)) {
          return exDelegate(nativeEx, innerException);
        }
        String^ exMsg = marshal_as<String^>( nativeEx.getMessage( ) );
        if ( exMsg->StartsWith( GeodeException::MgSysExPrefix ) ) {
          // Get the exception type
          String^ mgExStr = exMsg->Substring(
            GeodeException::MgSysExPrefix->Length );
          System::Int32 colonIndex = mgExStr->IndexOf( ':' );
          if ( colonIndex > 0 ) {
            String^ mgExName = mgExStr->Substring( 0, colonIndex )->Trim( );
            // Try to load this class by reflection
            Type^ mgExType = Type::GetType( mgExName, false, true );
            if ( mgExType != nullptr ) {
              System::Reflection::ConstructorInfo^ cInfo = mgExType->
                GetConstructor(gcnew array<Type^>{ String::typeid, Exception::typeid });
              if ( cInfo != nullptr ) {
                String^ mgMsg = mgExStr->Substring( colonIndex + 1 );
                Exception^ mgEx = dynamic_cast<Exception^>(cInfo->Invoke(
                      gcnew array<Object^>{ mgMsg, innerException }));
                if ( mgEx != nullptr ) {
                  return mgEx;
                }
              }
            }

          }
          if (innerException == nullptr) {
            return gcnew GeodeException(exName + ": " + exMsg,
                gcnew GeodeException(GetStackTrace(nativeEx)));
          }
        }

        return gcnew GeodeException(exName + ": " + exMsg, innerException);
      }

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
