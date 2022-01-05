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

namespace Apache.Geode.Client {
  public class CacheFactory<TKey, TValue> : GeodeNativeObject, ICacheFactory<TKey, TValue> {
    private string _version = String.Empty;
    private string _productDescription = String.Empty;
    private IAuthInitialize _authInitialize;



    public static ICacheFactory<TKey, TValue> Create() {
      return new CacheFactory<TKey, TValue>();
    }

    public CacheFactory() {
      _containedObject = CBindings.apache_geode_CreateCacheFactory();
    }

    public string Version {
      get {
        if (_version == String.Empty) {
          _version =
              Marshal.PtrToStringUTF8(CBindings.apache_geode_CacheFactory_GetVersion(_containedObject));
        }

        return _version;
      }
    }

    public string ProductDescription {
      get {
        if (_productDescription == String.Empty) {
          _productDescription = Marshal.PtrToStringUTF8(
              CBindings.apache_geode_CacheFactory_GetProductDescription(_containedObject));
        }

        return _productDescription;
      }
    }

    public bool PdxIgnoreUnreadFields {
      set => CBindings.apache_geode_CacheFactory_SetPdxIgnoreUnreadFields(_containedObject, value);
    }

    public bool PdxReadSerialized {
      set => CBindings.apache_geode_CacheFactory_SetPdxReadSerialized(_containedObject, value);
    }

    public IGeodeCache<TKey, TValue> CreateCache() {
      return new Cache<TKey, TValue>(_containedObject, _authInitialize);
    }

    public ICacheFactory<TKey, TValue> SetProperty(string key, string value) {
      var utf8Key = Marshal.StringToCoTaskMemUTF8(key);
      var utf8Value = Marshal.StringToCoTaskMemUTF8(value);
      CBindings.apache_geode_CacheFactory_SetProperty(_containedObject, utf8Key, utf8Value);
      Marshal.FreeCoTaskMem(utf8Key);
      Marshal.FreeCoTaskMem(utf8Value);
      return this;
    }

    public IAuthInitialize AuthInitialize { set => _authInitialize = value; }

    protected override void DestroyContainedObject() {
      CBindings.apache_geode_DestroyCacheFactory(_containedObject);
      _containedObject = IntPtr.Zero;
    }
  }
}
