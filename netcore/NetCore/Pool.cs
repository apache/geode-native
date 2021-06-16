using System;
using System.Runtime.InteropServices;

namespace Apache 
{
    namespace Geode
    {
        namespace NetCore
        {
            public class Pool : GeodeNativeObject
            {
                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern IntPtr apache_geode_Cache_GetPoolManager(IntPtr cache);

                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern IntPtr apache_geode_PoolFactory_CreatePool(IntPtr poolFactory, IntPtr poolName);

                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern void apache_geode_DestroyPool(IntPtr pool);

                internal Pool(IntPtr poolFactory, string poolName)
                {
                    IntPtr poolNamePtr = Marshal.StringToCoTaskMemUTF8(poolName);
                    _containedObject = apache_geode_PoolFactory_CreatePool(poolFactory, poolNamePtr);
                    Marshal.FreeCoTaskMem(poolNamePtr);
                }

                protected override void DestroyContainedObject()
                {
                    apache_geode_DestroyPool(_containedObject);
                    _containedObject = IntPtr.Zero;
                }
            }
        }
    }
}