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
        namespace NetCore
        {
            public class Region : GeodeNativeObject
            {
                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern IntPtr apache_geode_RegionFactory_CreateRegion(IntPtr cache, IntPtr regionName);

                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern bool apache_geode_Region_PutString(IntPtr region, IntPtr key, IntPtr value);

                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern bool apache_geode_Region_PutByteArray(IntPtr region, IntPtr key, IntPtr value, int length);

                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern IntPtr apache_geode_Region_GetString(IntPtr region, IntPtr key);

                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern void apache_geode_Region_GetByteArray(IntPtr region, IntPtr key, ref IntPtr value, ref int size);

                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern void apache_geode_Region_Remove(IntPtr region, IntPtr key);

                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern bool apache_geode_Region_ContainsValueForKey(IntPtr region, IntPtr key);

                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern IntPtr apache_geode_DestroyRegion(IntPtr region);

                internal Region(IntPtr regionFactory, string regionName)
                {
                    IntPtr regionNamePtr = Marshal.StringToCoTaskMemUTF8(regionName);
                    _containedObject = apache_geode_RegionFactory_CreateRegion(regionFactory, regionNamePtr);
                    Marshal.FreeCoTaskMem(regionNamePtr);
                }

                public void PutString(string key, string value)
                {
                    IntPtr keyPtr = Marshal.StringToCoTaskMemUTF8(key);
                    IntPtr valuePtr = Marshal.StringToCoTaskMemUTF8(value);
                    apache_geode_Region_PutString(_containedObject, keyPtr, valuePtr);
                    Marshal.FreeCoTaskMem(keyPtr);
                    Marshal.FreeCoTaskMem(valuePtr);
                }

                public void PutByteArray(string key, byte[] value)
                {
                    IntPtr keyPtr = Marshal.StringToCoTaskMemUTF8(key);
                    IntPtr valuePtr = Marshal.AllocCoTaskMem(value.Length);
                    Marshal.Copy(value, 0, valuePtr, value.Length);
                    apache_geode_Region_PutByteArray(_containedObject, keyPtr, valuePtr, value.Length);
                    Marshal.FreeCoTaskMem(keyPtr);
                    Marshal.FreeCoTaskMem(valuePtr);
                }

                public string GetString(string key)
                {
                    IntPtr keyPtr = Marshal.StringToCoTaskMemUTF8(key);
                    var result = Marshal.PtrToStringUTF8(apache_geode_Region_GetString(_containedObject, keyPtr));
                    Marshal.FreeCoTaskMem(keyPtr);
                    return result;
                }

                public byte[] GetByteArray(string key)
                {
                    IntPtr keyPtr = Marshal.StringToCoTaskMemUTF8(key);
                    IntPtr valPtr = (IntPtr)0;
                    int size = 0;
                    apache_geode_Region_GetByteArray(_containedObject, keyPtr, ref valPtr, ref size);
                    if (size > 0)
                    {
                      Byte[] byteArray = new Byte[size];
                      Marshal.Copy(valPtr, byteArray, 0, size);
                      Marshal.FreeCoTaskMem(valPtr);
                      return byteArray;
                    }
                    else
                      return null;
                }

                public void Remove(string key)
                {
                    IntPtr keyPtr = Marshal.StringToCoTaskMemUTF8(key);
                    apache_geode_Region_Remove(_containedObject, keyPtr);
                    Marshal.FreeCoTaskMem(keyPtr);
                }

                public bool ContainsValueForKey(string key)
                {
                    IntPtr keyPtr = Marshal.StringToCoTaskMemUTF8(key);
                    bool result = apache_geode_Region_ContainsValueForKey(_containedObject, keyPtr);
                    Marshal.FreeCoTaskMem(keyPtr);
                    return result;
                }

                protected override void DestroyContainedObject()
                {
                    apache_geode_DestroyRegion(_containedObject);
                    _containedObject = IntPtr.Zero;
                }
            }
        }
    }
}