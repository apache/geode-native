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
      public class Pool : GeodeNativeObject {
        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern IntPtr apache_geode_Cache_GetPoolManager(IntPtr cache);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern IntPtr apache_geode_PoolFactory_CreatePool(IntPtr poolFactory,
                                                                         IntPtr poolName);

        [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
        private static extern void apache_geode_DestroyPool(IntPtr pool);

        internal Pool(IntPtr poolFactory, string poolName) {
          IntPtr poolNamePtr = Marshal.StringToCoTaskMemUTF8(poolName);
          _containedObject = apache_geode_PoolFactory_CreatePool(poolFactory, poolNamePtr);
          Marshal.FreeCoTaskMem(poolNamePtr);
        }

        protected override void DestroyContainedObject() {
          apache_geode_DestroyPool(_containedObject);
          _containedObject = IntPtr.Zero;
        }
      }
    }
  }
}