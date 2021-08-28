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


#include <Windows.h>
#include <msclr/marshal_cppstd.h>

#include "geode_defs.hpp"
#include "begin_native.hpp"
#include <geode/ExceptionTypes.hpp>
#include "end_native.hpp"


using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::Serialization;
using namespace msclr::interop;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      ref class GeodeException;

      /// <summary>
      /// Factory delegate to create a managed Geode exception.
      /// </summary>
      /// <remarks>
      /// For each managed exception class, its factory delegate is registered
      /// and maintained in a static dictionary mapped to its corresponding
      /// native Geode C++ exception name.
      /// </remarks>
      delegate GeodeException^ CreateException2(
        const apache::geode::client::Exception& nativeEx, System::Exception^ innerException);

      /// <summary>
      /// The base exception class of all managed Geode exceptions.
      /// </summary>
      [Serializable]
      public ref class GeodeException
        : public System::Exception
      {
      private:

        /// <summary>
        /// Prefix for distiguishing managed system exceptions
        /// </summary>
        literal String^ MgSysExPrefix = "GFCLI_EXCEPTION:";

        /// <summary>
        /// This contains a mapping of the native Geode exception class
        /// name to the factory delegate of the corresponding managed Geode
        /// exception class.
        /// </summary>
        static Dictionary<String^, CreateException2^>^ Native2ManagedExMap =
          Init( );

        /// <summary>
        /// Name and delegate pair class. The Native2ManagedExMap dictionary
        /// is populated from a static array of this class.
        /// </summary>
        value class NameDelegatePair
        {
        public:

          /// <summary>
          /// The name of the native Geode exception class.
          /// </summary>
          String^ m_name;

          /// <summary>
          /// The factory delegate of the managed Geode exception class
          /// corresponding to <c>m_name</c>
          /// </summary>
          CreateException2^ m_delegate;
        };


      internal:

        /// <summary>
        /// Static method to associate the native exception names with
        /// the corresponding managed exception factory delegates.
        /// </summary>
        /// <remarks>
        /// This method is not thread-safe and should be called in a single thread.
        /// </remarks>
        static Dictionary<String^, CreateException2^>^ Init( );

        /// <summary>
        /// Create the managed Geode exception for a given native Geode exception.
        /// As a special case normal system exceptions are also created when the
        /// native exception is a wrapper of a managed system exception.
        /// </summary>
        /// <remarks>
        /// Wherever the native Geode C++ code raises a <c>apache::geode::client::Exception</c>,
        /// the CLI wrapper code should have a catch-all for those and use
        /// this function to create the corresponding managed Geode exception.
        /// If no managed Geode exception has been defined (or has not been
        /// added using _GF_MG_EXCEPTION_ADD3 in ExceptionTypesMN.cpp) then a
        /// generic <c>GeodeException</c> exception is returned.
        /// </remarks>
        /// <param name="nativeEx">The native Geode exception object</param>
        /// <returns>
        /// The managed Geode exception object corresponding to the provided
        /// native Geode exception object.
        /// </returns>
        static Exception^ Get(const apache::geode::client::Exception& nativeEx);

        /// <summary>
        /// Get the stack trace for the given native exception.
        /// </summary>
        /// <param name="nativeEx">The native Geode exception object</param>
        /// <returns>The stack trace of the native exception.</returns>
        inline static String^ GetStackTrace(
          const apache::geode::client::Exception& nativeEx )
        {
          return marshal_as<String^>(nativeEx.getStackTrace());
        }

        /// <summary>
        /// Gets the C++ native exception object for a given managed exception.
        /// </summary>
        /// <remarks>
        /// This method is to handle conversion of managed exceptions to
        /// C++ exception for those thrown by managed callbacks.
        /// For non-Geode .NET exceptions we wrap it inside the generic
        /// <c>GeodeException</c> with a special prefix in message.
        /// While converting the exception back from C++ to .NET if the
        /// prefix is found in the message, then it tries to construct
        /// the original exception by reflection on the name of exception
        /// contained in the message. Note that in this process the
        /// original stacktrace is appended to the message of the exception.
        /// </remarks>
        inline static std::shared_ptr<apache::geode::client::Exception> GetNative(Exception^ ex)
        {
          if (ex != nullptr) {
            GeodeException^ gfEx = dynamic_cast<GeodeException^>(ex);
            if (gfEx != nullptr) {
              return gfEx->GetNative();
            }
            else {
              std::shared_ptr<apache::geode::client::Exception> cause;
              if (ex->InnerException != nullptr) {
                cause = GeodeException::GetNative(ex->InnerException);
              }
              return std::make_shared<apache::geode::client::Exception>(
                marshal_as<std::string>(MgSysExPrefix + ex->ToString()) 
                  + (cause ? cause->getMessage() : ""));
            }
          }
          return nullptr;
        }

        /// <summary>
        /// Gets the C++ native exception object for this managed
        /// <c>GeodeException</c>.
        /// </summary>
        virtual std::shared_ptr<apache::geode::client::Exception> GetNative()
        {
          std::shared_ptr<apache::geode::client::Exception> cause;
          if (this->InnerException != nullptr) {
            cause = GeodeException::GetNative(this->InnerException);
          }
          return std::make_shared<apache::geode::client::Exception>(
            marshal_as<std::string>(this->Message + ": " + this->StackTrace)
              + (cause ? cause->getMessage() : ""));
        }

        /// <summary>
        /// Throws the C++ native exception object for the given .NET exception.
        /// </summary>
        /// <remarks>
        /// This method is to handle conversion of managed exceptions to
        /// C++ exception for those thrown by managed callbacks.
        /// For non-Geode .NET exceptions we wrap it inside the generic
        /// <c>GeodeException</c> with a special prefix in message.
        /// While converting the exception back from C++ to .NET if the
        /// prefix is found in the message, then it tries to construct
        /// the original exception by reflection on the name of exception
        /// contained in the message. Note that in this process the
        /// original stacktrace is appended to the message of the exception.
        /// </remarks>
        inline static void ThrowNative(Exception^ ex)
        {
          if (ex != nullptr) {
            std::shared_ptr<apache::geode::client::Exception> cause;
            if (ex->InnerException != nullptr) {
              cause = GeodeException::GetNative(ex->InnerException);
            }
            throw apache::geode::client::Exception(
              marshal_as<std::string>(MgSysExPrefix + ex->ToString())
                + (cause ? cause->getMessage() : ""));
          }
        }

        /// <summary>
        /// Throws the C++ native exception object for this managed
        /// <c>GeodeException</c>.
        /// </summary>
        inline void ThrowNative()
        {
          throw GetNative();
        }


      public:

        /// <summary>
        /// Default constructor.
        /// </summary>
        inline GeodeException( )
          : Exception( ) { }

        /// <summary>
        /// Constructor to create an exception object with the given message.
        /// </summary>
        /// <param name="message">The exception message.</param>
        inline GeodeException( String^ message )
          : Exception( message ) { }

        /// <summary>
        /// Constructor to create an exception object with the given message
        /// and with the given inner exception.
        /// </summary>
        /// <param name="message">The exception message.</param>
        /// <param name="innerException">The inner exception object.</param>
        inline GeodeException( String^ message, System::Exception^ innerException )
          : Exception( message, innerException ) { }

      protected:

        /// <summary>
        /// Initializes a new instance of the <c>GeodeException</c> class with
        /// serialized data.
        /// This allows deserialization of this exception in .NET remoting.
        /// </summary>
        /// <param name="info">
        /// holds the serialized object data about
        /// the exception being thrown
        /// </param>
        /// <param name="context">
        /// contains contextual information about
        /// the source or destination
        /// </param>
        inline GeodeException( SerializationInfo^ info, StreamingContext context )
          : Exception( info, context ) { }
      };

/// Creates a class <c>x</c> named for each exception <c>y</c>.
//#define _GF_MG_EXCEPTION_DEF4(x,y) \
//      [Serializable] \
//      public ref class x: public GeodeException \
//      { \
//      public: \
//      \
//        /** <summary>Default constructor</summary> */ \
//        x( ) \
//          : GeodeException( ) { } \
//        \
//        /** <summary>
//         *  Constructor to create an exception object with the given message.
//         *  </summary>
//         *  <param name="message">The exception message.</param>
//         */ \
//        x( String^ message ) \
//          : GeodeException( message ) { } \
//        \
//        /** <summary>
//         *  Constructor to create an exception object with the given message
//         *  and with the given inner exception.
//         *  </summary>
//         *  <param name="message">The exception message.</param>
//         *  <param name="innerException">The inner exception object.</param>
//         */ \
//        x( String^ message, System::Exception^ innerException ) \
//          : GeodeException( message, innerException ) { } \
//        \
//      protected: \
//      \
//        /** <summary>
//         *  Initializes a new instance of the class with serialized data.
//         *  This allows deserialization of this exception in .NET remoting.
//         *  </summary>
//         *  <param name="info">
//         *  holds the serialized object data about the exception being thrown
//         *  </param>
//         *  <param name="context">
//         *  contains contextual information about the source or destination
//         *  </param>
//         */ \
//        x( SerializationInfo^ info, StreamingContext context ) \
//          : GeodeException( info, context ) { } \
//      \
//      internal: \
//        x(const apache::geode::client::y& nativeEx) \
//          : GeodeException(marshal_as<String^>(nativeEx.getMessage()), \
//              gcnew GeodeException(GeodeException::GetStackTrace( \
//                nativeEx))) { } \
//        \
//        x(const apache::geode::client::y& nativeEx, Exception^ innerException) \
//          : GeodeException(marshal_as<String^>(nativeEx.getMessage()), \
//              innerException) { } \
//        \
//        static GeodeException^ Create(const apache::geode::client::Exception& ex, \
//            Exception^ innerException) \
//        { \
//          const apache::geode::client::y* nativeEx = dynamic_cast<const apache::geode::client::y*>( &ex ); \
//          if (nativeEx != nullptr) { \
//            if (innerException == nullptr) { \
//              return gcnew x(*nativeEx); \
//            } \
//            else { \
//              return gcnew x(*nativeEx, innerException); \
//            } \
//          } \
//          return nullptr; \
//        } \
//        virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override \
//        { \
//          auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); \
//          if (this->InnerException != nullptr) { \
//            auto cause = GeodeException::GetNative(this->InnerException); \
//            message += "Caused by: " + cause->getMessage(); \
//          } \
//          return std::make_shared<apache::geode::client::y>(message); \
//        } \
//      }
//
///// Creates a class named for each exception <c>x</c>.
//#define _GF_MG_EXCEPTION_DEF3(x) _GF_MG_EXCEPTION_DEF4(x,x)


      // For all the native Geode C++ exceptions, a corresponding definition
      // should be added below *AND* it should also be added to the static array
      // in ExceptionTypesMN.cpp using _GF_MG_EXCEPTION_ADD3( x )

      /// <summary>
      /// A geode assertion exception.
      /// </summary>
      [Serializable] public ref class AssertionException: public GeodeException { public: AssertionException( ) : GeodeException( ) { } AssertionException( String^ message ) : GeodeException( message ) { } AssertionException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: AssertionException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: AssertionException(const apache::geode::client::AssertionException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } AssertionException(const apache::geode::client::AssertionException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::AssertionException* nativeEx = dynamic_cast<const apache::geode::client::AssertionException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew AssertionException(*nativeEx); } else { return gcnew AssertionException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::AssertionException>(message); } };

      /// <summary>
      /// Thrown when an argument to a method is illegal.
      /// </summary>
      [Serializable] public ref class IllegalArgumentException: public GeodeException { public: IllegalArgumentException( ) : GeodeException( ) { } IllegalArgumentException( String^ message ) : GeodeException( message ) { } IllegalArgumentException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: IllegalArgumentException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: IllegalArgumentException(const apache::geode::client::IllegalArgumentException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } IllegalArgumentException(const apache::geode::client::IllegalArgumentException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::IllegalArgumentException* nativeEx = dynamic_cast<const apache::geode::client::IllegalArgumentException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew IllegalArgumentException(*nativeEx); } else { return gcnew IllegalArgumentException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::IllegalArgumentException>(message); } };

      /// <summary>
      /// Thrown when the state of cache is manipulated to be illegal.
      /// </summary>
      [Serializable] public ref class IllegalStateException: public GeodeException { public: IllegalStateException( ) : GeodeException( ) { } IllegalStateException( String^ message ) : GeodeException( message ) { } IllegalStateException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: IllegalStateException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: IllegalStateException(const apache::geode::client::IllegalStateException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } IllegalStateException(const apache::geode::client::IllegalStateException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::IllegalStateException* nativeEx = dynamic_cast<const apache::geode::client::IllegalStateException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew IllegalStateException(*nativeEx); } else { return gcnew IllegalStateException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::IllegalStateException>(message); } };

      /// <summary>
      /// Thrown when an attempt is made to create an existing cache.
      /// </summary>
      [Serializable] public ref class CacheExistsException: public GeodeException { public: CacheExistsException( ) : GeodeException( ) { } CacheExistsException( String^ message ) : GeodeException( message ) { } CacheExistsException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CacheExistsException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CacheExistsException(const apache::geode::client::CacheExistsException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CacheExistsException(const apache::geode::client::CacheExistsException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CacheExistsException* nativeEx = dynamic_cast<const apache::geode::client::CacheExistsException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CacheExistsException(*nativeEx); } else { return gcnew CacheExistsException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CacheExistsException>(message); } };

      /// <summary>
      /// Thrown when the cache xml is incorrect.
      /// </summary>
      [Serializable] public ref class CacheXmlException: public GeodeException { public: CacheXmlException( ) : GeodeException( ) { } CacheXmlException( String^ message ) : GeodeException( message ) { } CacheXmlException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CacheXmlException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CacheXmlException(const apache::geode::client::CacheXmlException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CacheXmlException(const apache::geode::client::CacheXmlException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CacheXmlException* nativeEx = dynamic_cast<const apache::geode::client::CacheXmlException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CacheXmlException(*nativeEx); } else { return gcnew CacheXmlException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CacheXmlException>(message); } };

      /// <summary>
      /// Thrown when a timout occurs.
      /// </summary>
      [Serializable] public ref class TimeoutException: public GeodeException { public: TimeoutException( ) : GeodeException( ) { } TimeoutException( String^ message ) : GeodeException( message ) { } TimeoutException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: TimeoutException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: TimeoutException(const apache::geode::client::TimeoutException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } TimeoutException(const apache::geode::client::TimeoutException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::TimeoutException* nativeEx = dynamic_cast<const apache::geode::client::TimeoutException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew TimeoutException(*nativeEx); } else { return gcnew TimeoutException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::TimeoutException>(message); } };

      /// <summary>
      /// Thrown when the cache writer aborts the operation.
      /// </summary>
      [Serializable] public ref class CacheWriterException: public GeodeException { public: CacheWriterException( ) : GeodeException( ) { } CacheWriterException( String^ message ) : GeodeException( message ) { } CacheWriterException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CacheWriterException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CacheWriterException(const apache::geode::client::CacheWriterException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CacheWriterException(const apache::geode::client::CacheWriterException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CacheWriterException* nativeEx = dynamic_cast<const apache::geode::client::CacheWriterException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CacheWriterException(*nativeEx); } else { return gcnew CacheWriterException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CacheWriterException>(message); } };

      /// <summary>
      /// Thrown when the cache listener throws an exception.
      /// </summary>
      [Serializable] public ref class CacheListenerException: public GeodeException { public: CacheListenerException( ) : GeodeException( ) { } CacheListenerException( String^ message ) : GeodeException( message ) { } CacheListenerException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CacheListenerException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CacheListenerException(const apache::geode::client::CacheListenerException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CacheListenerException(const apache::geode::client::CacheListenerException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CacheListenerException* nativeEx = dynamic_cast<const apache::geode::client::CacheListenerException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CacheListenerException(*nativeEx); } else { return gcnew CacheListenerException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CacheListenerException>(message); } };

      /// <summary>
      /// Thrown when an attempt is made to create an existing region.
      /// </summary>
      [Serializable] public ref class RegionExistsException: public GeodeException { public: RegionExistsException( ) : GeodeException( ) { } RegionExistsException( String^ message ) : GeodeException( message ) { } RegionExistsException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: RegionExistsException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: RegionExistsException(const apache::geode::client::RegionExistsException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } RegionExistsException(const apache::geode::client::RegionExistsException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::RegionExistsException* nativeEx = dynamic_cast<const apache::geode::client::RegionExistsException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew RegionExistsException(*nativeEx); } else { return gcnew RegionExistsException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::RegionExistsException>(message); } };

      /// <summary>
      /// Thrown when an operation is attempted on a closed cache.
      /// </summary>
      [Serializable] public ref class CacheClosedException: public GeodeException { public: CacheClosedException( ) : GeodeException( ) { } CacheClosedException( String^ message ) : GeodeException( message ) { } CacheClosedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CacheClosedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CacheClosedException(const apache::geode::client::CacheClosedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CacheClosedException(const apache::geode::client::CacheClosedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CacheClosedException* nativeEx = dynamic_cast<const apache::geode::client::CacheClosedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CacheClosedException(*nativeEx); } else { return gcnew CacheClosedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CacheClosedException>(message); } };

      /// <summary>
      /// Thrown when lease of cache proxy has expired.
      /// </summary>
      [Serializable] public ref class LeaseExpiredException: public GeodeException { public: LeaseExpiredException( ) : GeodeException( ) { } LeaseExpiredException( String^ message ) : GeodeException( message ) { } LeaseExpiredException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: LeaseExpiredException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: LeaseExpiredException(const apache::geode::client::LeaseExpiredException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } LeaseExpiredException(const apache::geode::client::LeaseExpiredException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::LeaseExpiredException* nativeEx = dynamic_cast<const apache::geode::client::LeaseExpiredException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew LeaseExpiredException(*nativeEx); } else { return gcnew LeaseExpiredException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::LeaseExpiredException>(message); } };

      /// <summary>
      /// Thrown when the cache loader aborts the operation.
      /// </summary>
      [Serializable] public ref class CacheLoaderException: public GeodeException { public: CacheLoaderException( ) : GeodeException( ) { } CacheLoaderException( String^ message ) : GeodeException( message ) { } CacheLoaderException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CacheLoaderException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CacheLoaderException(const apache::geode::client::CacheLoaderException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CacheLoaderException(const apache::geode::client::CacheLoaderException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CacheLoaderException* nativeEx = dynamic_cast<const apache::geode::client::CacheLoaderException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CacheLoaderException(*nativeEx); } else { return gcnew CacheLoaderException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CacheLoaderException>(message); } };

      /// <summary>
      /// Thrown when an operation is attempted on a destroyed region.
      /// </summary>
      [Serializable] public ref class RegionDestroyedException: public GeodeException { public: RegionDestroyedException( ) : GeodeException( ) { } RegionDestroyedException( String^ message ) : GeodeException( message ) { } RegionDestroyedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: RegionDestroyedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: RegionDestroyedException(const apache::geode::client::RegionDestroyedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } RegionDestroyedException(const apache::geode::client::RegionDestroyedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::RegionDestroyedException* nativeEx = dynamic_cast<const apache::geode::client::RegionDestroyedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew RegionDestroyedException(*nativeEx); } else { return gcnew RegionDestroyedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::RegionDestroyedException>(message); } };

      /// <summary>
      /// Thrown when an operation is attempted on a destroyed entry.
      /// </summary>
      [Serializable] public ref class EntryDestroyedException: public GeodeException { public: EntryDestroyedException( ) : GeodeException( ) { } EntryDestroyedException( String^ message ) : GeodeException( message ) { } EntryDestroyedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: EntryDestroyedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: EntryDestroyedException(const apache::geode::client::EntryDestroyedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } EntryDestroyedException(const apache::geode::client::EntryDestroyedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::EntryDestroyedException* nativeEx = dynamic_cast<const apache::geode::client::EntryDestroyedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew EntryDestroyedException(*nativeEx); } else { return gcnew EntryDestroyedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::EntryDestroyedException>(message); } };

      /// <summary>
      /// Thrown when the connecting target is not running.
      /// </summary>
      [Serializable] public ref class NoSystemException: public GeodeException { public: NoSystemException( ) : GeodeException( ) { } NoSystemException( String^ message ) : GeodeException( message ) { } NoSystemException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: NoSystemException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: NoSystemException(const apache::geode::client::NoSystemException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } NoSystemException(const apache::geode::client::NoSystemException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::NoSystemException* nativeEx = dynamic_cast<const apache::geode::client::NoSystemException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew NoSystemException(*nativeEx); } else { return gcnew NoSystemException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::NoSystemException>(message); } };

      /// <summary>
      /// Thrown when an attempt is made to connect to
      /// DistributedSystem second time.
      /// </summary>
      [Serializable] public ref class AlreadyConnectedException: public GeodeException { public: AlreadyConnectedException( ) : GeodeException( ) { } AlreadyConnectedException( String^ message ) : GeodeException( message ) { } AlreadyConnectedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: AlreadyConnectedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: AlreadyConnectedException(const apache::geode::client::AlreadyConnectedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } AlreadyConnectedException(const apache::geode::client::AlreadyConnectedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::AlreadyConnectedException* nativeEx = dynamic_cast<const apache::geode::client::AlreadyConnectedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew AlreadyConnectedException(*nativeEx); } else { return gcnew AlreadyConnectedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::AlreadyConnectedException>(message); } };

      /// <summary>
      /// Thrown when a non-existing file is accessed.
      /// </summary>
      [Serializable] public ref class FileNotFoundException: public GeodeException { public: FileNotFoundException( ) : GeodeException( ) { } FileNotFoundException( String^ message ) : GeodeException( message ) { } FileNotFoundException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: FileNotFoundException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: FileNotFoundException(const apache::geode::client::FileNotFoundException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } FileNotFoundException(const apache::geode::client::FileNotFoundException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::FileNotFoundException* nativeEx = dynamic_cast<const apache::geode::client::FileNotFoundException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew FileNotFoundException(*nativeEx); } else { return gcnew FileNotFoundException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::FileNotFoundException>(message); } };

      /// <summary>
      /// Thrown when an operation is interrupted.
      /// </summary>
      [Serializable] public ref class InterruptedException: public GeodeException { public: InterruptedException( ) : GeodeException( ) { } InterruptedException( String^ message ) : GeodeException( message ) { } InterruptedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: InterruptedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: InterruptedException(const apache::geode::client::InterruptedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } InterruptedException(const apache::geode::client::InterruptedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::InterruptedException* nativeEx = dynamic_cast<const apache::geode::client::InterruptedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew InterruptedException(*nativeEx); } else { return gcnew InterruptedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::InterruptedException>(message); } };

      /// <summary>
      /// Thrown when an operation unsupported by the
      /// current configuration is attempted.
      /// </summary>
      [Serializable] public ref class UnsupportedOperationException: public GeodeException { public: UnsupportedOperationException( ) : GeodeException( ) { } UnsupportedOperationException( String^ message ) : GeodeException( message ) { } UnsupportedOperationException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: UnsupportedOperationException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: UnsupportedOperationException(const apache::geode::client::UnsupportedOperationException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } UnsupportedOperationException(const apache::geode::client::UnsupportedOperationException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::UnsupportedOperationException* nativeEx = dynamic_cast<const apache::geode::client::UnsupportedOperationException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew UnsupportedOperationException(*nativeEx); } else { return gcnew UnsupportedOperationException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::UnsupportedOperationException>(message); } };

      /// <summary>
      /// Thrown when statistics are invoked for a region where
      /// they are disabled.
      /// </summary>
      [Serializable] public ref class StatisticsDisabledException: public GeodeException { public: StatisticsDisabledException( ) : GeodeException( ) { } StatisticsDisabledException( String^ message ) : GeodeException( message ) { } StatisticsDisabledException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: StatisticsDisabledException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: StatisticsDisabledException(const apache::geode::client::StatisticsDisabledException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } StatisticsDisabledException(const apache::geode::client::StatisticsDisabledException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::StatisticsDisabledException* nativeEx = dynamic_cast<const apache::geode::client::StatisticsDisabledException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew StatisticsDisabledException(*nativeEx); } else { return gcnew StatisticsDisabledException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::StatisticsDisabledException>(message); } };

      /// <summary>
      /// Thrown when a concurrent operation fails.
      /// </summary>
      [Serializable] public ref class ConcurrentModificationException: public GeodeException { public: ConcurrentModificationException( ) : GeodeException( ) { } ConcurrentModificationException( String^ message ) : GeodeException( message ) { } ConcurrentModificationException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: ConcurrentModificationException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: ConcurrentModificationException(const apache::geode::client::ConcurrentModificationException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } ConcurrentModificationException(const apache::geode::client::ConcurrentModificationException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::ConcurrentModificationException* nativeEx = dynamic_cast<const apache::geode::client::ConcurrentModificationException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew ConcurrentModificationException(*nativeEx); } else { return gcnew ConcurrentModificationException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::ConcurrentModificationException>(message); } };

      /// <summary>
      /// An unknown exception occurred.
      /// </summary>
      [Serializable] public ref class UnknownException: public GeodeException { public: UnknownException( ) : GeodeException( ) { } UnknownException( String^ message ) : GeodeException( message ) { } UnknownException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: UnknownException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: UnknownException(const apache::geode::client::UnknownException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } UnknownException(const apache::geode::client::UnknownException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::UnknownException* nativeEx = dynamic_cast<const apache::geode::client::UnknownException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew UnknownException(*nativeEx); } else { return gcnew UnknownException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::UnknownException>(message); } };

      /// <summary>
      /// Thrown when a cast operation fails.
      /// </summary>
      [Serializable] public ref class ClassCastException: public GeodeException { public: ClassCastException( ) : GeodeException( ) { } ClassCastException( String^ message ) : GeodeException( message ) { } ClassCastException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: ClassCastException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: ClassCastException(const apache::geode::client::ClassCastException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } ClassCastException(const apache::geode::client::ClassCastException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::ClassCastException* nativeEx = dynamic_cast<const apache::geode::client::ClassCastException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew ClassCastException(*nativeEx); } else { return gcnew ClassCastException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::ClassCastException>(message); } };

      /// <summary>
      /// Thrown when an operation is attempted on a non-existent entry.
      /// </summary>
      [Serializable] public ref class EntryNotFoundException: public GeodeException { public: EntryNotFoundException( ) : GeodeException( ) { } EntryNotFoundException( String^ message ) : GeodeException( message ) { } EntryNotFoundException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: EntryNotFoundException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: EntryNotFoundException(const apache::geode::client::EntryNotFoundException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } EntryNotFoundException(const apache::geode::client::EntryNotFoundException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::EntryNotFoundException* nativeEx = dynamic_cast<const apache::geode::client::EntryNotFoundException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew EntryNotFoundException(*nativeEx); } else { return gcnew EntryNotFoundException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::EntryNotFoundException>(message); } };

      /// <summary>
      /// Thrown when there is an input/output error.
      /// </summary>
      [Serializable] public ref class GeodeIOException: public GeodeException { public: GeodeIOException( ) : GeodeException( ) { } GeodeIOException( String^ message ) : GeodeException( message ) { } GeodeIOException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: GeodeIOException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: GeodeIOException(const apache::geode::client::GeodeIOException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } GeodeIOException(const apache::geode::client::GeodeIOException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::GeodeIOException* nativeEx = dynamic_cast<const apache::geode::client::GeodeIOException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew GeodeIOException(*nativeEx); } else { return gcnew GeodeIOException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::GeodeIOException>(message); } };

      /// <summary>
      /// Thrown when geode configuration file is incorrect.
      /// </summary>
      [Serializable] public ref class GeodeConfigException: public GeodeException { public: GeodeConfigException( ) : GeodeException( ) { } GeodeConfigException( String^ message ) : GeodeException( message ) { } GeodeConfigException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: GeodeConfigException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: GeodeConfigException(const apache::geode::client::GeodeConfigException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } GeodeConfigException(const apache::geode::client::GeodeConfigException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::GeodeConfigException* nativeEx = dynamic_cast<const apache::geode::client::GeodeConfigException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew GeodeConfigException(*nativeEx); } else { return gcnew GeodeConfigException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::GeodeConfigException>(message); } };

      /// <summary>
      /// Thrown when a null argument is provided to a method
      /// where it is expected to be non-null.
      /// </summary>
      [Serializable] public ref class NullPointerException: public GeodeException { public: NullPointerException( ) : GeodeException( ) { } NullPointerException( String^ message ) : GeodeException( message ) { } NullPointerException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: NullPointerException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: NullPointerException(const apache::geode::client::NullPointerException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } NullPointerException(const apache::geode::client::NullPointerException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::NullPointerException* nativeEx = dynamic_cast<const apache::geode::client::NullPointerException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew NullPointerException(*nativeEx); } else { return gcnew NullPointerException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::NullPointerException>(message); } };

      /// <summary>
      /// Thrown when attempt is made to create an existing entry.
      /// </summary>
      [Serializable] public ref class EntryExistsException: public GeodeException { public: EntryExistsException( ) : GeodeException( ) { } EntryExistsException( String^ message ) : GeodeException( message ) { } EntryExistsException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: EntryExistsException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: EntryExistsException(const apache::geode::client::EntryExistsException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } EntryExistsException(const apache::geode::client::EntryExistsException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::EntryExistsException* nativeEx = dynamic_cast<const apache::geode::client::EntryExistsException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew EntryExistsException(*nativeEx); } else { return gcnew EntryExistsException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::EntryExistsException>(message); } };

      /// <summary>
      /// Thrown when an operation is attempted before connecting
      /// to the distributed system.
      /// </summary>
      [Serializable] public ref class NotConnectedException: public GeodeException { public: NotConnectedException( ) : GeodeException( ) { } NotConnectedException( String^ message ) : GeodeException( message ) { } NotConnectedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: NotConnectedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: NotConnectedException(const apache::geode::client::NotConnectedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } NotConnectedException(const apache::geode::client::NotConnectedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::NotConnectedException* nativeEx = dynamic_cast<const apache::geode::client::NotConnectedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew NotConnectedException(*nativeEx); } else { return gcnew NotConnectedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::NotConnectedException>(message); } };

      /// <summary>
      /// Thrown when there is an error in the cache proxy.
      /// </summary>
      [Serializable] public ref class CacheProxyException: public GeodeException { public: CacheProxyException( ) : GeodeException( ) { } CacheProxyException( String^ message ) : GeodeException( message ) { } CacheProxyException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CacheProxyException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CacheProxyException(const apache::geode::client::CacheProxyException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CacheProxyException(const apache::geode::client::CacheProxyException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CacheProxyException* nativeEx = dynamic_cast<const apache::geode::client::CacheProxyException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CacheProxyException(*nativeEx); } else { return gcnew CacheProxyException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CacheProxyException>(message); } };

      /// <summary>
      /// Thrown when the system cannot allocate any more memory.
      /// </summary>
      [Serializable] public ref class OutOfMemoryException: public GeodeException { public: OutOfMemoryException( ) : GeodeException( ) { } OutOfMemoryException( String^ message ) : GeodeException( message ) { } OutOfMemoryException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: OutOfMemoryException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: OutOfMemoryException(const apache::geode::client::OutOfMemoryException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } OutOfMemoryException(const apache::geode::client::OutOfMemoryException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::OutOfMemoryException* nativeEx = dynamic_cast<const apache::geode::client::OutOfMemoryException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew OutOfMemoryException(*nativeEx); } else { return gcnew OutOfMemoryException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::OutOfMemoryException>(message); } };

      /// <summary>
      /// Thrown when an attempt is made to release a lock not
      /// owned by the thread.
      /// </summary>
      [Serializable] public ref class NotOwnerException: public GeodeException { public: NotOwnerException( ) : GeodeException( ) { } NotOwnerException( String^ message ) : GeodeException( message ) { } NotOwnerException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: NotOwnerException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: NotOwnerException(const apache::geode::client::NotOwnerException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } NotOwnerException(const apache::geode::client::NotOwnerException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::NotOwnerException* nativeEx = dynamic_cast<const apache::geode::client::NotOwnerException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew NotOwnerException(*nativeEx); } else { return gcnew NotOwnerException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::NotOwnerException>(message); } };

      /// <summary>
      /// Thrown when a region is created in an incorrect scope.
      /// </summary>
      [Serializable] public ref class WrongRegionScopeException: public GeodeException { public: WrongRegionScopeException( ) : GeodeException( ) { } WrongRegionScopeException( String^ message ) : GeodeException( message ) { } WrongRegionScopeException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: WrongRegionScopeException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: WrongRegionScopeException(const apache::geode::client::WrongRegionScopeException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } WrongRegionScopeException(const apache::geode::client::WrongRegionScopeException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::WrongRegionScopeException* nativeEx = dynamic_cast<const apache::geode::client::WrongRegionScopeException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew WrongRegionScopeException(*nativeEx); } else { return gcnew WrongRegionScopeException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::WrongRegionScopeException>(message); } };

      /// <summary>
      /// Thrown when the internal buffer size is exceeded.
      /// </summary>
      [Serializable] public ref class BufferSizeExceededException: public GeodeException { public: BufferSizeExceededException( ) : GeodeException( ) { } BufferSizeExceededException( String^ message ) : GeodeException( message ) { } BufferSizeExceededException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: BufferSizeExceededException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: BufferSizeExceededException(const apache::geode::client::BufferSizeExceededException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } BufferSizeExceededException(const apache::geode::client::BufferSizeExceededException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::BufferSizeExceededException* nativeEx = dynamic_cast<const apache::geode::client::BufferSizeExceededException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew BufferSizeExceededException(*nativeEx); } else { return gcnew BufferSizeExceededException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::BufferSizeExceededException>(message); } };

      /// <summary>
      /// Thrown when a region creation operation fails.
      /// </summary>
      [Serializable] public ref class RegionCreationFailedException: public GeodeException { public: RegionCreationFailedException( ) : GeodeException( ) { } RegionCreationFailedException( String^ message ) : GeodeException( message ) { } RegionCreationFailedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: RegionCreationFailedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: RegionCreationFailedException(const apache::geode::client::RegionCreationFailedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } RegionCreationFailedException(const apache::geode::client::RegionCreationFailedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::RegionCreationFailedException* nativeEx = dynamic_cast<const apache::geode::client::RegionCreationFailedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew RegionCreationFailedException(*nativeEx); } else { return gcnew RegionCreationFailedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::RegionCreationFailedException>(message); } };

      /// <summary>
      /// Thrown when there is a fatal internal exception in Geode.
      /// </summary>
      [Serializable] public ref class FatalInternalException: public GeodeException { public: FatalInternalException( ) : GeodeException( ) { } FatalInternalException( String^ message ) : GeodeException( message ) { } FatalInternalException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: FatalInternalException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: FatalInternalException(const apache::geode::client::FatalInternalException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } FatalInternalException(const apache::geode::client::FatalInternalException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::FatalInternalException* nativeEx = dynamic_cast<const apache::geode::client::FatalInternalException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew FatalInternalException(*nativeEx); } else { return gcnew FatalInternalException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::FatalInternalException>(message); } };

      /// <summary>
      /// Thrown by the persistence manager when a write
      /// fails due to disk failure.
      /// </summary>
      [Serializable] public ref class DiskFailureException: public GeodeException { public: DiskFailureException( ) : GeodeException( ) { } DiskFailureException( String^ message ) : GeodeException( message ) { } DiskFailureException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: DiskFailureException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: DiskFailureException(const apache::geode::client::DiskFailureException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } DiskFailureException(const apache::geode::client::DiskFailureException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::DiskFailureException* nativeEx = dynamic_cast<const apache::geode::client::DiskFailureException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew DiskFailureException(*nativeEx); } else { return gcnew DiskFailureException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::DiskFailureException>(message); } };

      /// <summary>
      /// Thrown by the persistence manager when the data
      /// to be read from disk is corrupt.
      /// </summary>
      [Serializable] public ref class DiskCorruptException: public GeodeException { public: DiskCorruptException( ) : GeodeException( ) { } DiskCorruptException( String^ message ) : GeodeException( message ) { } DiskCorruptException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: DiskCorruptException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: DiskCorruptException(const apache::geode::client::DiskCorruptException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } DiskCorruptException(const apache::geode::client::DiskCorruptException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::DiskCorruptException* nativeEx = dynamic_cast<const apache::geode::client::DiskCorruptException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew DiskCorruptException(*nativeEx); } else { return gcnew DiskCorruptException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::DiskCorruptException>(message); } };

      /// <summary>
      /// Thrown when persistence manager fails to initialize.
      /// </summary>
      [Serializable] public ref class InitFailedException: public GeodeException { public: InitFailedException( ) : GeodeException( ) { } InitFailedException( String^ message ) : GeodeException( message ) { } InitFailedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: InitFailedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: InitFailedException(const apache::geode::client::InitFailedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } InitFailedException(const apache::geode::client::InitFailedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::InitFailedException* nativeEx = dynamic_cast<const apache::geode::client::InitFailedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew InitFailedException(*nativeEx); } else { return gcnew InitFailedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::InitFailedException>(message); } };

      /// <summary>
      /// Thrown when persistence manager fails to close properly.
      /// </summary>
      [Serializable] public ref class ShutdownFailedException: public GeodeException { public: ShutdownFailedException( ) : GeodeException( ) { } ShutdownFailedException( String^ message ) : GeodeException( message ) { } ShutdownFailedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: ShutdownFailedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: ShutdownFailedException(const apache::geode::client::ShutdownFailedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } ShutdownFailedException(const apache::geode::client::ShutdownFailedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::ShutdownFailedException* nativeEx = dynamic_cast<const apache::geode::client::ShutdownFailedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew ShutdownFailedException(*nativeEx); } else { return gcnew ShutdownFailedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::ShutdownFailedException>(message); } };

      /// <summary>
      /// Thrown when an exception occurs on the cache server.
      /// </summary>
      [Serializable] public ref class CacheServerException: public GeodeException { public: CacheServerException( ) : GeodeException( ) { } CacheServerException( String^ message ) : GeodeException( message ) { } CacheServerException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CacheServerException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CacheServerException(const apache::geode::client::CacheServerException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CacheServerException(const apache::geode::client::CacheServerException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CacheServerException* nativeEx = dynamic_cast<const apache::geode::client::CacheServerException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CacheServerException(*nativeEx); } else { return gcnew CacheServerException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CacheServerException>(message); } };

      /// <summary>
      /// Thrown when bound of array/vector etc. is exceeded.
      /// </summary>
      [Serializable] public ref class OutOfRangeException: public GeodeException { public: OutOfRangeException( ) : GeodeException( ) { } OutOfRangeException( String^ message ) : GeodeException( message ) { } OutOfRangeException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: OutOfRangeException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: OutOfRangeException(const apache::geode::client::OutOfRangeException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } OutOfRangeException(const apache::geode::client::OutOfRangeException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::OutOfRangeException* nativeEx = dynamic_cast<const apache::geode::client::OutOfRangeException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew OutOfRangeException(*nativeEx); } else { return gcnew OutOfRangeException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::OutOfRangeException>(message); } };

      /// <summary>
      /// Thrown when query exception occurs at the server.
      /// </summary>
      [Serializable] public ref class QueryException: public GeodeException { public: QueryException( ) : GeodeException( ) { } QueryException( String^ message ) : GeodeException( message ) { } QueryException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: QueryException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: QueryException(const apache::geode::client::QueryException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } QueryException(const apache::geode::client::QueryException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::QueryException* nativeEx = dynamic_cast<const apache::geode::client::QueryException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew QueryException(*nativeEx); } else { return gcnew QueryException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::QueryException>(message); } };

      /// <summary>
      /// Thrown when an unknown message is received from the server.
      /// </summary>
      [Serializable] public ref class MessageException: public GeodeException { public: MessageException( ) : GeodeException( ) { } MessageException( String^ message ) : GeodeException( message ) { } MessageException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: MessageException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: MessageException(const apache::geode::client::MessageException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } MessageException(const apache::geode::client::MessageException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::MessageException* nativeEx = dynamic_cast<const apache::geode::client::MessageException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew MessageException(*nativeEx); } else { return gcnew MessageException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::MessageException>(message); } };

      /// <summary>
      /// Thrown when a client operation is not authorized on the server.
      /// </summary>
      [Serializable] public ref class NotAuthorizedException: public GeodeException { public: NotAuthorizedException( ) : GeodeException( ) { } NotAuthorizedException( String^ message ) : GeodeException( message ) { } NotAuthorizedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: NotAuthorizedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: NotAuthorizedException(const apache::geode::client::NotAuthorizedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } NotAuthorizedException(const apache::geode::client::NotAuthorizedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::NotAuthorizedException* nativeEx = dynamic_cast<const apache::geode::client::NotAuthorizedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew NotAuthorizedException(*nativeEx); } else { return gcnew NotAuthorizedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::NotAuthorizedException>(message); } };

      /// <summary>
      /// Thrown when authentication to the server fails.
      /// </summary>
      [Serializable] public ref class AuthenticationFailedException: public GeodeException { public: AuthenticationFailedException( ) : GeodeException( ) { } AuthenticationFailedException( String^ message ) : GeodeException( message ) { } AuthenticationFailedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: AuthenticationFailedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: AuthenticationFailedException(const apache::geode::client::AuthenticationFailedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } AuthenticationFailedException(const apache::geode::client::AuthenticationFailedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::AuthenticationFailedException* nativeEx = dynamic_cast<const apache::geode::client::AuthenticationFailedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew AuthenticationFailedException(*nativeEx); } else { return gcnew AuthenticationFailedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::AuthenticationFailedException>(message); } };

      /// <summary>
      /// Thrown when credentials are not provided to a server which expects them.
      /// </summary>
      [Serializable] public ref class AuthenticationRequiredException: public GeodeException { public: AuthenticationRequiredException( ) : GeodeException( ) { } AuthenticationRequiredException( String^ message ) : GeodeException( message ) { } AuthenticationRequiredException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: AuthenticationRequiredException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: AuthenticationRequiredException(const apache::geode::client::AuthenticationRequiredException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } AuthenticationRequiredException(const apache::geode::client::AuthenticationRequiredException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::AuthenticationRequiredException* nativeEx = dynamic_cast<const apache::geode::client::AuthenticationRequiredException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew AuthenticationRequiredException(*nativeEx); } else { return gcnew AuthenticationRequiredException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::AuthenticationRequiredException>(message); } };

      /// <summary>
      /// Thrown when a duplicate durable client id is provided to the server.
      /// </summary>
      [Serializable] public ref class DuplicateDurableClientException: public GeodeException { public: DuplicateDurableClientException( ) : GeodeException( ) { } DuplicateDurableClientException( String^ message ) : GeodeException( message ) { } DuplicateDurableClientException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: DuplicateDurableClientException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: DuplicateDurableClientException(const apache::geode::client::DuplicateDurableClientException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } DuplicateDurableClientException(const apache::geode::client::DuplicateDurableClientException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::DuplicateDurableClientException* nativeEx = dynamic_cast<const apache::geode::client::DuplicateDurableClientException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew DuplicateDurableClientException(*nativeEx); } else { return gcnew DuplicateDurableClientException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::DuplicateDurableClientException>(message); } };

      /// <summary>
      /// Thrown when a client is unable to contact any locators.
      /// </summary>
      [Serializable] public ref class NoAvailableLocatorsException: public GeodeException { public: NoAvailableLocatorsException( ) : GeodeException( ) { } NoAvailableLocatorsException( String^ message ) : GeodeException( message ) { } NoAvailableLocatorsException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: NoAvailableLocatorsException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: NoAvailableLocatorsException(const apache::geode::client::NoAvailableLocatorsException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } NoAvailableLocatorsException(const apache::geode::client::NoAvailableLocatorsException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::NoAvailableLocatorsException* nativeEx = dynamic_cast<const apache::geode::client::NoAvailableLocatorsException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew NoAvailableLocatorsException(*nativeEx); } else { return gcnew NoAvailableLocatorsException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::NoAvailableLocatorsException>(message); } };

      /// <summary>
      /// Thrown when all connections in a pool are in use..
      /// </summary>
      [Serializable] public ref class AllConnectionsInUseException: public GeodeException { public: AllConnectionsInUseException( ) : GeodeException( ) { } AllConnectionsInUseException( String^ message ) : GeodeException( message ) { } AllConnectionsInUseException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: AllConnectionsInUseException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: AllConnectionsInUseException(const apache::geode::client::AllConnectionsInUseException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } AllConnectionsInUseException(const apache::geode::client::AllConnectionsInUseException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::AllConnectionsInUseException* nativeEx = dynamic_cast<const apache::geode::client::AllConnectionsInUseException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew AllConnectionsInUseException(*nativeEx); } else { return gcnew AllConnectionsInUseException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::AllConnectionsInUseException>(message); } };

      /// <summary>
      /// Thrown when cq is invalid
      /// </summary>
      [Serializable] public ref class CqInvalidException: public GeodeException { public: CqInvalidException( ) : GeodeException( ) { } CqInvalidException( String^ message ) : GeodeException( message ) { } CqInvalidException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CqInvalidException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CqInvalidException(const apache::geode::client::CqInvalidException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CqInvalidException(const apache::geode::client::CqInvalidException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CqInvalidException* nativeEx = dynamic_cast<const apache::geode::client::CqInvalidException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CqInvalidException(*nativeEx); } else { return gcnew CqInvalidException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CqInvalidException>(message); } };

      /// <summary>
      /// Thrown when function execution failed
      /// </summary>
      [Serializable] public ref class FunctionExecutionException: public GeodeException { public: FunctionExecutionException( ) : GeodeException( ) { } FunctionExecutionException( String^ message ) : GeodeException( message ) { } FunctionExecutionException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: FunctionExecutionException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: FunctionExecutionException(const apache::geode::client::FunctionExecutionException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } FunctionExecutionException(const apache::geode::client::FunctionExecutionException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::FunctionExecutionException* nativeEx = dynamic_cast<const apache::geode::client::FunctionExecutionException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew FunctionExecutionException(*nativeEx); } else { return gcnew FunctionExecutionException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::FunctionExecutionException>(message); } };

      /// <summary>
      /// Thrown during continuous query execution time.
      /// </summary>
      [Serializable] public ref class CqException: public GeodeException { public: CqException( ) : GeodeException( ) { } CqException( String^ message ) : GeodeException( message ) { } CqException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CqException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CqException(const apache::geode::client::CqException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CqException(const apache::geode::client::CqException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CqException* nativeEx = dynamic_cast<const apache::geode::client::CqException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CqException(*nativeEx); } else { return gcnew CqException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CqException>(message); } };

      /// <summary>
      /// Thrown if the Cq on which the operaion performed is closed
      /// </summary>
      [Serializable] public ref class CqClosedException: public GeodeException { public: CqClosedException( ) : GeodeException( ) { } CqClosedException( String^ message ) : GeodeException( message ) { } CqClosedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CqClosedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CqClosedException(const apache::geode::client::CqClosedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CqClosedException(const apache::geode::client::CqClosedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CqClosedException* nativeEx = dynamic_cast<const apache::geode::client::CqClosedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CqClosedException(*nativeEx); } else { return gcnew CqClosedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CqClosedException>(message); } };

      /// <summary>
      /// Thrown if the Cq Query failed
      /// </summary>
      [Serializable] public ref class CqQueryException: public GeodeException { public: CqQueryException( ) : GeodeException( ) { } CqQueryException( String^ message ) : GeodeException( message ) { } CqQueryException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CqQueryException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CqQueryException(const apache::geode::client::CqQueryException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CqQueryException(const apache::geode::client::CqQueryException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CqQueryException* nativeEx = dynamic_cast<const apache::geode::client::CqQueryException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CqQueryException(*nativeEx); } else { return gcnew CqQueryException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CqQueryException>(message); } };

      /// <summary>
      /// Thrown if a Cq by this name already exists on this client
      /// </summary>
      [Serializable] public ref class CqExistsException: public GeodeException { public: CqExistsException( ) : GeodeException( ) { } CqExistsException( String^ message ) : GeodeException( message ) { } CqExistsException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CqExistsException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CqExistsException(const apache::geode::client::CqExistsException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CqExistsException(const apache::geode::client::CqExistsException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CqExistsException* nativeEx = dynamic_cast<const apache::geode::client::CqExistsException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CqExistsException(*nativeEx); } else { return gcnew CqExistsException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CqExistsException>(message); } };

      [Serializable] public ref class InvalidDeltaException: public GeodeException { public: InvalidDeltaException( ) : GeodeException( ) { } InvalidDeltaException( String^ message ) : GeodeException( message ) { } InvalidDeltaException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: InvalidDeltaException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: InvalidDeltaException(const apache::geode::client::InvalidDeltaException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } InvalidDeltaException(const apache::geode::client::InvalidDeltaException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::InvalidDeltaException* nativeEx = dynamic_cast<const apache::geode::client::InvalidDeltaException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew InvalidDeltaException(*nativeEx); } else { return gcnew InvalidDeltaException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::InvalidDeltaException>(message); } };

      /// <summary>
      /// Thrown if a Key is not present in the region.
      /// </summary>
      [Serializable] public ref class KeyNotFoundException: public GeodeException { public: KeyNotFoundException( ) : GeodeException( ) { } KeyNotFoundException( String^ message ) : GeodeException( message ) { } KeyNotFoundException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: KeyNotFoundException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: KeyNotFoundException(const apache::geode::client::KeyNotFoundException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } KeyNotFoundException(const apache::geode::client::KeyNotFoundException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::KeyNotFoundException* nativeEx = dynamic_cast<const apache::geode::client::KeyNotFoundException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew KeyNotFoundException(*nativeEx); } else { return gcnew KeyNotFoundException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::KeyNotFoundException>(message); } };

      /// <summary>
      /// Thrown if commit fails.
      /// </summary>
      [Serializable] public ref class CommitConflictException: public GeodeException { public: CommitConflictException( ) : GeodeException( ) { } CommitConflictException( String^ message ) : GeodeException( message ) { } CommitConflictException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: CommitConflictException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: CommitConflictException(const apache::geode::client::CommitConflictException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } CommitConflictException(const apache::geode::client::CommitConflictException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::CommitConflictException* nativeEx = dynamic_cast<const apache::geode::client::CommitConflictException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew CommitConflictException(*nativeEx); } else { return gcnew CommitConflictException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::CommitConflictException>(message); } };

	        /// <summary>
      /// Thrown if transaction delegate went down.
      /// </summary>
      [Serializable] public ref class TransactionDataNodeHasDepartedException: public GeodeException { public: TransactionDataNodeHasDepartedException( ) : GeodeException( ) { } TransactionDataNodeHasDepartedException( String^ message ) : GeodeException( message ) { } TransactionDataNodeHasDepartedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: TransactionDataNodeHasDepartedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: TransactionDataNodeHasDepartedException(const apache::geode::client::TransactionDataNodeHasDepartedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } TransactionDataNodeHasDepartedException(const apache::geode::client::TransactionDataNodeHasDepartedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::TransactionDataNodeHasDepartedException* nativeEx = dynamic_cast<const apache::geode::client::TransactionDataNodeHasDepartedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew TransactionDataNodeHasDepartedException(*nativeEx); } else { return gcnew TransactionDataNodeHasDepartedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::TransactionDataNodeHasDepartedException>(message); } };

	        /// <summary>
      /// Thrown if commit rebalance happens during a transaction.
      /// </summary>
      [Serializable] public ref class TransactionDataRebalancedException: public GeodeException { public: TransactionDataRebalancedException( ) : GeodeException( ) { } TransactionDataRebalancedException( String^ message ) : GeodeException( message ) { } TransactionDataRebalancedException( String^ message, System::Exception^ innerException ) : GeodeException( message, innerException ) { } protected: TransactionDataRebalancedException( SerializationInfo^ info, StreamingContext context ) : GeodeException( info, context ) { } internal: TransactionDataRebalancedException(const apache::geode::client::TransactionDataRebalancedException& nativeEx) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), gcnew GeodeException(GeodeException::GetStackTrace( nativeEx))) { } TransactionDataRebalancedException(const apache::geode::client::TransactionDataRebalancedException& nativeEx, Exception^ innerException) : GeodeException(marshal_as<String^>(nativeEx.getMessage()), innerException) { } static GeodeException^ Create(const apache::geode::client::Exception& ex, Exception^ innerException) { const apache::geode::client::TransactionDataRebalancedException* nativeEx = dynamic_cast<const apache::geode::client::TransactionDataRebalancedException*>( &ex ); if (nativeEx != nullptr) { if (innerException == nullptr) { return gcnew TransactionDataRebalancedException(*nativeEx); } else { return gcnew TransactionDataRebalancedException(*nativeEx, innerException); } } return nullptr; } virtual std::shared_ptr<apache::geode::client::Exception> GetNative() override { auto message = marshal_as<std::string>(this->Message + ": " + this->StackTrace); if (this->InnerException != nullptr) { auto cause = GeodeException::GetNative(this->InnerException); message += "Caused by: " + cause->getMessage(); } return std::make_shared<apache::geode::client::TransactionDataRebalancedException>(message); } };

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

