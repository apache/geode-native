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
using System.Runtime.InteropServices;

namespace Apache {
  namespace Geode {
    namespace Client {
      public class CacheFactory : GeodeNativeObject, ICacheFactory {
        private string _version = String.Empty;
        private string _productDescription = String.Empty;
        private IAuthInitialize _authInitialize;

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern IntPtr apache_geode_CreateCacheFactory();

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern void apache_geode_DestroyCacheFactory(IntPtr factory);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern IntPtr apache_geode_CacheFactory_GetVersion(IntPtr factory);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern IntPtr apache_geode_CacheFactory_GetProductDescription(
            IntPtr factory);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern void apache_geode_CacheFactory_SetPdxIgnoreUnreadFields(
            IntPtr factory, bool pdxIgnoreUnreadFields);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern void apache_geode_CacheFactory_SetPdxReadSerialized(
            IntPtr factory, bool pdxReadSerialized);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern void apache_geode_CacheFactory_SetProperty(IntPtr factory, IntPtr key,
                                                                         IntPtr value);

        public static ICacheFactory Create() {
          return new CacheFactory();
        }

        public CacheFactory() {
          _containedObject = apache_geode_CreateCacheFactory();
        }

        public string Version {
          get {
            if (_version == String.Empty) {
              _version =
                  Marshal.PtrToStringUTF8(apache_geode_CacheFactory_GetVersion(_containedObject));
            }

            return _version;
          }
        }

        public string ProductDescription {
          get {
            if (_productDescription == String.Empty) {
              _productDescription = Marshal.PtrToStringUTF8(
                  apache_geode_CacheFactory_GetProductDescription(_containedObject));
            }

            return _productDescription;
          }
        }

        public bool PdxIgnoreUnreadFields {
          set => apache_geode_CacheFactory_SetPdxIgnoreUnreadFields(_containedObject, value);
        }

        public bool PdxReadSerialized {
          set => apache_geode_CacheFactory_SetPdxReadSerialized(_containedObject, value);
        }

        public IGeodeCache CreateCache() {
          return new Cache(_containedObject, _authInitialize);
        }

        public ICacheFactory SetProperty(string key, string value) {
          var utf8Key = Marshal.StringToCoTaskMemUTF8(key);
          var utf8Value = Marshal.StringToCoTaskMemUTF8(value);
          apache_geode_CacheFactory_SetProperty(_containedObject, utf8Key, utf8Value);
          Marshal.FreeCoTaskMem(utf8Key);
          Marshal.FreeCoTaskMem(utf8Value);
          return this;
        }

        public IAuthInitialize AuthInitialize { set => _authInitialize = value; }

        protected override void DestroyContainedObject() {
          apache_geode_DestroyCacheFactory(_containedObject);
          _containedObject = IntPtr.Zero;
        }
      }
    }
  }
}