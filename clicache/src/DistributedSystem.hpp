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

#include "geode_defs.hpp"
#include "begin_native.hpp"
#include <geode/DistributedSystem.hpp>
#include "end_native.hpp"

#include "native_conditional_unique_ptr.hpp"
#include "SystemProperties.hpp"
#include "Properties.hpp"
#include "impl/CliCallbackDelgate.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      namespace native = apache::geode::client;

      /// <summary>
      /// DistributedSystem encapsulates this applications "connection" into the
      /// Geode Java servers.
      /// </summary>
      /// <remarks>
      /// In order to participate as a client in the Geode Java servers
      /// distributed system, each application needs to connect to the
      /// DistributedSystem.
      /// </remarks>
      public ref class DistributedSystem sealed
      {
      public:

        /// <summary>
        /// Initializes the Native Client system to be able to connect to the Geode Java servers.
        /// </summary>
        /// <param name="name">the name of the system to connect to</param>
        /// <exception cref="IllegalArgumentException">if name is null</exception>
        /// <exception cref="NoSystemException">
        /// if the connecting target is not running
        /// </exception>
        /// <exception cref="AlreadyConnectedException">
        /// if trying a second connect.
        /// An application can have one only one connection to a DistributedSystem.
        /// </exception>
        /// <exception cref="UnknownException">otherwise</exception>
        DistributedSystem^ Connect(String^ name, Cache^ cache);

        /// <summary>
        /// Initializes the Native Client system to be able to connect to the
        /// Geode Java servers.
        /// </summary>
        /// <param name="name">the name of the system to connect to</param>
        /// <param name="config">the set of properties</param>
        /// <exception cref="IllegalArgumentException">if name is null</exception>
        /// <exception cref="NoSystemException">
        /// if the connecting target is not running
        /// </exception>
        /// <exception cref="AlreadyConnectedException">
        /// if trying a second connect.
        /// An application can have one only one connection to a DistributedSystem.
        /// </exception>
        /// <exception cref="UnknownException">otherwise</exception>
        static DistributedSystem^ Connect(String^ name, Properties<String^, String^>^ config, Cache^ cache);

        /// <summary>
        /// Disconnect from the distributed system.
        /// </summary>
        /// <exception cref="IllegalStateException">if not connected</exception>
        void Disconnect(Cache^ cache);

        /// <summary>
        /// Returns the SystemProperties used to create this instance of a
        /// <c>DistributedSystem</c>.
        /// </summary>
        /// <returns>the SystemProperties</returns>
        property Apache::Geode::Client::SystemProperties^ SystemProperties
        {
          Apache::Geode::Client::SystemProperties^ get();
        }

        /// <summary>
        /// Get the name that identifies this DistributedSystem instance.
        /// </summary>
        /// <returns>the name of the DistributedSystem instance.</returns>
        property String^ Name
        {
          String^ get();
        }


      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class with null pointer check.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        /// <returns>
        /// The managed wrapper object; null if the native pointer is null.
        /// </returns>
        static DistributedSystem^ Create(native::DistributedSystem* nativeptr);

        DistributedSystem(std::unique_ptr<native::DistributedSystem> nativeptr);

        static void acquireDisconnectLock();

        static void disconnectInstance();

        static void releaseDisconnectLock();

        static void connectInstance();

        delegate void cliCallback(apache::geode::client::Cache& cache);

        static void registerCliCallback();

        static void unregisterCliCallback();
        /// <summary>
        /// Stuff that needs to be done for Connect in each AppDomain.
        /// </summary>
        static void AppDomainInstanceInitialization(Cache^ cache);

        /// <summary>
        /// Managed registrations and other stuff to be done for the manage
        /// layer after the first connect.
        /// </summary>
        static void ManagedPostConnect(Cache^ cache);

        /// <summary>
        /// Stuff that needs to be done for Connect in each AppDomain but
        /// only after the first Connect has completed.
        /// </summary>
        static void AppDomainInstancePostInitialization();

        /// <summary>
        /// Unregister the builtin managed types like CacheableObject.
        /// </summary>
        static void UnregisterBuiltinManagedTypes(Cache^ cache);

        native::DistributedSystem& GetNative()
        {
          return *(m_nativeptr->get());
        }

      private:

        ///// <summary>
        ///// Stores the task ID of the task that adjusts unmanaged memory
        ///// pressure in managed GC.
        ///// </summary>
        //static long s_memoryPressureTaskID = -1;

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        DistributedSystem(native::DistributedSystem* nativeptr);

        /// <summary>
        /// Finalizer for the singleton instance of this class.
        /// </summary>
        ~DistributedSystem();

        native_conditional_unique_ptr<native::DistributedSystem>^ m_nativeptr;


        /// <summary>
        /// Periodically adjust memory pressure of unmanaged heap for GC.
        /// </summary>
        static void HandleMemoryPressure(System::Object^ state);

        /// <summary>
        /// Timer task to periodically invoke <c>HandleMemoryPressure</c>.
        /// </summary>
        System::Threading::Timer^ m_memoryPressureHandler;

        /// <summary>
        /// Singleton instance of this class.
        /// </summary>
        static volatile DistributedSystem^ m_instance;

        /// <summary>
        /// Static lock object to protect the singleton instance of this class.
        /// </summary>
        static System::Object^ m_singletonSync = gcnew System::Object();

        static CliCallbackDelegate^ m_cliCallBackObj;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

