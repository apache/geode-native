using System;
using System.Runtime.InteropServices;

namespace Apache
{
    namespace Geode
    {
        namespace NetCore
        {
            public class Client : GeodeNativeObject
            {
                [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
                private static extern IntPtr apache_geode_ClientInitialize();

                [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
                private static extern int apache_geode_ClientUninitialize(IntPtr client);

                public Client()
                {
                    _containedObject = apache_geode_ClientInitialize();
                }

                protected override void DestroyContainedObject()
                {
                    var err = apache_geode_ClientUninitialize(_containedObject);
                    _containedObject = IntPtr.Zero;
                    if (err != 0) {
                        throw new InvalidOperationException("One or more native objects was leaked!  See Gemfire log for debugging info.");
                    }
                }
            }
        }
    }
}