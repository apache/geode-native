using System;
using System.Runtime.InteropServices;

namespace Apache 
{
    namespace Geode
    {
        namespace NetCore
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