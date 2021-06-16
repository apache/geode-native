using System;

namespace Apache
{
    namespace Geode
    {
        namespace NetCore
        {
            public interface ICacheFactory : IDisposable
            {
                IGeodeCache CreateCache();
                ICacheFactory SetProperty(String name, String value);
                IAuthInitialize AuthInitialize { set; }
                bool PdxIgnoreUnreadFields { set; }
                bool PdxReadSerialized { set; }
                String Version { get; }
                String ProductDescription { get; }
            }
        }
    }
}
