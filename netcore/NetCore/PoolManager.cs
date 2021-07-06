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

namespace Apache 
{
    namespace Geode
    {
        namespace Client
        {
            public class PoolManager : GeodeNativeObject
            {
                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern IntPtr apache_geode_Cache_GetPoolManager(IntPtr cache);

                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern IntPtr apache_geode_DestroyPoolManager(IntPtr poolManager);

                internal PoolManager(IntPtr cache)
                {
                    _containedObject = apache_geode_Cache_GetPoolManager(cache);
                }

                public PoolFactory CreatePoolFactory()
                {
                    return new PoolFactory(_containedObject);
                }

                protected override void DestroyContainedObject()
                {
                    apache_geode_DestroyPoolManager(_containedObject);
                    _containedObject = IntPtr.Zero;
                }
            }
        }
    }
}