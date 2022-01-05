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

using Apache.Geode.Client;


namespace Apache.Geode.Client {
  public class RegionFactory<TKey, TValue> : GeodeNativeObject {
    //[DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    //private static extern IntPtr apache_geode_Cache_CreateRegionFactory(IntPtr cache,
    //                                                                    int regionType);

    //[DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    //private static extern IntPtr apache_geode_DestroyRegionFactory(IntPtr regionFactory);

    internal RegionFactory(IntPtr cache, RegionShortcut regionType) {
      _containedObject = CBindings.apache_geode_Cache_CreateRegionFactory(cache, (int)regionType);
    }

    public IRegion<TValue> CreateRegion(string regionName) {
      return new Region<TValue>(_containedObject, regionName);
    }

    protected override void DestroyContainedObject() {
      CBindings.apache_geode_DestroyRegionFactory(_containedObject);
      _containedObject = IntPtr.Zero;
    }
  }

  //public class RegionFactory<TVal> {
  //  RegionFactory regionFactory_;
  //  public RegionFactory(IntPtr cache, RegionShortcut regionType)
  //  {
  //    regionFactory_ = new RegionFactory(cache, regionType);
  //  }
  //  public Region<TVal> CreateRegion<TVal>(string regionName)
  //  {
  //    return new regionFactory_.CreateRegion(regionName);
  //  }
  //}
}
