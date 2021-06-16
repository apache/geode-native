using System;
using System.Runtime.InteropServices;

namespace Apache 
{
    namespace Geode
    {
        namespace NetCore
        {
            public class RegionFactory : GeodeNativeObject
            {
                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern IntPtr apache_geode_Cache_CreateRegionFactory(IntPtr cache, int regionType);

                [DllImport(Constants.libPath,
                    CharSet = CharSet.Auto)]
                private static extern IntPtr apache_geode_DestroyRegionFactory(IntPtr regionFactory);

                internal RegionFactory(IntPtr cache, RegionShortcut regionType)
                {
                    _containedObject = apache_geode_Cache_CreateRegionFactory(cache, (int) regionType);
                }

                public Region CreateRegion(string regionName)
                {
                    return new Region(_containedObject, regionName);
                }

                protected override void DestroyContainedObject()
                {
                    apache_geode_DestroyRegionFactory(_containedObject);
                    _containedObject = IntPtr.Zero;
                }
            }
        }
    }
}