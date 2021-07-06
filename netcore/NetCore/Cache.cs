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
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Apache {
  namespace Geode {
    namespace Client {
      public class Cache : GeodeNativeObject, IGeodeCache {
        private static string _name = String.Empty;
        private PoolManager _poolManager = null;
        private PoolFactory _poolFactory = null;
        private IAuthInitialize _authInitialize;
        private GetCredentialsDelegateInternal _getCredentialsDelegate;
        private CloseDelegateInternal _closeDelegate;

        internal delegate void GetCredentialsDelegateInternal(IntPtr cache);

        internal delegate void CloseDelegateInternal();

        [DllImport(Constants.libPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern void apache_geode_CacheFactory_SetAuthInitialize(
            IntPtr factory, GetCredentialsDelegateInternal getCredentials,
            CloseDelegateInternal close);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern IntPtr apache_geode_CacheFactory_CreateCache(IntPtr factory);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern bool apache_geode_Cache_GetPdxIgnoreUnreadFields(IntPtr cache);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern bool apache_geode_Cache_GetPdxReadSerialized(IntPtr cache);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern IntPtr apache_geode_Cache_GetName(IntPtr cache);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern void apache_geode_Cache_Close(IntPtr cache, bool keepalive);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern bool apache_geode_Cache_IsClosed(IntPtr cache);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern bool apache_geode_AuthInitialize_AddProperty(IntPtr properties,
                                                                           IntPtr key,
                                                                           IntPtr value);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern void apache_geode_DestroyCache(IntPtr cache);

        internal Cache(IntPtr cacheFactory, IAuthInitialize authInitialize) {
          _authInitialize = authInitialize;
          if (_authInitialize != null) {
            _getCredentialsDelegate = new GetCredentialsDelegateInternal(AuthGetCredentials);
            _closeDelegate = new CloseDelegateInternal(AuthClose);

            apache_geode_CacheFactory_SetAuthInitialize(cacheFactory, _getCredentialsDelegate,
                                                        _closeDelegate);
          }
          _containedObject = apache_geode_CacheFactory_CreateCache(cacheFactory);
        }

        internal void AuthGetCredentials(IntPtr properties) {
          Console.WriteLine("In AuthInitialize::GetCredentials callback");
          if (_authInitialize == null) {
            throw new InvalidOperationException(
                "AuthInitialize callback received for cache without authentication!");
          }

          var credentials = _authInitialize.GetCredentials();
          foreach (KeyValuePair<string, string> entry in credentials) {
            Console.WriteLine("Found credential: (k, v) = ({0}, {1})", entry.Key, entry.Value);
            IntPtr keyPtr = Marshal.StringToCoTaskMemUTF8(entry.Key);
            IntPtr valuePtr = Marshal.StringToCoTaskMemUTF8(entry.Value);
            apache_geode_AuthInitialize_AddProperty(properties, keyPtr, valuePtr);
            Marshal.FreeCoTaskMem(keyPtr);
            Marshal.FreeCoTaskMem(valuePtr);
          }
        }

        public void AuthClose() {
          Console.WriteLine("In AuthInitialize::Close callback");
        }

        public void Close() {
          apache_geode_Cache_Close(_containedObject, false);
        }

        public void Close(bool keepalive) {
          apache_geode_Cache_Close(_containedObject, keepalive);
        }

        public bool GetPdxIgnoreUnreadFields() {
          return apache_geode_Cache_GetPdxIgnoreUnreadFields(_containedObject);
        }

        public bool GetPdxReadSerialized() {
          return apache_geode_Cache_GetPdxReadSerialized(_containedObject);
        }

        public string Name {
          get {
            if (_name == String.Empty) {
              _name = Marshal.PtrToStringUTF8(apache_geode_Cache_GetName(_containedObject));
            }

            return _name;
          }
        }

        public PoolManager PoolManager {
          get {
            if (_poolManager == null) {
              _poolManager = new PoolManager(_containedObject);
            }

            return _poolManager;
          }
        }

        public PoolFactory PoolFactory {
          get {
            if (_poolFactory == null) {
              _poolFactory = PoolManager.CreatePoolFactory();
            }

            return _poolFactory;
          }
        }

        public RegionFactory CreateRegionFactory(RegionShortcut regionType) {
          return new RegionFactory(_containedObject, regionType);
        }

        public bool Closed => apache_geode_Cache_IsClosed(_containedObject);

        protected override void DestroyContainedObject() {
          // It turns out, C# "wrapper" objects need to get rid of
          // *all* contained objects, due to vagaries of Geode
          // Native object graph, in order to ensure a leak-free
          // shutdown.  We get rid of our non-cache objects first
          // here, in case it makes a difference.
          _poolManager?.Dispose();
          _poolManager = null;
          _poolFactory?.Dispose();
          _poolFactory = null;
          apache_geode_DestroyCache(_containedObject);
          _containedObject = IntPtr.Zero;
        }
      }
    }
  }
}
